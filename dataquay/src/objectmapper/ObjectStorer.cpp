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

#include "objectmapper/ObjectStorer.h"
#include "objectmapper/ObjectBuilder.h"
#include "objectmapper/ContainerBuilder.h"
#include "objectmapper/TypeMapping.h"

#include "PropertyObject.h"
#include "Store.h"
#include "../Debug.h"

#include <memory>

#include <QMetaProperty>
#include <QSet>

namespace Dataquay
{

class ObjectStorer::D
{
    typedef QSet<QObject *> ObjectSet;

public:
    struct StoreState {
        
        StoreState() { }

        /// Objects the customer has explicitly asked to store
        QObjectList requested;

        /// Objects needing URIs allocated
        ObjectSet toAllocate;
        
        /// Objects whose properties have not yet been stored
        ObjectSet toStore;

        /// Objects for which blank nodes shouldn't be used regardless of policy
        ObjectSet noBlanks;

        /// All known object-node correspondences, to be updated as we go
        ObjectNodeMap map;
    };

    D(ObjectStorer *m, Store *s) :
        m_m(m),
        m_ob(ObjectBuilder::getInstance()),
        m_cb(ContainerBuilder::getInstance()),
        m_s(s),
        m_psp(StoreAlways),
        m_bp(PermitBlankObjectNodes),
        m_fp(FollowNone) {
        updatePropertyNames();
    }

    Store *getStore() {
        return m_s;
    }

    void setTypeMapping(const TypeMapping &tm) {
	m_tm = tm;
        updatePropertyNames();
    }

    const TypeMapping &getTypeMapping() const {
	return m_tm;
    }

    void setPropertyStorePolicy(PropertyStorePolicy psp) {
        m_psp = psp; 
    }

    PropertyStorePolicy getPropertyStorePolicy() const {
        return m_psp;
    }

    void setBlankNodePolicy(BlankNodePolicy bp) {
        m_bp = bp; 
    }

    BlankNodePolicy getBlankNodePolicy() const {
        return m_bp;
    }

    void setFollowPolicy(FollowPolicy fp) {
        m_fp = fp; 
    }

    FollowPolicy getFollowPolicy() const {
        return m_fp;
    }

    void updatePropertyNames() {
        m_parentProp = Uri(m_tm.getRelationshipPrefix().toString() + "parent");
        m_followProp = Uri(m_tm.getRelationshipPrefix().toString() + "follows");
    }

    void removeObject(Node n) {
        Triples triples = m_s->match(Triple(n, Node(), Node()));
        foreach (Triple t, triples) {
            if (t.b.type == Node::URI) {
                removePropertyNodes(n, Uri(t.b.value));
            }
        }
        m_s->remove(Triple(Node(), Node(), n));
    }

    Uri store(QObject *o, ObjectNodeMap &map) {

        StoreState state;
        state.requested << o;
        state.map = map;

        collect(state);
        store(state);

        map = state.map;

        Node node = state.map.value(o);

        if (node.type != Node::URI) {
            // This shouldn't happen (see above)
            DEBUG << "ObjectStorer::store: Stored object node "
                  << node << " is not a URI node" << endl;
            std::cerr << "WARNING: ObjectStorer::store: No URI for stored object!" << std::endl;
            return Uri();
        } else {
            return Uri(node.value);
        }
    }

    void store(QObjectList ol, ObjectNodeMap &map) {

        StoreState state;
        state.requested = ol;
        state.map = map;

        collect(state);
        store(state);

        map = state.map;
    }

    void addStoreCallback(StoreCallback *cb) {
        m_storeCallbacks.push_back(cb);
    }

private:
    ObjectStorer *m_m;
    ObjectBuilder *m_ob;
    ContainerBuilder *m_cb;
    Store *m_s;
    TypeMapping m_tm;
    PropertyStorePolicy m_psp;
    BlankNodePolicy m_bp;
    FollowPolicy m_fp;
    QList<StoreCallback *> m_storeCallbacks;
    Uri m_parentProp;
    Uri m_followProp;

    void collect(StoreState &state) {

        QObjectList candidates = state.requested;
        state.noBlanks = ObjectSet::fromList(candidates);
        ObjectSet visited;

        // Avoid ever pushing null (if returned as absence case) as a
        // future candidate by marking it as used already

        visited << 0;

        // Use counter to iterate, so that when additional elements
        // pushed onto the end of state.desired will be iterated over

        for (int i = 0; i < candidates.size(); ++i) {

            QObject *obj = candidates[i];

            visited << obj;

            if (!state.map.contains(obj) || state.map.value(obj) == Node()) {

                state.toAllocate.insert(obj);
                state.toStore.insert(obj);

            } else if (i < state.requested.size()) {

                // This is one of the requested objects, which were at
                // the start of the candidates list.  It didn't hit
                // the previous test so it already has a node, but we
                // still need to store it as requested

                state.toStore.insert(obj);
            }

            QObjectList relatives;

            if (m_fp & FollowParent) {
                relatives << obj->parent();
            }
            if (m_fp & FollowChildren) {
                relatives << obj->children();
            }
            if (m_fp & FollowSiblings) {
                if (obj->parent()) {
                    relatives << obj->parent()->children();
                }
            }

            foreach (QObject *r, relatives) {

                if (!visited.contains(r)) {

                    DEBUG << "ObjectStorer::collect: relative " << r 
                          << " is new, listing it as candidate" << endl;
                    candidates << r;

                    // Although (in PermitBlankObjectNodes mode) we
                    // technically can use blank nodes for objects
                    // that are not referred to as properties but are
                    // accessible through the object tree, the result
                    // can be rather counterintuitive -- it means we
                    // end up with spare-looking blank nodes at top
                    // level.  Best avoided.
                    state.noBlanks << r;

                } else {
                    // We've seen this one before.  If we see any
                    // object more than once by different routes, that
                    // implies that we can't use a blank node for it
                    DEBUG << "ObjectStorer::collect: relative " << r 
                          << " has been seen more than once,"
                          << " ensuring it doesn't get a blank node" << endl;
                    state.noBlanks << r;
                }
            }

            if (m_fp & FollowObjectProperties) {

                QObjectList properties = propertyObjectsOf(obj);

                foreach (QObject *p, properties) {

                    if (!visited.contains(p)) {

                        DEBUG << "ObjectStorer::collect: property " << p 
                              << " is new, listing it as candidate" << endl;
                        candidates << p;
                        
                    } else {
                        // We've seen this one before (as above)
                        DEBUG << "ObjectStorer::collect: property " << p 
                              << " has been seen more than once,"
                              << " ensuring it doesn't get a blank node" << endl;
                        state.noBlanks << p;
                    }
                }
            }
        }
        
        DEBUG << "ObjectStorer::collect: "
              << "requested = " << state.requested.size()
              << ", toAllocate = " << state.toAllocate.size()
              << ", toStore = " << state.toStore.size()
              << endl;

        DEBUG << "Requested:";
        foreach (QObject *obj, state.requested) DEBUG << obj;

        DEBUG << "toAllocate:";
        foreach (QObject *obj, state.toAllocate) DEBUG << obj;

        DEBUG << "toStore:";
        foreach (QObject *obj, state.toStore) DEBUG << obj;

        DEBUG << endl;
    }

    QObjectList objectsOf(QVariant v) {
        
        QObjectList objects;

        const char *typeName = QMetaType::typeName(v.userType());
        if (!typeName) return objects;

        if (m_cb->canExtractContainer(typeName)) {

            QVariantList list = m_cb->extractContainer(typeName, v);
            
            foreach (QVariant member, list) {
                objects << objectsOf(member);
            }
            
        } else if (m_ob->canExtract(typeName)) {

            QObject *obj = m_ob->extract(typeName, v);
            if (obj) objects << obj;
        }

        return objects;
    }

    QObjectList propertyObjectsOf(QObject *o) {
        
        QObjectList pobjects;

        QString cname = o->metaObject()->className();

        for (int i = 0; i < o->metaObject()->propertyCount(); ++i) {

            QMetaProperty property = o->metaObject()->property(i);

            if (!property.isStored() ||
                !property.isReadable()) {
                continue;
            }

            QString pname = property.name();
            QByteArray pnba = pname.toLocal8Bit();

            if (pname == "uri") continue;

            QVariant v = o->property(pnba.data());
            if (v != QVariant()) pobjects << objectsOf(v);
        }

        return pobjects;
    }

    void store(StoreState &state) {

        foreach (QObject *obj, state.toAllocate) {
            DEBUG << "store: calling allocate(" << obj << ")" << endl;
            allocate(state, obj);//!!! can this fail, as it can in loader?
        }

        // store() removes from state.toStore, so we need to make a
        // note of the original set for calling load callbacks
        ObjectSet ts = state.toStore;

        foreach (QObject *obj, ts) {
            DEBUG << "store: calling store(" << obj << ")" << endl;
            store(state, obj);
        }

        foreach (QObject *obj, ts) {
            DEBUG << "store: calling callStoreCallbacks(" << obj << ")" << endl;
            callStoreCallbacks(state, obj);
        }
    }

    bool isStarType(const char *) const;
    bool variantsEqual(const QVariant &, const QVariant &) const;
    Uri getUriFrom(QObject *o) const;
    bool isListNode(Node n) const;

    void allocate(StoreState &state, QObject *o);
    void store(StoreState &state, QObject *o);
    void storeSingle(StoreState &state, QObject *o, Node node);

    void callStoreCallbacks(StoreState &state, QObject *o);
    void storeProperties(StoreState &state, QObject *o, Node node);
    void removeUnusedNode(Node node);
    void removePropertyNodes(Node node, Uri propertyUri, QSet<Node> *retain = 0);
    void replacePropertyNodes(Node node, Uri propertyUri, Node newValue);
    void replacePropertyNodes(Node node, Uri propertyUri, Nodes newValues);
    Nodes variantToPropertyNodeList(StoreState &state, QVariant v);
    Node objectToPropertyNode(StoreState &state, QObject *o);
    Node listToPropertyNode(StoreState &state, QVariantList list);
};

bool
ObjectStorer::D::isStarType(const char *typeName) const
{
    // equivalent of: 
    //     QString(typeName).contains("*") ||
    //     QString(typeName).endsWith("Star")

    int i;
    for (i = 0; typeName[i]; ++i) {
        if (typeName[i] == '*') return true;
    }
    // i is now equal to length of typeName
    if (i >= 4 && !strcmp(typeName + i - 4, "Star")) return true;
    return false;
}

bool
ObjectStorer::D::variantsEqual(const QVariant &v1, const QVariant &v2) const
{
    if (v1 == v2) return true;
    if (v1.userType() != v2.userType()) return false;
    
    // "In the case of custom types, their equalness operators are not
    // called. Instead the values' addresses are compared."  This is
    // acceptable for objects, but not sufficient to establish
    // equality for our purposes for those things that can in fact be
    // converted to Node.

    const char *typeName = QMetaType::typeName(v1.userType());
    if (!typeName ||
        isStarType(typeName) ||
        m_cb->canExtractContainer(typeName) ||
        m_ob->canExtract(typeName)) {
        // Objects, containers, weird unknown types: give up on these.
        // An occasional false negative is OK for us -- we claim that
        // the variants are equal if we return true, not that they
        // differ if we return false -- we are already doing better
        // than QVariant::operator== anyway
        return false;
    }

    Node n1 = Node::fromVariant(v1);
    Node n2 = Node::fromVariant(v2);
    DEBUG << "variantsEqual: comparing " << n1 << " and " << n2 << endl;
    return (n1 == n2);
}

Uri
ObjectStorer::D::getUriFrom(QObject *o) const
{
    Uri uri;
    QVariant uriVar = o->property("uri");
    
    if (uriVar != QVariant()) {
        if (Uri::hasUriType(uriVar)) {
            uri = uriVar.value<Uri>();
        } else {
            uri = m_s->expand(uriVar.toString());
        }
    }

    return uri;
}

void
ObjectStorer::D::storeProperties(StoreState &state, QObject *o, Node node)
{
    QString cname = o->metaObject()->className();
    PropertyObject po(m_s, m_tm.getPropertyPrefix().toString(), node);

    for (int i = 0; i < o->metaObject()->propertyCount(); ++i) {

        QMetaProperty property = o->metaObject()->property(i);

        if (!property.isStored() ||
            !property.isReadable()) {
            continue;
        }

        QString pname = property.name();
        QByteArray pnba = pname.toLocal8Bit();

        if (pname == "uri") continue;

        QVariant value = o->property(pnba.data());

        bool store = true;

        if (m_psp == StoreIfChanged) {
            if (m_ob->knows(cname)) {
                std::auto_ptr<QObject> c(m_ob->build(cname, 0));
                QVariant deftValue = c->property(pnba.data());
                if (variantsEqual(value, deftValue)) {
                    store = false;
                } else {
                    DEBUG << "Property " << pname << " of object "
                          << node << " is changed from default value "
                          << c->property(pnba.data()) << ", writing" << endl;
                }
            } else {
                DEBUG << "Can't check property " << pname << " of object "
                      << node << " for change from default value: "
                      << "object builder doesn't know type " << cname
                      << " so cannot build default object" << endl;
            }
        }

        if (store) {
            DEBUG << "For object " << node << " (" << o << ") writing property " << pname << " of type " << property.userType() << endl;
        }

        Uri puri;
	if (!m_tm.getPropertyUri(cname, pname, puri)) {
            puri = po.getPropertyUri(pname);
        }

        if (store) {

            Nodes pnodes = variantToPropertyNodeList(state, value);
            replacePropertyNodes(node, puri, pnodes);

        } else {

            removePropertyNodes(node, puri);
        }
    }
}            
            
void
ObjectStorer::D::removePropertyNodes(Node node, Uri propertyUri, QSet<Node> *retain) 
{
    Triple t(node, propertyUri, Node());
    Triples m(m_s->match(t));
    DEBUG << "removePropertyNodes: Node " << node << " and property "
          << propertyUri << " yields " << m.size() << " matching triples"
          << endl;
    foreach (t, m) {
        if (retain && retain->contains(t.c)) {
            DEBUG << "...retaining " << t.c << endl;
            retain->remove(t.c);
        } else {
            DEBUG << "...removing " << t.c << endl;
            m_s->remove(t);
            if (t.c == node) continue;
            // If this is a blank node, or if it is a node only used
            // as a list head, then we can safely remove it.  Formerly
            // we removed only blank nodes here (list nodes are most
            // usually blank anyway), but that means we don't replace
            // list properties correctly with NeverUseBlankNodes
            if (t.c.type == Node::Blank) {
                removeUnusedNode(t.c);
            } else if (isListNode(t.c)) {
                removeUnusedNode(t.c);
            }
        }
    }
}

bool
ObjectStorer::D::isListNode(Node n) const
{
    Triples ts = m_s->match(Triple(n, Node(), Node()));

    bool isList = false;

    DEBUG << "isListNode: Testing node " << n << endl;

    // A list head should have only rdf:first and rdf:rest properties
    if (ts.size() == 2) {
        Node first = m_s->expand("rdf:first");
        Node rest = m_s->expand("rdf:rest");
        if ((ts[0].b == first || ts[1].b == rest) ||
            (ts[0].b == rest || ts[1].b == first)) {
            isList = true;
        }
    }

    DEBUG << "isListNode: isList = " << isList << endl;

    return isList;
}

void
ObjectStorer::D::replacePropertyNodes(Node node, Uri propertyUri, Node newValue)
{
    QSet<Node> nodeSet;
    nodeSet << newValue;
    removePropertyNodes(node, propertyUri, &nodeSet);
    // nodeSet now contains only those nodes whose triples need to be
    // added, i.e. those not present as our properties before
    if (!nodeSet.empty()) {
        m_s->add(Triple(node, propertyUri, newValue));
    }
}

void
ObjectStorer::D::replacePropertyNodes(Node node, Uri propertyUri, Nodes newValues)
{
    QSet<Node> nodeSet = QSet<Node>::fromList(newValues);
    removePropertyNodes(node, propertyUri, &nodeSet);
    // nodeSet now contains only those nodes whose triples need to be
    // added, i.e. those not present as our properties before
    foreach (Node pn, nodeSet) {
        m_s->add(Triple(node, propertyUri, pn));
    }
}

void
ObjectStorer::D::removeUnusedNode(Node node)
{
    // The node is known to be a blank node or a list head.  If it is
    // not referred to by anything else, then we want to remove
    // everything it refers to.  If it happens to be a list node, then
    // we also want to recurse to its tail.

    if (m_s->matchOnce(Triple(Node(), Node(), node)) != Triple()) {
        // The node is still a target of some predicate, leave it alone
        DEBUG << "removeUnusedNode: Blank or list node " << node
              << " is still a target for another predicate, leaving" << endl;
        return;
    }

    DEBUG << "removeUnusedNode: Blank or list node " << node
          << " is not target for any other predicate" << endl;

    // check for a list tail (query first, but then delete our own
    // triples first so that the existence of this rdf:rest
    // relationship isn't seen as a reason not to delete the tail)
    Triples tails(m_s->match(Triple(node, m_s->expand("rdf:rest"), Node())));

    DEBUG << "... removing everything with it as subject" << endl;
    m_s->remove(Triple(node, Node(), Node()));

    foreach (Triple t, tails) {
        DEBUG << "... recursing to list tail" << endl;
        removeUnusedNode(t.c);
    }
}

Nodes
ObjectStorer::D::variantToPropertyNodeList(StoreState &state, QVariant v)
{
    const char *typeName = QMetaType::typeName(v.userType());
    Nodes nodes;
    if (!typeName) {
        std::cerr << "ObjectStorer::variantToPropertyNodeList: No type name?! Going ahead anyway" << std::endl;
        nodes << Node::fromVariant(v);
        return nodes;
    }

    DEBUG << "variantToPropertyNodeList: typeName = " << typeName << endl;
        
    if (m_cb->canExtractContainer(typeName)) {

        QVariantList list = m_cb->extractContainer(typeName, v);
        ContainerBuilder::ContainerKind k = m_cb->getContainerKind(typeName);

        if (k == ContainerBuilder::SequenceKind) {
            Node node = listToPropertyNode(state, list);
            if (node != Node()) nodes << node;
                
        } else if (k == ContainerBuilder::SetKind) {
            foreach (QVariant member, list) {
                nodes += variantToPropertyNodeList(state, member);
            }
        }
            
    } else if (m_ob->canExtract(typeName)) {

        QObject *obj = m_ob->extract(typeName, v);
        if (obj) {
            Node n = objectToPropertyNode(state, obj);
            if (n != Node()) nodes << n;
        } else {
            DEBUG << "variantToPropertyNodeList: Note: obtained NULL object from variant" << endl;
        }
            
    } else if (isStarType(typeName)) {

        // do not attempt to write binary pointers!
        DEBUG << "variantToPropertyNodeList: Note: Ignoring pointer type "
              << typeName
              << " that is unknown to container and object builders" << endl;
        return Nodes();

    } else {
        Node node = Node::fromVariant(v);
        if (node != Node()) nodes << node;
    }

    return nodes;
}

Node
ObjectStorer::D::objectToPropertyNode(StoreState &state, QObject *o)
{
    Node pnode;

    DEBUG << "objectToPropertyNode: " << o << endl;

    if (state.map.contains(o) && state.map.value(o) != Node()) {
        pnode = state.map.value(o);
    } else {
        // if the object is not intended to be stored, but it has a
        // URI, we should nonetheless write a reference to that -- but
        // not put it in the map
        Uri uri = getUriFrom(o);
        if (uri != Uri()) {
            pnode = Node(uri);
            return pnode;
        }
    }

    return pnode;
}

Node
ObjectStorer::D::listToPropertyNode(StoreState &state, QVariantList list)
{
    DEBUG << "listToPropertyNode: have " << list.size() << " items" << endl;

    Node node, first, previous;

    foreach (QVariant v, list) {

        Nodes pnodes = variantToPropertyNodeList(state, v);
        if (pnodes.empty()) {
            std::cerr << "WARNING: ObjectStorer::listToPropertyNode: Obtained nil Node in list" << std::endl;
            continue;
        } else if (pnodes.size() > 1) {
            std::cerr << "WARNING: ObjectStorer::listToPropertyNode: Found set within sequence, can't handle this, using first element only" << std::endl;
        }

        Node pnode = pnodes[0];

        if (m_bp != NeverUseBlankNodes) {
            node = m_s->addBlankNode();
        } else {
            //!!! This is a hack -- we can give a list node a stable
            //!!! (and more attractive) URI by deriving it from the
            //!!! node it contains -- but we have no way to ensure
            //!!! that this is unique... hm
            if (pnode.type == Node::URI) {
                node = Node(Uri(pnode.value + "_listnode"));
            } else {
                node = Node(m_s->getUniqueUri(":listnode_"));
            }
        }

        if (first == Node()) first = node;

        if (previous != Node()) {
            m_s->remove(Triple(previous, m_s->expand("rdf:rest"), Node()));
            m_s->add(Triple(previous, m_s->expand("rdf:rest"), node));
        }

        m_s->remove(Triple(node, m_s->expand("rdf:first"), Node()));
        m_s->add(Triple(node, m_s->expand("rdf:first"), pnode));
        previous = node;
    }

    if (node != Node()) {
        m_s->remove(Triple(node, m_s->expand("rdf:rest"), Node()));
        m_s->add(Triple(node, m_s->expand("rdf:rest"), m_s->expand("rdf:nil")));
    }

    return first;
}

void
ObjectStorer::D::store(StoreState &state, QObject *o)
{
    DEBUG << "ObjectStorer::store: Examining " << o << endl;

    if (!state.toStore.contains(o)) return;

    Node node = state.map.value(o);
    if (node == Node()) {
        // We don't allocate any nodes here -- that happened already
        DEBUG << "ObjectStorer::store: Strange -- object " << o << " has no node, why wasn't it allocated?" << endl;
        return;
    }

    storeSingle(state, o, node);

    DEBUG << "ObjectStorer::store: Object " << o << " has node " << node << endl;

    // If we have a parent with a node, write a parent property; if we
    // are following siblings, we need to write a follow property to
    // the prior sibling.  We don't need to worry about actually
    // storing the sibling, parents or children -- they will already
    // be in the toStore list if we're supposed to be storing them
    
    QObject *parent = o->parent();

    if (parent) {
        Node pn = state.map.value(parent);
        if (pn != Node()) {
            replacePropertyNodes(node, m_parentProp, pn);
        }
    } else {
        DEBUG << "ObjectStorer::store: Node " << node
              << " has no parent" << endl;
        m_s->remove(Triple(node, m_parentProp, Node()));
    }

    bool followsWritten = false;

    if (parent && (m_fp & FollowSiblings)) {
            
        QObjectList siblings = parent->children();

        // find previous sibling (laboriously)

        QObject *previous = 0;
        for (int i = 0; i < siblings.size(); ++i) {
            if (siblings[i] == o) {
                if (i > 0) {
                    previous = siblings[i-1];
                }
                break;
            }
        }
        
        if (previous) {

            DEBUG << "ObjectStorer::store: Node " << node
                  << " follows sibling object " << previous << endl;

            Node sn = state.map.value(previous);

            if (sn != Node()) {

                DEBUG << "ObjectStorer::store: Node " << node
                      << " follows sibling " << sn << endl;
                replacePropertyNodes(node, m_followProp, sn);
                followsWritten = true;

            } else {
                // previous sibling has no node
                std::cerr << "Internal error: FollowSiblings set, but previous sibling has not been allocated" << std::endl;
            }
        } else {
            // no previous sibling
            DEBUG << "ObjectStorer::store: Node " << node
                  << " is first child, follows nothing" << endl;
        }
    }

    if (!followsWritten) {
        m_s->remove(Triple(node, m_followProp, Node()));
    }

    DEBUG << "store: Finished with " << o << endl;
}

void
ObjectStorer::D::allocate(StoreState &state, QObject *o)
{
    DEBUG << "allocate " << o << endl;

    //!!! too many of these tests, some must be redundant
    Node node = state.map.value(o);
    if (node != Node()) {
        DEBUG << "allocate: object " << o << " already has node " << node << endl;
        return;
    }

    Uri uri = getUriFrom(o);
    if (uri != Uri()) {
        node = Node(uri);
        state.map.insert(o, node);
        state.toAllocate.remove(o);
        DEBUG << "allocate: object " << o << " has known uri " << uri << endl;
        return;
    }

    if (m_bp == PermitBlankObjectNodes && !state.noBlanks.contains(o)) {

        node = m_s->addBlankNode();

        DEBUG << "allocate: object " << o << " being given new blank node " << node << endl;

    } else {
        QString className = o->metaObject()->className();
        DEBUG << "className = " << className << endl;
        Uri prefix;
        if (!m_tm.getUriPrefixForClass(className, prefix)) {
            //!!! put this in TypeMapping?
            QString tag = className.toLower() + "_";
            tag.replace("::", "_");
            prefix = m_s->expand(":" + tag);
        }
        Uri uri = m_s->getUniqueUri(prefix.toString());
        o->setProperty("uri", QVariant::fromValue(uri));
        node = uri;

        DEBUG << "allocate: object " << o << " being given new URI " << uri << endl;
    }

    state.map.insert(o, node);
    state.toAllocate.remove(o);
}

void
ObjectStorer::D::storeSingle(StoreState &state, QObject *o, Node node)
{
    QString className = o->metaObject()->className();
    m_s->add(Triple(node, Uri("a"), m_tm.synthesiseTypeUriForClass(className)));
    storeProperties(state, o, node);
    state.toStore.remove(o);
}

void
ObjectStorer::D::callStoreCallbacks(StoreState &state, QObject *o)
{
    Node node = state.map.value(o);
    foreach (StoreCallback *cb, m_storeCallbacks) {
        cb->stored(m_m, state.map, o, node);
    }
}

ObjectStorer::ObjectStorer(Store *s) :
    m_d(new D(this, s))
{ }

ObjectStorer::~ObjectStorer()
{
    delete m_d;
}

Store *
ObjectStorer::getStore()
{
    return m_d->getStore();
}

void
ObjectStorer::setTypeMapping(const TypeMapping &tm)
{
    m_d->setTypeMapping(tm);
}

const TypeMapping &
ObjectStorer::getTypeMapping() const
{
    return m_d->getTypeMapping();
}

void
ObjectStorer::setPropertyStorePolicy(PropertyStorePolicy policy)
{
    m_d->setPropertyStorePolicy(policy);
}

ObjectStorer::PropertyStorePolicy
ObjectStorer::getPropertyStorePolicy() const
{
    return m_d->getPropertyStorePolicy();
}

void
ObjectStorer::setBlankNodePolicy(BlankNodePolicy policy)
{
    m_d->setBlankNodePolicy(policy);
}

BlankNodePolicy
ObjectStorer::getBlankNodePolicy() const
{
    return m_d->getBlankNodePolicy();
}

void
ObjectStorer::setFollowPolicy(FollowPolicy policy)
{
    m_d->setFollowPolicy(policy);
}

ObjectStorer::FollowPolicy
ObjectStorer::getFollowPolicy() const
{
    return m_d->getFollowPolicy();
}

void
ObjectStorer::removeObject(Node n)
{
    m_d->removeObject(n);
}

Uri
ObjectStorer::store(QObject *o)
{
    ObjectNodeMap map;
    return m_d->store(o, map);
}

Uri
ObjectStorer::store(QObject *o, ObjectNodeMap &map)
{
    return m_d->store(o, map);
}

void
ObjectStorer::store(QObjectList o)
{
    ObjectNodeMap map;
    m_d->store(o, map);
}

void
ObjectStorer::store(QObjectList o, ObjectNodeMap &map)
{
    m_d->store(o, map);
}

void
ObjectStorer::addStoreCallback(StoreCallback *cb)
{
    m_d->addStoreCallback(cb);
}

}

