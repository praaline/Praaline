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

#ifndef _DATAQUAY_OBJECT_STORER_H_
#define _DATAQUAY_OBJECT_STORER_H_

#include "../Node.h"

#include "ObjectMapperDefs.h"

#include <QHash>

class QObject;

namespace Dataquay
{

class Store;
class TypeMapping;

/**
 * \class ObjectStorer ObjectStorer.h <dataquay/objectmapper/ObjectStorer.h>
 *
 * ObjectStorer is a storage handler capable of turning objects
 * derived from QObject into RDF triples in a Store, such that under
 * the right conditions the original objects can be recreated from the
 * store by ObjectLoader.
 *
 * See also ObjectMapper, for a class which manages a set of objects
 * and uses ObjectStorer and ObjectLoader to map changes
 * bidirectionally between object hierarchy and datastore.
 *
 * ObjectStorer typically creates a new URI for each object it stores,
 * based on the object's class name with a unique suffix.  (In some
 * cases -- where an object only exists as the value of a property of
 * another object and is not referred to elsewhere in the hierarchy --
 * it will by default be given a blank node instead of a URI.)  The
 * URI created for an object will be stored in that object as a user
 * property named "uri" of type Dataquay::Uri.  If that property
 * already exists, ObjectStorer will use its value instead of creating
 * a new URI; you can exploit this if you wish to override the
 * generated URI.  Note that ObjectStorer cannot handle objects having
 * a uri property of any type other than Dataquay::Uri.
 *
 * For each object, ObjectStorer will write an rdf:type; an RDF
 * property for each of the QObject properties of the object that have
 * STORED set to true; and RDF properties identifying the parent and
 * sibling objects if the object is part of a QObject hierarchy.  The
 * URIs used for the type and these properties can be controlled using
 * a TypeMapping object.
 *
 * ObjectStorer uses Node::fromVariant to convert QObject property
 * values to RDF literals: see Node::registerDatatype for the means to
 * add new datatypes.  Properties with subclass-of-QObject-pointer
 * values can be written recursively (see setFollowPolicy); the
 * default is to write them only if the "value" QObject already has a
 * URI.  ObjectStorer can write properties which have set and sequence
 * container types (converting sets to multiple RDF properties with
 * the same subject URI, and sequences to RDF lists) if those types
 * have been registered with ContainerBuilder.
 *
 * Finally, you can register callbacks (using addStoreCallback) to be
 * called after each object is stored, in case you wish to associate
 * more information with an object.
 *
 * ObjectStorer is primarily intended to provide a simple, open and
 * extensible storage format for small networks of
 * application-specific objects.  With TypeMapping there is some
 * flexibility to assist with creating object structures from
 * arbitrary RDF graphs, and the mechanism has been tested and
 * optimised to some degree for some millions of triples, but that way
 * could lie madness.
 *
 * ObjectStorer is re-entrant, but not thread-safe.
 */

class ObjectStorer
{
public:
    /**
     * ObjectNodeMap contains a record of the RDF node used for each
     * object.  This can be filled in a call to store() and passed to
     * subsequent calls in order to hasten lookup, avoid unnecessary
     * repeated stores, and ensure consistency for generated URIs.
     * 
     * Note that, although ObjectStorer places the URI of each object
     * in its uri property, certain objects may be written using blank
     * nodes -- those nodes can only be retrieved through this map.
     *
     * The caller is responsible for ensuring that any objects
     * subsequently deleted are also removed from this map.
     */
    typedef QHash<QObject *, Node> ObjectNodeMap;

    /**
     * Create an ObjectStorer ready to store objects to the given
     * datastore.
     */
    ObjectStorer(Store *s);
    ~ObjectStorer();

    /**
     * Retrieve the store object that was passed to the constructor.
     */
    Store *getStore();

    /**
     * Provide a TypeMapping object, which controls the URIs chosen by
     * ObjectStorer to represent object types and properties.
     * Generally if you are using ObjectStorer and ObjectLoader
     * together, you will want to use the same TypeMapping object with
     * both.
     */
    void setTypeMapping(const TypeMapping &);

    /**
     * Retrieve the current TypeMapping object.
     */
    const TypeMapping &getTypeMapping() const;

    enum PropertyStorePolicy {

        /**
         * Store only properties that differ from default object
         */
        StoreIfChanged,

        /**
         * Store all properties (if storable, readable & writable) (default)
         */
        StoreAlways
    };

    /**
     * Set the policy used to determine whether to store a property.
     *
     * If StoreIfChanged, properties will only be written if they
     * differ in value from those retrieved from a newly-constructed
     * instance of the object.  If StoreAlways, all suitable
     * properties will be written.  The default is StoreAlways.
     *
     * StoreIfChanged only works for objects that have been registered
     * with ObjectBuilder so that a default object can be constructed.
     * Any other objects will have all suitable properties written.
     *
     * In either case, only properties whose QObject property
     * definitions have all of READ, WRITE, and STORED set to true
     * will be considered suitable and stored.
     */
    void setPropertyStorePolicy(PropertyStorePolicy policy);

    /**
     * Retrieve the current policy used to determine whether to store
     * a property.
     */
    PropertyStorePolicy getPropertyStorePolicy() const;

    /**
     * Set the policy used to determine whether to give an object a
     * URI or use a blank node for it.
     *
     * If BlankNodesAsNeeded (the default), objects will be given
     * blank nodes if it appears to ObjectStorer that they do not need
     * URIs.  In practice this means that objects which are referred
     * to because they are properties of other objects and which do
     * not appear elsewhere in the list of objects being stored, do
     * not have an existing uri property, and do not have a URI node
     * allocated in an ObjectNodeMap passed to the store method, will
     * be assigned blank nodes.
     *
     * If NoBlankObjectNodes, all objects written will be given URIs.
     * These will be drawn from the object's uri property if it exists
     * and is of Dataquay::Uri type, or else invented uniquely based
     * on the object's class name.
     * 
     * If NeverUseBlankNodes, the ObjectStorer will never generate a
     * blank node -- all objects written will be given URIs, and list
     * nodes will be given URIs generated from those of their
     * contents.  With this setting, an object graph written twice
     * using the same object-node map will produce identical RDF
     * graphs.
     *!!! ^^^ write a unit test for this, and ensure that it is true
     *
     * If you are using BlankNodesAsNeeded but would prefer
     * ObjectStorer not to use a blank node for a specific object, you
     * can assign a URI in advance by setting a Dataquay::Uri to its
     * "uri" property (either a declared property or a user property).
     *
     * Note that if a blank node is used for an object, there will be
     * no way to retrieve that node through the object (no equivalent
     * of the "uri" property).  If you want to refer to the node
     * subsequently you will need to ensure you provide an
     * ObjectNodeMap to the store method to retrieve the node that was
     * generated.
     */
    void setBlankNodePolicy(BlankNodePolicy policy);

    /**
     * Retrieve the current policy used to determine whether to give
     * an object a URI or use a blank node for it.
     */
    BlankNodePolicy getBlankNodePolicy() const;

    enum FollowOption {
        FollowNone             = 0, // the default
        FollowObjectProperties = 1,
        FollowParent           = 2,
        FollowSiblings         = 4,
        FollowChildren         = 8
        // there is no FollowAll; it generally isn't a good idea
        //!!! revise this in light of two-step rework
    };
    typedef int FollowPolicy;

    /**
     * Set the policy used to determine which objects to store, based
     * on their relationship to an object whose storage is being
     * explicitly requested.
     *
     * If the policy is FollowNone, only the objects explicitly
     * requested for storage by being passed as arguments to a store()
     * method call will be stored.
     *
     * If the policy has FollowObjectProperties set, then where an
     * object has a property that is suitable for storing (see
     * setPropertyStorePolicy) and whose type is a pointer-to-object
     * class for some subclass of QObject, the object referred to by
     * that property will be stored.  Otherwise, such properties will
     * only be written where their objects have URIs available already
     * (either because they exist in the object-node map or because
     * they have a QObject property of name uri and Dataquay::Uri
     * type).
     *
     * If the policy has FollowParent set, then where an object has a
     * QObject parent, that parent will also be written.
     *
     * If the policy has FollowSiblings set, then where an object has
     * QObject siblings (i.e. the object and those other objects share
     * a parent), those siblings will also be written.
     *
     * If the policy has FollowChildren set, then where an object has
     * QObject children, those children will also be written.
     */
    void setFollowPolicy(FollowPolicy policy);
    FollowPolicy getFollowPolicy() const;

    /**
     * Store the given object and return its URI in the datastore.
     * Other objects may also be stored, depending on the FollowPolicy
     * setting.
     *
     * The object will be stored even if it already exists in the
     * store.  No other data will be changed; for example, if other
     * triples already exist with this object's URI as subject, they
     * will be left alone.  See setBlankNodePolicy for details of the
     * assignment of nodes to objects.
     */
    Uri store(QObject *o);

    /**
     * Store the given object; add the object and its node to the
     * ObjectNodeMap, and return its URI in the datastore.  Other
     * objects may also be stored, depending on the FollowPolicy
     * setting, and will be recorded in the ObjectNodeMap as well.
     *
     * The object will be stored even if it already exists in the
     * store.  No other data will be changed; for example, if other
     * triples already exist with this object's URI as subject, they
     * will be left alone.  See setBlankNodePolicy for details of the
     * assignment of nodes to objects.
     */
    Uri store(QObject *o, ObjectNodeMap &map);

    /**
     * Store the given objects.  Other objects may also be stored,
     * depending on the FollowPolicy setting.
     */
    void store(QObjectList o);

    /**
     * Store the given objects, and add them and their nodes to the
     * ObjectNodeMap.  Other objects may also be stored, depending on
     * the FollowPolicy setting.
     */
    void store(QObjectList o, ObjectNodeMap &map);

    /**
     * Remove an object from the store, given its node. This removes
     * all triples with the node as subject.  If any such triple
     * references a blank node that is not referred to elsewhere in
     * the store, all triples with that node as subject will be
     * removed as well.  If such a blank node is also the head of an
     * RDF list, the rest of the RDF list will also be removed
     * provided it fulfils the same criteria.
     */
    void removeObject(Node node);

    struct StoreCallback {
        /**
         * An object has been stored by the given ObjectStorer,
         * resulting in the given RDF node.  The object and node will
         * also be found in the ObjectNodeMap, which additionally
         * references any other objects that have been stored during
         * this storage sequence.
         */
        virtual void stored(ObjectStorer *, ObjectNodeMap &, QObject *, Node) = 0;
    };

    /**
     * Register the given callback (a subclass of the abstract
     * StoreCallback class) as providing a "stored" callback method
     * which will be called after each object is stored.
     */
    void addStoreCallback(StoreCallback *callback);

private:
    ObjectStorer(const ObjectStorer &);
    ObjectStorer &operator=(const ObjectStorer &);

    class D;
    D *m_d;
};

}

#endif
