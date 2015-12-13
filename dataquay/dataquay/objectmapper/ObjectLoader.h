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

#ifndef _DATAQUAY_OBJECT_LOADER_H_
#define _DATAQUAY_OBJECT_LOADER_H_

#include "../Node.h"

#include <QHash>
#include <QPointer>

class QObject;

namespace Dataquay
{

class Store;
class TypeMapping;

/**
 * \class ObjectLoader ObjectLoader.h <dataquay/objectmapper/ObjectLoader.h>
 *
 * ObjectLoader constructs objects corresponding to nodes in the RDF
 * store and sets properties on those objects corresponding to the
 * node's RDF properties.  The class of each object is based on the
 * node's RDF type.  TypeMapping is used to relate node types to
 * object classes, and ObjectBuilder is used to construct the objects
 * (which must be subclasses of QObject).
 * 
 * In addition to some specification of which nodes to load,
 * ObjectLoader methods may also take a reference to a NodeObjectMap
 * in which is stored the object corresponding to each loaded node.
 * This map may be used by the caller as a persistent record of
 * node-object relationships, as it is updated on each new
 * ObjectLoader call with any unaffected nodes remaining unchanged in
 * the map.
 *
 * By default, ObjectLoader loads only those objects passed in to each
 * load() or reload() call.  ObjectLoader sets as many QObject
 * properties on each object as possible, given the information
 * available to it:
 *
 * \li Properties with non-object-type values will be assigned from
 * RDF properties with literal value nodes, provided Node::toVariant
 * is able to carry out the conversion from literal;
 *
 * \li Properties with object-type values will be assigned from RDF
 * properties with URI value nodes, provided those URI nodes have
 * corresponding objects available to ObjectLoader, i.e. also in the
 * set being loaded or in the NodeObjectMap.  (However, see also
 * FollowObjectProperties below.)
 * 
 * \li Properties whose value types are sequenced containers such as
 * QList or std::vector will be assigned from RDF properties with
 * sequence values, provided their container types have been
 * registered with ContainerBuilder;
 *
 * \li Properties whose value types are set containers such as QSet
 * will be assigned from the aggregation of all RDF properties with
 * the appropriate subject and predicate, provided their container
 * types have been registered with ContainerBuilder.
 *
 * Some behaviour can be adjusted using setFollowPolicy and
 * setAbsentPropertyPolicy, as follows:
 *
 * \li \c FollowPolicy is a set of flags describing how ObjectLoader
 * should recurse from each object to those related to it.  It can be
 * used to cause ObjectLoader to load more objects than are explicitly
 * requested.  The flag \c FollowObjectProperties causes objects to be
 * loaded whenever they are required as the values of properties on
 * other objects in the loaded set.  The flags \c FollowParent, \c
 * FollowSiblings and \c FollowChildren cause object tree
 * relationships to be followed up, using the "parent" and "follow"
 * properties with the TypeMapping's relationship prefix to determine
 * family relationships.
 *
 * \li \c AbsentPropertyPolicy determines how ObjectLoader handles
 * properties of an object that have no definition in the RDF store.
 * These properties are ignored if IgnoreAbsentProperties (the
 * default) is set, but if ResetAbsentProperties is set ObjectLoader
 * will attempt to reset each property to its default value based on
 * the value found in a freshly-constructed default instance of the
 * object class in question.
 *
 * The load procedure follows a defined order:
 *
 * \li The requested objects, and any relatives required by the
 * FollowPolicy, are constructed with their default properties (no
 * properties assigned from RDF yet).  If the FollowPolicy includes
 * FollowParents or FollowSiblings, these will be followed before the
 * current object is loaded; if FollowChildren, they will be followed
 * afterwards;
 *
 * \li After all objects have been constructed, those properties that
 * have "simple" RDF literal values are assigned for each object;
 *
 * \li Next any immediate callbacks registered with addLoadCallback
 * are called for each object that has been loaded (i.e. any object
 * that was either constructed or assigned to).  Use immediate
 * callbacks if you need to do any further initialisation work on an
 * object before its neighbours or property objects are loaded.
 *
 * \li After all "simple" properties have been assigned, any further
 * properties are set (those with container and object types);
 *
 * \li Finally, any final callbacks registered with addLoadCallback
 * are called for each object that has been loaded (i.e. any object
 * that was either constructed or assigned to).
 *
 * Note that ObjectLoader always maintains a one-to-one correspondence
 * between QObjects and the RDF nodes that it loads as QObjects.  In
 * particular, where multiple objects have properties that refer to
 * the same URI, no more than a single value object will be
 * constructed and the same value object will be assigned to all of
 * those objects' properties.  This implies that objects to be loaded
 * using ObjectLoader should be designed so that they do not attempt
 * to "own" (control lifecycle for) any other QObjects that appear as
 * their properties.  Ownership must be maintained separately from the
 * property relationship.
 *
 * ObjectLoader is re-entrant, but not thread-safe.
 */
 
class ObjectLoader
{
public:
    /// Map from RDF node to object
    typedef QHash<Node, QPointer<QObject> > NodeObjectMap;

    /**
     * Create an ObjectLoader ready to load objects from the given RDF
     * store.
     */
    ObjectLoader(Store *s);
    ~ObjectLoader();

    Store *getStore();

    void setTypeMapping(const TypeMapping &);
    const TypeMapping &getTypeMapping() const;

    //!!! document this (after ObjectStorer) -- or pull it into ObjectMapperDefs
    enum FollowOption {
        FollowNone             = 0, // the default
        FollowObjectProperties = 1,
        FollowParent           = 2,
        FollowSiblings         = 4,
        FollowChildren         = 8
    };
    typedef int FollowPolicy;

    void setFollowPolicy(FollowPolicy policy);
    FollowPolicy getFollowPolicy() const;

    enum AbsentPropertyPolicy {
        IgnoreAbsentProperties,
        ResetAbsentProperties
    };

    void setAbsentPropertyPolicy(AbsentPropertyPolicy policy);
    AbsentPropertyPolicy getAbsentPropertyPolicy() const;

    /**
     * Construct a QObject based on the properties of the given object
     * URI in the object mapper's store.  The type of class created
     * will be calculated from the rdf:type for the URI, using the
     * current TypeMapping (see TypeMapping::getClassForTypeUri).
     *
     * Use caution in calling this method when the FollowPolicy is set
     * to anything other than FollowNone.  Other objects may be loaded
     * when following connections from the given node according to the
     * current FollowPolicy, but only the object initially requested
     * is actually returned from the function -- other objects loaded
     * may be only accessible as parent/child or properties of this
     * node, or in some cases (e.g. FollowSiblings) may even be
     * inaccessible to the caller and be leaked.
     */
    QObject *load(Node node);

    /**
     * For each node of the given RDF type found in the store,
     * construct a corresponding QObject, returning the objects.
     */
    QObjectList loadType(Uri type);

    /**
     * For each node of the given RDF type found in the store,
     * construct a corresponding QObject, updating the map with all
     * resulting node-object correspondences and returning the
     * objects.
     */
    QObjectList loadType(Uri type, NodeObjectMap &map);
    
    /**
     * Examine each of the nodes passed in, and if there is no
     * corresponding object in the node-object map, load the node as a
     * new QObject and place it in the map; if there is a
     * corresponding object in the node-object map, update it with
     * current properties from the store.  If a node is passed in that
     * does not exist in the store, delete any object associated with
     * it from the map.
     */
    void reload(Nodes nodes, NodeObjectMap &map);

    /**
     * Load and return an object for each node in the store that can
     * be loaded.
     */
    QObjectList loadAll();

    /**
     * Load and return an object for each node in the store that can
     * be loaded, updating the map with all resulting node-object
     * correspondences.  Note that this loads all suitably-typed nodes
     * found in the store, not the objects found in the map.  If there
     * are nodes in the map which are not found in the store, they
     * will be ignored (and not deleted from the map).
     */
    QObjectList loadAll(NodeObjectMap &map);
    
    struct LoadCallback {
        /**
         * An object has been loaded by the given ObjectLoader from
         * the given RDF node.  The node and object will also be found
         * in the NodeObjectMap, which additionally references any
         * other objects which have been loaded during this load
         * sequence.
         */
        virtual void loaded(ObjectLoader *, NodeObjectMap &, Node, QObject *) = 0;
    };

    /**
     * Type of a load callback.  Immediate callbacks are called after
     * each node's literal properties have been assigned but before
     * any child, sibling, property etc relationships are followed.
     * Final callbacks are called after all work has been done on all
     * nodes and the graph is complete.
     */
    enum LoadCallbackType {
        ImmediateCallback,
        FinalCallback
    };

    /**
     * Register the given callback (a subclass of the abstract
     * LoadCallback class) as providing a "loaded" callback method
     * which will be called after each object is loaded.
     */
    void addLoadCallback(LoadCallback *callback,
                         LoadCallbackType type);

private slots:
    void objectDestroyed(QObject *);

private:
    ObjectLoader(const ObjectLoader &);
    ObjectLoader &operator=(const ObjectLoader &);
    class D;
    D *m_d;
};

}

#endif
