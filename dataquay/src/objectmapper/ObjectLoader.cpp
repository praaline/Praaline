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

#include "objectmapper/ObjectLoader.h"

#include "objectmapper/ObjectBuilder.h"
#include "objectmapper/ContainerBuilder.h"
#include "objectmapper/TypeMapping.h"

#include "objectmapper/ObjectMapperExceptions.h"

#include <QMetaProperty>
#include <QSet>

#include "PropertyObject.h"
#include "Store.h"

#include "../Debug.h"

namespace Dataquay {


/*
 * Generally we pass around a LoadState object recording current
 * progress and work still to do.  See LoadState for the terminology
 * used.
 *
 * We have five phases:
 *
 * 1. Collect -- given the requested node set, fill the toAllocate,
 * toInitialise, and toPopulate sets based on the FollowPolicy.
 *
 * 2. Allocate -- construct new nodes for those in toAllocate that are
 * not yet in the map.  Recurse as appropriate to parent, siblings and
 * children.  (toAllocate contains all the nodes we will need to load,
 * including those we will recurse to when assigning properties for
 * FollowObjectProperties policy later.)
 * 
 * 3. Initialise -- set the "literal" properties for each node in
 * toInitialise
 *
 * 4. Immediate callbacks -- call any registered immediate load
 * callbacks for each node (these are callbacks that may use the
 * "literal" data but should be called before related nodes are
 * loaded)

 * 5. Populate -- set all remaining properties for each node in
 * toPopulate
 *
 * 6. Final callbacks -- call any registered final load callbacks for
 * each node
 *
 * Notes
 *  
 * We should construct a new object when:
 *
 * - a node in requested does not appear in map or is null in map
 *
 * - a node called for as parent or object property does not appear in
 *   map and the relevant follows policy is set
 *
 * We should reload properties for an object node when:
 * 
 * - the node is in requested
 *
 * - the node has just been loaded
 *
 * We should delete an object when:
 * 
 * - a node in requested is in the map but not in the store
 *
 * Cycles are only a problem when loading objects -- not when setting
 * properties.  So we need to ensure that all objects that will need
 * to be loaded for object properties or parents etc are identified
 * before we start loading, then we load relationship tree (no
 * cycles), then we load property objects, then we go through setting
 * properties on the appropriate objects.
 */

class ObjectLoader::D
{
    typedef QSet<Node> NodeSet;

public:
    struct LoadState {

        LoadState() : loadFlags(0) { }

        /// Nodes the customer has explicitly asked to load or reload
        Nodes requested;

        /// Nodes whose objects will need to be constructed if possible
        NodeSet toAllocate;

        /// Nodes pending the first (literal) property assignment
        NodeSet toInitialise;

        /// Nodes pending the full property assignment
        NodeSet toPopulate;

        /// All known node-object correspondences, to be updated as we go
        NodeObjectMap map;

        enum LoadFlags {
            /// Do not throw exception if RDF type unknown (for loadAll etc)
            IgnoreUnknownTypes = 1 << 0,
        };
        unsigned int loadFlags;
    };

    D(ObjectLoader *m, Store *s) :
        m_m(m),
        m_ob(ObjectBuilder::getInstance()),
        m_cb(ContainerBuilder::getInstance()),
        m_s(s),
        m_fp(FollowNone),
        m_ap(IgnoreAbsentProperties) {
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

    void setFollowPolicy(FollowPolicy fp) {
        m_fp = fp; 
    }

    FollowPolicy getFollowPolicy() const {
        return m_fp;
    }

    void setAbsentPropertyPolicy(AbsentPropertyPolicy ap) {
        m_ap = ap;
    }

    AbsentPropertyPolicy getAbsentPropertyPolicy() const {
        return m_ap;
    }

    void updatePropertyNames() {
        m_parentProp = Uri(m_tm.getRelationshipPrefix().toString() + "parent");
        m_followProp = Uri(m_tm.getRelationshipPrefix().toString() + "follows");
    }

    QObject *load(Node node) {
        LoadState state;
        state.requested << node;
        collect(state);
        load(state);
        return state.map.value(node);
    }
    
    void reload(Nodes nodes, NodeObjectMap &map) {

        DEBUG << "reload: " << nodes << endl;
        
        LoadState state;
        state.requested = nodes;
        state.map = map;
        state.loadFlags = LoadState::IgnoreUnknownTypes;

        collect(state);
        load(state);        

        map = state.map;
    }

    QObjectList loadType(Uri type) {
        NodeObjectMap map;
        return loadType(type, map);
    }

    QObjectList loadType(Uri type, NodeObjectMap &map) {
        return loadType(Node(type), map);
    }

    QObjectList loadType(Node typeNode, NodeObjectMap &map) {

        Nodes nodes;
        
        Triples candidates = m_s->match(Triple(Node(), Uri("a"), typeNode));
        foreach (Triple t, candidates) {
            if (t.c.type != Node::URI) continue;
            nodes.push_back(t.a);
        }

        LoadState state;
        state.requested = nodes;
        state.map = map;

        collect(state);
        load(state);

        QObjectList objects;
        foreach (Node n, nodes) {
            QObject *o = map.value(n);
            if (o) objects.push_back(o);
        }
        return objects;
    }

    QObjectList loadAll() {
        NodeObjectMap map;
        return loadAll(map);
    }

    QObjectList loadAll(NodeObjectMap &map) {
        
        Nodes nodes;
        
        Triples candidates = m_s->match(Triple(Node(), Uri("a"), Node()));
        foreach (Triple t, candidates) {
            if (t.c.type != Node::URI) continue;
            nodes.push_back(t.a);
        }

        LoadState state;
        state.requested = nodes;
        state.map = map;
        state.loadFlags = LoadState::IgnoreUnknownTypes;

        collect(state);
        load(state);

        map = state.map;

        QObjectList objects;
        foreach (Node n, nodes) {
            QObject *o = map.value(n);
            if (o) objects.push_back(o);
        }
        return objects;
    }
    
    void addLoadCallback(LoadCallback *cb, LoadCallbackType type) {
        switch (type) {
        case ImmediateCallback:
            m_immediateCallbacks.push_back(cb);
            break;
        case FinalCallback:
            m_finalCallbacks.push_back(cb);
            break;
        }
    }

private:
    ObjectLoader *m_m;
    ObjectBuilder *m_ob;
    ContainerBuilder *m_cb;
    Store *m_s;
    TypeMapping m_tm;
    FollowPolicy m_fp;
    AbsentPropertyPolicy m_ap;
    QList<LoadCallback *> m_immediateCallbacks;
    QList<LoadCallback *> m_finalCallbacks;
    Uri m_parentProp;
    Uri m_followProp;

    void collect(LoadState &state) {

        Nodes candidates = state.requested;
        NodeSet visited;

        // Avoid ever pushing the nil Node as a future candidate by
        // marking it as used already

        visited << Node();
        
        // Use counter to iterate, so that when additional elements
        // pushed onto the end of state.desired will be iterated over

        for (int i = 0; i < candidates.size(); ++i) {

            Node node = candidates[i];

            visited << node;

            if (!state.map.contains(node) || state.map.value(node) == 0) {

                if (!nodeHasTypeInStore(node)) {
                    continue;
                } else {
                    state.toAllocate.insert(node);
                    state.toInitialise.insert(node);
                    state.toPopulate.insert(node);
                } 

            } else if (i < state.requested.size()) {
                
                // This is one of the requested nodes, which were at
                // the start of the candidates list

                if (!nodeHasTypeInStore(node)) {
                    DEBUG << "Node " << node
                          << " has no type in store, deleting and resetting"
                          << endl;
                    delete state.map.value(node);
                    state.map.insert(node, 0);
                    continue;
                }

                state.toInitialise.insert(node);
                state.toPopulate.insert(node);
            }

            Nodes relatives;

            if (m_fp & FollowParent) {
                relatives << parentOf(node);
            }
            if (m_fp & FollowChildren) {
                relatives << childrenOf(node);
            }
            if (m_fp & FollowSiblings) {
                relatives << prevSiblingOf(node) << nextSiblingOf(node);
            }
            if (m_fp & FollowObjectProperties) {
                relatives << potentialPropertyNodesOf(node);
            }
                
            foreach (Node r, relatives) {
                if (!visited.contains(r)) {
                    candidates << r;
                }
            }
        }

        DEBUG << "ObjectLoader: collect: "
              << "requested = " << state.requested.size()
              << ", toAllocate = " << state.toAllocate.size()
              << ", toInitialise = " << state.toInitialise.size()
              << ", toPopulate = " << state.toPopulate.size()
              << endl;

        DEBUG << "Requested:";
        foreach (Node n, state.requested) DEBUG << n;

        DEBUG << "toAllocate:";
        foreach (Node n, state.toAllocate) DEBUG << n;

        DEBUG << "toInitialise:";
        foreach (Node n, state.toInitialise) DEBUG << n;

        DEBUG << "toPopulate:";
        foreach (Node n, state.toPopulate) DEBUG << n;

        DEBUG << endl;
    }

    bool nodeHasTypeInStore(Node node) {
        Triple t = m_s->matchOnce(Triple(node, Uri("a"), Node()));
        return (t.c.type == Node::URI);
    }

    Node parentOf(Node node) {
        Triple t = m_s->matchOnce(Triple(node, m_parentProp, Node()));
        if (t != Triple()) return t.c;
        else return Node();
    }

    Nodes childrenOf(Node node) {
        Nodes nn;
        Triples tt = m_s->match(Triple(Node(), m_parentProp, node));
        foreach (Triple t, tt) nn << t.a;
        return nn;
    }

    Node prevSiblingOf(Node node) {
        Triple t = m_s->matchOnce(Triple(node, m_followProp, Node()));
        if (t != Triple()) return t.c;
        else return Node();
    }

    Node nextSiblingOf(Node node) {
        Triple t = m_s->matchOnce(Triple(Node(), m_followProp, node));
        if (t != Triple()) return t.a;
        else return Node();
    }

    Nodes orderedSiblingsOf(Node node) {
        Node current = node;
        Node prior;
        while ((prior = prevSiblingOf(current)) != Node()) {
            current = prior;
        }
        Nodes siblings;
        while (current != Node()) {
            siblings << current;
            current = nextSiblingOf(current);
        }
        return siblings;
    }    

    Nodes orderedChildrenOf(Node node) {
        // We're not certain to find follows properties for all
        // children; if some or all are missing, we still need to
        // return the right number of children -- they just won't
        // actually be ordered
        Nodes children = childrenOf(node);
        if (children.empty()) return children;
        NodeSet remaining = NodeSet::fromList(children);
        Nodes ordered = orderedSiblingsOf(children[0]);
        remaining.subtract(NodeSet::fromList(ordered));
        foreach (Node n, remaining) ordered.push_back(n);
        DEBUG << "orderedChildrenOf: Node " << node << " has " << ordered.size()
              << " children: " << ordered << endl;
        return ordered;
    }
        
    Nodes potentialPropertyNodesOf(Node node) {
        //!!! what to do about nodes that end up in candidates and so are loaded, but are never actually needed?
        Nodes nn;
        Triples tt = m_s->match(Triple(node, Node(), Node()));
        foreach (Triple t, tt) {
            if (nodeHasTypeInStore(t.c)) {
                nn << t.c;
            } else {
                Nodes sequence = sequenceStartingAt(t.c);
                foreach (Node sn, sequence) {
                    if (nodeHasTypeInStore(sn)) {
                        nn << sn;
                    }
                }
            }
        }
        return nn;
    }

    Nodes sequenceStartingAt(Node node) {

        Nodes nn;
        Triple t;

        Node itr = node;
        Node nil = m_s->expand("rdf:nil");
        
        while ((t = m_s->matchOnce(Triple(itr, m_s->expand("rdf:first"), Node())))
               != Triple()) {

            nn << t.c;

            t = m_s->matchOnce(Triple(itr, m_s->expand("rdf:rest"), Node()));
            if (t == Triple()) break;

            itr = t.c;
            if (itr == nil) break;
        }

        if (!nn.empty()) {
            DEBUG << "sequenceStartingAt " << node << " has " << nn.size() << " item(s)" << endl;
        }

        return nn;
    }

    void load(LoadState &state) {
        foreach (Node node, state.toAllocate) {
            DEBUG << "load: calling allocate(" << node << ")" << endl;
            try {
                allocate(state, node);
            } catch (UnknownTypeException &e) {
                if (state.loadFlags & LoadState::IgnoreUnknownTypes) {
                    DEBUG << "load: IgnoreUnknownTypes is set, removing object of unknown type and continuing" << endl;
                    delete state.map.value(node);
                    state.map.insert(node, 0);
                    state.toInitialise.remove(node);
                    state.toPopulate.remove(node);
                } else {
                    throw;
                }
            }
        }

        // allocate() also calls initialise(), because we want the
        // initialise to happen before children are added to a node --
        // this is just belt and braces
        foreach (Node node, state.toInitialise) {
            DEBUG << "load: calling initialise(" << node << ")" << endl;
            initialise(state, node);
        }

        // populate() removes from state.toPopulate, so we need to
        // make a note of the original set for calling load callbacks
        NodeSet tp = state.toPopulate;

        foreach (Node node, tp) {
            DEBUG << "load: calling populate(" << node << ")" << endl;
            populate(state, node);
        }
        foreach (Node node, tp) {
            DEBUG << "load: calling callLoadCallbacks(" << node << ")" << endl;
            callLoadCallbacks(state, node, m_finalCallbacks);
        }
    }

    QObject *parentObjectOf(LoadState &state, Node node) {
        
        Node parent = parentOf(node);
        QObject *parentObject = 0;

        if (parent != Node()) {
            allocate(state, parent);
            parentObject = state.map.value(parent);
        }

        return parentObject;
    }

    void allocate(LoadState &state, Node node) {

        //!!! too many of these tests, some must be redundant
        if (!state.toAllocate.contains(node)) return;

        QObject *parentObject = parentObjectOf(state, node);

        allocate(state, node, parentObject);
    }

    void allocate(LoadState &state, Node node, QObject *parentObject) {

        //!!! too many of these tests, some must be redundant
        if (!state.toAllocate.contains(node)) return;

        if (m_fp & FollowSiblings) {
            Nodes siblings = orderedSiblingsOf(node);
            foreach (Node s, siblings) {
                //!!! Hmm. Do we want to recurse to children of siblings if FollowChildren is set? Trouble is we don't want to recurse to siblings of siblings (that would lead to a cycle)
                allocateSingle(state, s, parentObject);
            }
        }

        QObject *o = allocateSingle(state, node, parentObject);

        if (state.toInitialise.contains(node)) {
            DEBUG << "load: calling initialise(" << node << ") from allocate" << endl;
            initialise(state, node);
        }

        if (m_fp & FollowChildren) {
            Nodes children = orderedChildrenOf(node);
            foreach (Node c, children) {
                allocate(state, c, o);
            }
        }
    }

    QObject *allocateSingle(LoadState &state, Node node) {
        QObject *parentObject = parentObjectOf(state, node);
        return allocateSingle(state, node, parentObject);
    }

    QObject *allocateSingle(LoadState &state, Node node, QObject *parentObject) {

        DEBUG << "allocateSingle: " << node << " (parent = " << parentObject << ")" << endl;

        //!!! too many of these tests, some must be redundant
        if (!state.toAllocate.contains(node)) {
            DEBUG << "already loaded: returning existing value (" << state.map.value(node) << ")" << endl;
            return state.map.value(node);
        }

        QObject *o = allocateObject(node, parentObject);

        DEBUG << "Setting object " << o << " to map for node " << node << endl;

        QObject *old = state.map.value(node);
        if (o != old) {
            DEBUG << "Deleting old object " << old << endl;
            delete old;
        }

        state.map.insert(node, o);
        state.toAllocate.remove(node);

        QObject *x = state.map.value(node);
        DEBUG << "New value is " << x << endl;

        return o;
    }

    void callLoadCallbacks(LoadState &state, Node node,
                           QList<LoadCallback *> &callbacks) {

        QObject *o = state.map.value(node);

        DEBUG << "callLoadCallbacks: " << node << " -> " << o << endl;

        if (!o) return;

        foreach (LoadCallback *cb, callbacks) {
            cb->loaded(m_m, state.map, node, o);
        }
    }

    QString getClassNameForNode(Node node);

    QObject *allocateObject(Node node, QObject *parent);

    void initialise(LoadState &, Node node);
    void populate(LoadState &, Node node);

    enum PropertyLoadType {
        LoadAllProperties,
        LoadLiteralProperties,
        LoadNonLiteralProperties
    };
    void loadProperties(LoadState &, Node node, PropertyLoadType);

    QVariant propertyNodeListToVariant(LoadState &, QString typeName, Nodes pnodes);
    QObject *propertyNodeToObject(LoadState &, Node pnode);
    QVariant propertyNodeToVariant(LoadState &, QString typeName, Node pnode);
    QVariantList propertyNodeToList(LoadState &, QString typeName, Node pnode);
};

void
ObjectLoader::D::initialise(LoadState &state, Node node)
{
    loadProperties(state, node, LoadLiteralProperties);
    callLoadCallbacks(state, node, m_immediateCallbacks);
    state.toInitialise.remove(node);
}

void
ObjectLoader::D::populate(LoadState &state, Node node)
{
    loadProperties(state, node, LoadNonLiteralProperties);
    state.toPopulate.remove(node);
}

void
ObjectLoader::D::loadProperties(LoadState &state, Node node,
                                PropertyLoadType loadType)
{
    QObject *o = state.map.value(node);
    if (!o) return;

    QString cname = o->metaObject()->className();

    CacheingPropertyObject *po = new CacheingPropertyObject
        (m_s, m_tm.getPropertyPrefix().toString(), node);

    QObject *defaultsObject = 0;

    for (int i = 0; i < o->metaObject()->propertyCount(); ++i) {

        QMetaProperty property = o->metaObject()->property(i);

        if (!property.isStored() ||
            !property.isReadable() ||
            !property.isWritable()) {
            continue;
        }

        QString pname = property.name(); // name to use when setting on QObject
        QString plookup(pname);          // name or URI for PropertyObject

	Uri puri;
	if (m_tm.getPropertyUri(cname, pname, puri)) {
	    plookup = puri.toString();
	}

        Nodes pnodes;
        bool haveProperty = po->hasProperty(plookup);
        if (haveProperty) {
            pnodes = po->getPropertyNodeList(plookup);
            if (pnodes.empty()) {
                haveProperty = false;
            }
        }

        if (loadType != LoadAllProperties) {
            bool literal = true;
            foreach (Node n, pnodes) {
                if (n.type != Node::Literal) {
                    literal = false;
                    break;
                }
            }
            if ( literal && (loadType == LoadNonLiteralProperties)) continue;
            if (!literal && (loadType == LoadLiteralProperties)) continue;
        }
        
        DEBUG << "For property " << pname << " of " << node << " have "
              << pnodes.size() << " node(s)" << endl;

        if (!haveProperty && m_ap == IgnoreAbsentProperties) continue;

        QVariant value;
        QByteArray pnba = pname.toLocal8Bit();

        if (!haveProperty) {
            if (!defaultsObject) {
                if (m_ob->knows(cname)) {
                    defaultsObject = m_ob->build(cname, 0);
                } else {
                    DEBUG << "Can't reset absent property " << pname
                          << " of object " << node << ": object builder "
                          << "doesn't know type " << cname << " so cannot "
                          << "build defaults object" << endl;
                }
            }

            if (defaultsObject) {
                DEBUG << "Resetting property " << pname << " to default" << endl;
                value = defaultsObject->property(pnba.data());
            }

        } else {
            QString typeName = property.typeName();
            DEBUG << "Setting property " << pname << " of type " << typeName << endl;
            value = propertyNodeListToVariant(state, typeName, pnodes);
        }

        if (!value.isValid()) {
            DEBUG << "Ignoring invalid variant for value of property "
                  << pname << ", type " << property.typeName()
                  << " of object " << node << endl;
            continue;
        }

        if (!o->setProperty(pnba.data(), value)) {
            DEBUG << "loadProperties: Property set failed "
                  << "for property " << pname << " of type "
                  << property.typeName() << " (" << property.userType()
                  << ") to value of type " << value.type() 
                  << " and value " << value
                  << " from (first) node " << pnodes[0].value
                  << endl;
            DEBUG << "loadProperties: If the RDF datatype is correct, check "
                  << "[1] that the datatype is known to Dataquay::Node for "
                  << "node-variant conversion"
                  << "(datatype is one of the standard set, "
                  << "or registered with Node::registerDatatype) and "
                  << "[2] that the Q_PROPERTY type declaration "
                  << property.typeName()
                  << " matches the name passed to qRegisterMetaType (including namespace)"
                  << endl;
            std::cerr << "ObjectLoader::loadProperties: Failed to set property on object, ignoring" << std::endl;
        }
    }

    delete defaultsObject;
    delete po;
}

QVariant
ObjectLoader::D::propertyNodeListToVariant(LoadState &state, 
                                           QString typeName, Nodes pnodes)
{
    if (pnodes.empty()) return QVariant();

    Node firstNode = pnodes[0];

    DEBUG << "propertyNodeListToVariant: typeName = " << typeName << endl;

    if (typeName == "") {
        return firstNode.toVariant();
    }

    if (m_cb->canInjectContainer(typeName)) {

        QString inContainerType = m_cb->getTypeNameForContainer(typeName);
        ContainerBuilder::ContainerKind k = m_cb->getContainerKind(typeName);

        if (k == ContainerBuilder::SequenceKind) {
            QVariantList list =
                propertyNodeToList(state, inContainerType, firstNode);
            return m_cb->injectContainer(typeName, list);

        } else if (k == ContainerBuilder::SetKind) {
            QVariantList list;
            foreach (Node pnode, pnodes) {
                Nodes sublist;
                sublist << pnode;
                list << propertyNodeListToVariant(state, inContainerType, sublist);
            }
            return m_cb->injectContainer(typeName, list);

        } else {
            return QVariant();
        }

    } else if (m_ob->canInject(typeName)) {

        QObject *obj = propertyNodeToObject(state, firstNode);
        QVariant v;
        if (obj) {
            v = m_ob->inject(typeName, obj);
            if (v == QVariant()) {
                DEBUG << "propertyNodeListToVariant: "
                      << "Type of node " << firstNode
                      << " is incompatible with expected "
                      << typeName << endl;
                std::cerr << "ObjectLoader::propertyNodeListToVariant: "
                          << "Incompatible node type, ignoring" << std::endl;
                //!!! don't delete the object without removing it from the map!
                // but which to do -- remove it, or leave it?
//                delete obj;
            }
        }
        return v;

    } else if (QString(typeName).contains("*") ||
               QString(typeName).endsWith("Star")) {
        // do not attempt to read binary pointers!
        return QVariant();

    } else {

        return propertyNodeToVariant(state, typeName, firstNode);
    }
}

QVariant
ObjectLoader::D::propertyNodeToVariant(LoadState &state,
                                       QString typeName, Node pnode)
{
    // Usually we can take the default conversion from node to
    // QVariant.  But in two cases this will fail in ways we need to
    // correct:
    // 
    // - The node is an untyped literal and the property is known to
    // have a type other than string (default conversion would produce
    // a string)
    // 
    // - The node is a URI and the property is known to have a type
    // other than Uri (several legitimate property types could
    // reasonably receive data from a URI node)
    //
    // One case should not be corrected:
    //
    // - The node has a type but it doesn't match that of the property
    //
    // We have to fail that one, because we're not in any position to
    // do general type conversion here.

    DEBUG << "propertyNodeToVariant: typeName = " << typeName << endl;

    if (typeName == "") {
        return pnode.toVariant();
    }

    bool acceptDefault = true;

    if (pnode.type == Node::URI && typeName != Uri::metaTypeName()) {

        DEBUG << "ObjectLoader::propertyNodeListToVariant: "
              << "Non-URI property is target for RDF URI, converting" << endl;
        
        acceptDefault = false;

    } else if (pnode.type == Node::Literal &&
               pnode.datatype == Uri() && // no datatype
               typeName != QMetaType::typeName(QMetaType::QString)) {

        DEBUG << "ObjectLoader::propertyNodeListToVariant: "
              << "No datatype for RDF literal, deducing from typename \""
              << typeName << "\"" << endl;

        acceptDefault = false;
    }

    if (acceptDefault) {
        return pnode.toVariant();
    }

    QByteArray ba = typeName.toLocal8Bit();
    int metatype = QMetaType::type(ba.data());
    if (metatype != 0) return pnode.toVariant(metatype);
    else return pnode.toVariant();
}

QObject *
ObjectLoader::D::propertyNodeToObject(LoadState &state, Node pnode)
{
    QObject *o = 0;

    if (pnode.type == Node::URI || pnode.type == Node::Blank) {
        o = allocateSingle(state, pnode);
    } else {
        DEBUG << "Not an object node, ignoring" << endl;
    }

    return o;
}

QVariantList
ObjectLoader::D::propertyNodeToList(LoadState &state, 
                                    QString typeName, Node pnode)
{
    Nodes sequence = sequenceStartingAt(pnode);

    QVariantList list;

    foreach (Node node, sequence) {

        Nodes vnodes;
        vnodes << node;
        QVariant value = propertyNodeListToVariant(state, typeName, vnodes);

        if (value.isValid()) {
            DEBUG << "Found value: " << value << endl;
            list.push_back(value);
        } else {
            DEBUG << "propertyNodeToList: Invalid value in list, skipping" << endl;
        }
    }

    DEBUG << "propertyNodeToList: list has " << list.size() << " item(s)" << endl;
    return list;
}

QString
ObjectLoader::D::getClassNameForNode(Node node)
{
    Uri typeUri;
    Triple t = m_s->matchOnce(Triple(node, Uri("a"), Node()));
    if (t.c.type == Node::URI) typeUri = Uri(t.c.value);

    QString className;
    if (typeUri != Uri()) {
        try {
            className = m_tm.synthesiseClassForTypeUri(typeUri);
        } catch (UnknownTypeException) {
            DEBUG << "getClassNameForNode: Unknown type URI " << typeUri << endl;
            throw;
        }
    } else {
        DEBUG << "getClassNameForNode: No type URI for " << node << endl;
        throw UnknownTypeException("");
    }
        
    if (!m_ob->knows(className)) {
        DEBUG << "ObjectLoader::getClassNameForNode: Unknown object class "
              << className << endl;
        throw UnknownTypeException(className);
    }

    return className;
}

QObject *
ObjectLoader::D::allocateObject(Node node, QObject *parent)
{
    // Note that we cannot rely on the object class from a property
    // declaration to know what type to construct.  For example, if we
    // have been called as part of a process of loading something
    // declared as container of base-class pointers, but each
    // individual object is actually of a derived class, then the RDF
    // should specify the derived class but we will only have been
    // passed the base class.  So we must use the RDF type.

    QString className = getClassNameForNode(node);
    
    DEBUG << "Making object " << node.value << " of type "
          << className << " with parent " << parent << endl;

    QObject *o = m_ob->build(className, parent);
    if (!o) throw ConstructionFailedException(className);
	
    if (node.type == Node::URI) {
        o->setProperty("uri", QVariant::fromValue(m_s->expand(node.value)));
    }

    DEBUG << "Made object: " << o << endl;

    return o;
}

ObjectLoader::ObjectLoader(Store *s) :
    m_d(new D(this, s))
{ }

ObjectLoader::~ObjectLoader()
{
    delete m_d;
}

Store *
ObjectLoader::getStore()
{
    return m_d->getStore();
}

void
ObjectLoader::setTypeMapping(const TypeMapping &tm)
{
    m_d->setTypeMapping(tm);
}

const TypeMapping &
ObjectLoader::getTypeMapping() const
{
    return m_d->getTypeMapping();
}

void
ObjectLoader::setFollowPolicy(FollowPolicy policy)
{
    m_d->setFollowPolicy(policy);
}

ObjectLoader::FollowPolicy
ObjectLoader::getFollowPolicy() const
{
    return m_d->getFollowPolicy();
}

void
ObjectLoader::setAbsentPropertyPolicy(AbsentPropertyPolicy policy)
{
    m_d->setAbsentPropertyPolicy(policy);
}

ObjectLoader::AbsentPropertyPolicy
ObjectLoader::getAbsentPropertyPolicy() const
{
    return m_d->getAbsentPropertyPolicy();
}

QObject *
ObjectLoader::load(Node node)
{
    return m_d->load(node);
}

void
ObjectLoader::reload(Nodes nodes, NodeObjectMap &map)
{
    m_d->reload(nodes, map);
}

QObjectList
ObjectLoader::loadType(Uri type)
{
    return m_d->loadType(type);
}

QObjectList
ObjectLoader::loadType(Uri type, NodeObjectMap &map)
{
    return m_d->loadType(type, map);
}

QObjectList
ObjectLoader::loadAll()
{
    return m_d->loadAll();
}

QObjectList
ObjectLoader::loadAll(NodeObjectMap &map)
{
    return m_d->loadAll(map);
}

void
ObjectLoader::addLoadCallback(LoadCallback *cb, LoadCallbackType type)
{
    m_d->addLoadCallback(cb, type);
}

}


	
