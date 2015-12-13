/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Dataquay

    A C++/Qt library for simple RDF datastore management.
    Copyright 2009-2012 Chris Cannam.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the name of Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#include "objectmapper/ObjectMapper.h"

#include "objectmapper/ObjectMapperExceptions.h"
#include "objectmapper/ObjectMapperForwarder.h"
#include "objectmapper/ObjectLoader.h"
#include "objectmapper/ObjectStorer.h"
#include "objectmapper/ContainerBuilder.h"

#include "objectmapper/TypeMapping.h"

#include "TransactionalStore.h"
#include "Connection.h"
#include "PropertyObject.h"

#include "../Debug.h"

#include <typeinfo>

#include <QMetaProperty>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QHash>

namespace Dataquay {
    
class
ObjectMapper::D : public QObject
{
    struct Network
    {
        // Use Network only with mutex held please

        ObjectLoader::NodeObjectMap nodeObjectMap;
        ObjectStorer::ObjectNodeMap objectNodeMap;

        ObjectLoader::NodeObjectMap listNodeObjectMap;
        
        Node getNodeForObject(QObject *o) {
            ObjectStorer::ObjectNodeMap::const_iterator i =
                objectNodeMap.find(o);
            if (i != objectNodeMap.end()) return i.value();
            return Node();
        }
        QObject *getObjectByNode(Node n) {
            ObjectLoader::NodeObjectMap::const_iterator i =
                nodeObjectMap.find(n);
            if (i != nodeObjectMap.end()) return i.value();
            return 0;
        }
    };

    struct LoadStoreCallback : public ObjectStorer::StoreCallback,
                               public ObjectLoader::LoadCallback
    {
        LoadStoreCallback(ObjectMapper::D *d) : m_d(d) { }
        void loaded(ObjectLoader *, ObjectLoader::NodeObjectMap &,
                    Node, QObject *o) {
            DEBUG << "LoadStoreCallback::loaded: Object " << o << endl;
            m_d->manage(o);
        }
        void stored(ObjectStorer *, ObjectStorer::ObjectNodeMap &,
                    QObject *o, Node) {
            DEBUG << "LoadStoreCallback::stored: Object " << o << endl;
            m_d->manage(o);
        }
    private:
        ObjectMapper::D *m_d;
    };

public:
    D(ObjectMapper *m, TransactionalStore *s) :
        m_m(m),
        m_s(s),
        m_c(s),
        m_mutex(QMutex::Recursive),
        m_inCommit(false),
        m_inReload(false),
        m_callback(this)
    {
        m_loader = new ObjectLoader(&m_c);
        m_loader->setAbsentPropertyPolicy(ObjectLoader::ResetAbsentProperties);
        m_loader->setFollowPolicy(ObjectLoader::FollowObjectProperties);
        m_loader->addLoadCallback(&m_callback, ObjectLoader::FinalCallback);

        m_storer = new ObjectStorer(&m_c);
        m_storer->setPropertyStorePolicy(ObjectStorer::StoreIfChanged);
        m_storer->setBlankNodePolicy(NoBlankObjectNodes);
        m_storer->setFollowPolicy(ObjectStorer::FollowObjectProperties);
        m_storer->addStoreCallback(&m_callback);

        connect(&m_c, SIGNAL(transactionCommitted(const ChangeSet &)),
                m_m, SLOT(transactionCommitted(const ChangeSet &)));
    }

    virtual ~D() {
        delete m_storer;
        delete m_loader;
    }
    
    TransactionalStore *getStore() {
        return m_s;
    }

    void setTypeMapping(const TypeMapping &tm) {
        QMutexLocker locker(&m_mutex);
        m_tm = tm;
        m_loader->setTypeMapping(m_tm);
        m_storer->setTypeMapping(m_tm);
    }

    const TypeMapping &getTypeMapping() const {
        return m_tm;
    }

    void setBlankNodePolicy(BlankNodePolicy bp) {
        m_storer->setBlankNodePolicy(bp);
    }

    BlankNodePolicy getBlankNodePolicy() const {
        return m_storer->getBlankNodePolicy();
    }

    Node getNodeForObject(QObject *o) {
        QMutexLocker locker(&m_mutex);
        return m_n.getNodeForObject(o);
    }
    
    QObject *getObjectByNode(Node n) {
        QMutexLocker locker(&m_mutex);
        return m_n.getObjectByNode(n);
    }

    QObject *load(Node n) {
        QMutexLocker locker(&m_mutex);
        return m_loader->load(n);
    }

    QObjectList loadType(Uri u) {
        QMutexLocker locker(&m_mutex);
        return m_loader->loadType(u);
    }

    void add(QObject *o) {
        QMutexLocker locker(&m_mutex);
        try {
            manage(o);
        } catch (NoUriException) {
            // doesn't matter; it will be assigned one when mapped,
            // which will happen on the next commit because we are
            // adding it to the changed object map
        }
        DEBUG << "ObjectMapper::add: Adding " << o << " to changed list" << endl;
        m_changedObjects.insert(o);
    }

    void add(QObjectList ol) {
        QMutexLocker locker(&m_mutex);

        foreach (QObject *o, ol) {
            try {
                manage(o);
            } catch (NoUriException) {
                // doesn't matter (as above)
            }
        }
        DEBUG << "ObjectMapper::add: Adding " << ol.size() << " object(s) to changed list" << endl;
        foreach (QObject *o, ol) {
            m_changedObjects.insert(o);
        }
    }

    void manage(QObject *o) {
        QMutexLocker locker(&m_mutex);

        // Ensure that the Uri datatype has been registered. This is a
        // bit of a hack
        (void)Uri::metaTypeId();

        Uri uri = o->property("uri").value<Uri>();
        if (uri == Uri()) {
            //!!! document this -- generally, document conditions for manage() to be used rather than add()
            throw NoUriException(o->objectName(), o->metaObject()->className());
        }

        // An object is managed if we have it in both maps. If it's
        // only in one map, then it has probably been stored or loaded
        // by following a property or other connection, and stored in
        // the map by the loader/storer call, but not yet properly
        // managed (particularly, not yet connected to a forwarder).
        // This function is called from the load/store callback
        // specifically to deal with managing such
        // known-but-not-properly-managed objects before the two maps
        // are synchronised.  If it wasn't called, such objects would
        // remain forever in a sort of half-managed limbo.
        
        if (m_n.objectNodeMap.contains(o) &&
            m_n.nodeObjectMap.contains(Node(uri))) {
            DEBUG << "ObjectMapper::manage: Object " << o
                  << " " << uri << " is already managed" << endl;
            return;
        }

        DEBUG << "ObjectMapper::manage: Managing " << o
              << " " << uri << endl;

        // The forwarder avoids us trying to connect potentially many,
        // many signals to the same mapper object -- which is slow.
        // Note the forwarder is a child of the ObjectMapper, so we
        // don't need to explicitly destroy it in dtor
        ObjectMapperForwarder *f = new ObjectMapperForwarder(m_m, o);
        m_forwarders.insert(o, f);

        m_n.objectNodeMap.insert(o, uri);
        m_n.nodeObjectMap.insert(uri, o);
    }

    void addListNodesFor(QObject *o) {

        DEBUG << "addListNodesFor(" << o << ")" << endl;

        if (!m_n.objectNodeMap.contains(o)) {
            DEBUG << "addListNodesFor(" << o << "): Object is unknown to us" << endl;
            return;
        }
            
        Node n = m_n.objectNodeMap.value(o);

        DEBUG << "addListNodesFor: Node is " << n << endl;

        ContainerBuilder *cb = ContainerBuilder::getInstance();
        
        for (int i = 0; i < o->metaObject()->propertyCount(); ++i) {
            
            QMetaProperty property = o->metaObject()->property(i);
            
            if (!property.isStored() ||
                !property.isReadable()) {
                continue;
            }
            
            if (cb->getContainerKind(property.typeName()) ==
                ContainerBuilder::SequenceKind) {
                DEBUG << "addListNodesFor: Property " << property.name() << " is a sequence kind" << endl;
                addListNodesForProperty(o, n, property);
            }
        }
    }

    void removeListNodesFor(QObject *o) {

        DEBUG << "removeListNodesFor(" << o << ")" << endl;
        
        //!!! ouch

        Nodes toRemove;
        for (ObjectLoader::NodeObjectMap::iterator i = m_n.listNodeObjectMap.begin();
             i != m_n.listNodeObjectMap.end(); ++i) {
            if (i.value() == o) toRemove.push_back(i.key());
        }

        foreach (Node n, toRemove) {
            m_n.listNodeObjectMap.remove(n);
        }
    }

    void addListNodesForProperty(QObject *o, Node n,
                                 const QMetaProperty &property) {
        
        //!!! todo: write a unit test!

        QString cname = o->metaObject()->className();
        QString pname = property.name();
        Uri puri;
            
        if (!m_tm.getPropertyUri(cname, pname, puri)) {
            PropertyObject po(m_s, m_tm.getPropertyPrefix().toString(), n);
            puri = po.getPropertyUri(pname);
        }

        //!!! should be matching all, surely?
        Node itr = m_s->matchOnce(Triple(n, puri, Node())).c;
        if (itr == Node()) {
            return; // property has no values
        }
        
        Node nil = m_s->expand("rdf:nil");

        while (1) {
            Node next = m_s->matchOnce
                (Triple(itr, m_s->expand("rdf:rest"), Node())).c;
            if (next == Node()) { // This is not a list node at all!
                DEBUG << "addListNodesForProperty: Strange, node " << itr
                      << " (from property URI " << puri << " of object node "
                      << n << ", object " << o << ") is not a list node" << endl;
                break;
            }
            m_n.listNodeObjectMap[itr] = o;
            DEBUG << "addListNodesForProperty: Added node " << itr
                  << " for object " << o << endl;
            if (next == nil) { // we've finished
                break;
            }
            itr = next;
        }
    }

    void manage(QObjectList ol) {
        foreach (QObject *o, ol) {
            manage(o);
        }
    }

    void unmanage(QObject *) { } //!!!

    void unmanage(QObjectList ol) {
        foreach (QObject *o, ol) {
            unmanage(o);
        }
    }

    void objectModified(QObject *o) {
        DEBUG << "ObjectMapper:: objectModified(" << o << ")" << endl;
        QMutexLocker locker(&m_mutex);
        if (m_inReload) {
            // This signal must have been emitted by a modification
            // caused by our own transactionCommitted method (see
            // similar comment about m_inCommit in that method).
            DEBUG << "(by us, ignoring it)" << endl;
            return;
        }

        //!!! what if the thing that changed about the object was its URL???!!!

        m_changedObjects.insert(o);
        DEBUG << "ObjectMapper::objectModified done" << endl;
    }

    void objectDestroyed(QObject *o) {
        DEBUG << "ObjectMapper::objectDestroyed(" << o << ")" << endl;
        QMutexLocker locker(&m_mutex);
        m_changedObjects.remove(o);
        if (m_forwarders.contains(o)) {
            delete m_forwarders.value(o);
            m_forwarders.remove(o);
        }
        Node node = m_n.objectNodeMap.value(o);
        if (node == Node()) {
            DEBUG << "(have no node for this)" << endl;
            return;
        }
        m_n.objectNodeMap.remove(o);
        removeListNodesFor(o);
        if (m_inReload) {
            // This signal must have been emitted by a modification
            // caused by our own transactionCommitted method (see
            // similar comment about m_inCommit in that method).
            // However, we can't indiscriminately ignore it --
            // consider for example if user removed the triples for an
            // object from the store, causing us to delete the object
            // in our reload, but that object was the parent of
            // another managed object -- then we will end up here
            // because Qt deleted the child when the parent was
            // deleted, and we should take note of that.  We only want
            // to ignore events for objects we know we are
            // synchronising already.
            if (m_reloading.contains(node)) {
                DEBUG << "(by us, ignoring it)" << endl;
                return;
            }
            // ^^^ write a unit test for this!
        }
        m_deletedObjectNodes.insert(node);
        DEBUG << "ObjectMapper::objectDestroyed done" << endl;
    }

    void transactionCommitted(const ChangeSet &cs) {
        DEBUG << "ObjectMapper::transactionCommitted" << endl;
        QMutexLocker locker(&m_mutex);
        if (m_inCommit) {
            // This signal must have been emitted by a commit invoked
            // from our own commit method.  We only set m_inCommit
            // true for a period in which our own mutex is held, so if
            // it is set here, we must be in a recursive call from
            // that mutex section (noting that m_mutex is a recursive
            // mutex, otherwise we would have deadlocked).  And we
            // don't want to update on the basis of our own commits,
            // only on the basis of commits happening elsewhere.
            DEBUG << "(by us, ignoring it)" << endl;
            return;
        }
        //!!! but now what?
        m_inReload = true;
        DEBUG << "ObjectMapper::transactionCommitted: Synchronising from " << cs.size()
              << " change(s) in transaction" << endl;

#ifndef NDEBUG
        DEBUG << "ObjectMapper: before sync, node-object map contains:" << endl;
        for (ObjectLoader::NodeObjectMap::iterator i = m_n.nodeObjectMap.begin();
             i != m_n.nodeObjectMap.end(); ++i) {
            QString n;
            QObject *o = i.value();
            if (o) n = o->objectName();
            DEBUG << i.key() << " -> " << i.value() << " [" << n << "]" << endl;
        }

        DEBUG << "ObjectMapper: before sync, object-node map contains:" << endl;
        for (ObjectStorer::ObjectNodeMap::iterator i = m_n.objectNodeMap.begin();
             i != m_n.objectNodeMap.end(); ++i) {
            QString n;
            QObject *o = i.key();
            if (o) n = o->objectName();
            DEBUG << i.key() << " [" << n << "] -> " << i.value() << endl;
        }

        DEBUG << "ObjectMapper: before sync, list-node-object map contains:" << endl;
        for (ObjectLoader::NodeObjectMap::iterator i = m_n.listNodeObjectMap.begin();
             i != m_n.listNodeObjectMap.end(); ++i) {
            QString n;
            QObject *o = i.value();
            if (o) n = o->objectName();
            DEBUG << i.key() << " -> " << i.value() << " [" << n << "]" << endl;
        }
#endif
        //!!! if an object has been effectively deleted from the
        //!!! store, we can't know that without querying the store to
        //!!! discover whether any triples remain -- so we should let
        //!!! something like ObjectLoader::reload() handle deleting
        //!!! objects that have been removed from store

        foreach (const Change &c, cs) {

            Node subject = c.second.a;

            // If the subject of a change is a node for an object,
            // reload that object
            if (m_n.nodeObjectMap.contains(subject)) {
                m_reloading.insert(subject);
                continue;
            }

            // Also if the subject of a change is a list node for a
            // property of an object, reload that object.
            if (m_n.listNodeObjectMap.contains(subject)) {
                QObject *o = m_n.listNodeObjectMap.value(subject);
                DEBUG << "transactionCommitted: Node " << subject
                      << " is a list node for object " << o << endl;
                if (m_n.objectNodeMap.contains(o)) {
                    m_reloading.insert(m_n.objectNodeMap.value(o));
                }
            }
        }

        Nodes nodes;
        foreach (const Node &n, m_reloading) {
            nodes.push_back(n);
        }

        DEBUG << "transactionCommitted: Have " << nodes.size() << " node(s) to reload" << endl;

        m_loader->reload(nodes, m_n.nodeObjectMap);
        m_reloading.clear();

        // The load call will have updated m_n.nodeObjectMap; sync the
        // unchanged (since the last commit call) m_n.objectNodeMap
        // from it
        syncMap(m_n.objectNodeMap, m_n.nodeObjectMap);

        DEBUG << "ObjectMapper: after sync, object-node map contains:" << endl;
        for (ObjectStorer::ObjectNodeMap::iterator i = m_n.objectNodeMap.begin();
             i != m_n.objectNodeMap.end(); ++i) {
            QString n;
            QObject *o = i.key();
            if (o) n = o->objectName();
            DEBUG << i.key() << " [" << n << "] -> " << i.value() << endl;
        }

        m_inReload = false;
        DEBUG << "ObjectMapper::transactionCommitted done" << endl;
    }

    void doCommit(ChangeSet *cs) { 

        QMutexLocker locker(&m_mutex);
        DEBUG << "ObjectMapper::commit: Synchronising " << m_changedObjects.size()
              << " changed and " << m_deletedObjectNodes.size()
              << " deleted object(s)" << endl;
        //!!! if an object has been added as a new sibling of existing
        //!!! objects, then we presumably may have to rewrite our
        //!!! follows relationships?

        // What other objects can be affected by the addition or
        // modification of an object?

        // - Adding a new child -> affects nothing directly, as child
        // goes at end

        foreach (Node n, m_deletedObjectNodes) {
            m_storer->removeObject(n);
        }

        QObjectList ol;
        foreach (QObject *o, m_changedObjects) ol.push_back(o);
        m_storer->store(ol, m_n.objectNodeMap);

        m_inCommit = true;
        if (cs) {
            *cs = m_c.commitAndObtain();
        } else {
            m_c.commit();
        }            
        m_inCommit = false;

        // The store call will have updated m_n.objectNodeMap; sync
        // the unchanged (since the last "external" transaction)
        // m_n.nodeObjectMap from it
        syncMap(m_n.nodeObjectMap, m_n.objectNodeMap);

        // If an object has list properties, then we (sadly) need to
        // associate all of the list nodes with the object as well in
        // order to ensure that it gets reloaded if the list tail
        // changes (i.e. if something in the list changes but its head
        // node does not).  Note that we have to do this regardless of
        // whether the object is already managed -- we may have been
        // called from a reload callback.  Quite a subtle problem that
        // has rather sad efficiency implications.
        foreach (QObject *o, m_changedObjects) {
            addListNodesFor(o);
        }

        m_deletedObjectNodes.clear();
        m_changedObjects.clear();
        DEBUG << "ObjectMapper::commit done" << endl;
    }

    void commit() {
        doCommit(0);
    }

    ChangeSet commitAndObtain() {
        ChangeSet cs;
        doCommit(&cs);
        return cs;
    }

private:
    ObjectMapper *m_m;
    TransactionalStore *m_s;
    Connection m_c;
    TypeMapping m_tm;
    Network m_n;

    QMutex m_mutex;
    QHash<QObject *, ObjectMapperForwarder *> m_forwarders;
    QSet<QObject *> m_changedObjects;
    QSet<Node> m_deletedObjectNodes;

    bool m_inCommit;
    bool m_inReload;
    QSet<Node> m_reloading;

    ObjectLoader *m_loader;
    ObjectStorer *m_storer;
    LoadStoreCallback m_callback;

    class InternalMappingInconsistency : virtual public std::exception {
    public:
        InternalMappingInconsistency(QString a, QString b) throw() :
            m_a(a), m_b(b) { }
        virtual ~InternalMappingInconsistency() throw() { }
        virtual const char *what() const throw() {
            return QString("Internal inconsistency: internal map from %1 to %2 "
                           "contains different %2 from that found in map from "
                           "%2 to %1").arg(m_a).arg(m_b).toLocal8Bit().data();
        }
    protected:
        QString m_a;
        QString m_b;
    };

    void syncMap(ObjectLoader::NodeObjectMap &target,
                 ObjectStorer::ObjectNodeMap &source) {
        
        ObjectLoader::NodeObjectMap newMap;
        int inCommon = 0;

        for (ObjectStorer::ObjectNodeMap::iterator i = source.begin();
             i != source.end(); ++i) {
            
            Node n = i.value();

            if (target.contains(n)) {
                QObject *o = target.value(n);
                if (o && o != i.key()) {
                    throw InternalMappingInconsistency("Node", "QObject");
                }
                ++inCommon;
            }
            
            newMap.insert(n, i.key());
        }

        DEBUG << "syncMap: Note: resized NodeObjectMap from " << target.size()
              << " to " << newMap.size() << " element(s); " << inCommon
              << " unchanged or trivial" << endl;

        target = newMap;
    }

    void syncMap(ObjectStorer::ObjectNodeMap &target,
                 ObjectLoader::NodeObjectMap &source) {

        ObjectStorer::ObjectNodeMap newMap;
        int inCommon = 0;

        for (ObjectLoader::NodeObjectMap::iterator i = source.begin();
             i != source.end(); ++i) {
            
            QObject *o = i.value();
            if (!o) continue;
            
            if (target.contains(o)) {
                Node n(target.value(o));
                if (n != Node() && n != i.key()) {
                    throw InternalMappingInconsistency("QObject", "Node");
                }
                ++inCommon;
            }
            
            newMap.insert(o, i.key());
        }

        DEBUG << "syncMap: Note: resized ObjectNodeMap from " << target.size()
              << " to " << newMap.size() << " element(s); " << inCommon
              << " unchanged or trivial" << endl;

        target = newMap;
    }
};

ObjectMapper::ObjectMapper(TransactionalStore *s) :
    m_d(new D(this, s))
{
}

ObjectMapper::~ObjectMapper()
{
    delete m_d;
}

TransactionalStore *
ObjectMapper::getStore()
{
    return m_d->getStore();
}

void
ObjectMapper::setTypeMapping(const TypeMapping &tm)
{
    m_d->setTypeMapping(tm);
}

const TypeMapping &
ObjectMapper::getTypeMapping() const
{
    return m_d->getTypeMapping();
}

void
ObjectMapper::setBlankNodePolicy(BlankNodePolicy policy)
{
    m_d->setBlankNodePolicy(policy);
}

BlankNodePolicy
ObjectMapper::getBlankNodePolicy() const
{
    return m_d->getBlankNodePolicy();
}

Node
ObjectMapper::getNodeForObject(QObject *o) const
{
    return m_d->getNodeForObject(o);
}

QObject *
ObjectMapper::getObjectByNode(Node n) const
{
    return m_d->getObjectByNode(n);
}

QObject *
ObjectMapper::load(Node node)
{
    return m_d->load(node);
}

QObjectList
ObjectMapper::loadType(Uri type)
{
    return m_d->loadType(type);
}

void
ObjectMapper::add(QObject *o)
{
    m_d->add(o);
}

void
ObjectMapper::add(QObjectList ol)
{
    m_d->add(ol);
}

void
ObjectMapper::manage(QObject *o)
{
    m_d->manage(o);
}

void
ObjectMapper::manage(QObjectList ol)
{
    m_d->manage(ol);
}

void
ObjectMapper::unmanage(QObject *o)
{
    m_d->unmanage(o);
}

void
ObjectMapper::unmanage(QObjectList ol)
{
    m_d->unmanage(ol);
}

void
ObjectMapper::commit()
{
    m_d->commit();
}

ChangeSet
ObjectMapper::commitAndObtain()
{
    return m_d->commitAndObtain();
}

void
ObjectMapper::objectModified(QObject *o)
{
    m_d->objectModified(o);
}

void
ObjectMapper::objectDestroyed(QObject *o)
{
    m_d->objectDestroyed(o);
}

void
ObjectMapper::transactionCommitted(const ChangeSet &cs)
{
    m_d->transactionCommitted(cs);
}

}


	
