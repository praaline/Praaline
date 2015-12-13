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

#ifndef _DATAQUAY_PROPERTY_OBJECT_H_
#define _DATAQUAY_PROPERTY_OBJECT_H_

#include <QString>
#include <QStringList>
#include <QHash>
#include <QVariant>
#include <QVariantList>

#include "Node.h"

namespace Dataquay
{

class Transaction;
class Store;

/**
 * \class PropertyObject PropertyObject.h <dataquay/PropertyObject.h>
 *
 * PropertyObject is a helper class for managing RDF properties of an
 * object URI -- that is, triples that share a common subject and
 * possibly a common prefix for the predicate, and that have only one
 * value for each subject-predicate combination.  This could be of use
 * in situations where properties of a single object URI are referred
 * to often.  This class provides set and get methods that act
 * directly upon the backing datastore, optionally using a
 * transaction.  See CacheingPropertyObject for a cacheing
 * alternative.
 *
 * PropertyObject is constructed using a "property prefix" (a string)
 * and "my URI" (a URI).  The URI is used by the property object as
 * the subject for all RDF triples.
 * 
 * All the property handling methods then also take a "property name",
 * which is a string.  If this name contains no ':' character, it will
 * be prefixed with the property prefix that was supplied to the
 * PropertyObject constructor before being subjected to prefix
 * expansion in the RDF store.  The result is then used as the
 * predicate for the RDF triple.  If the name does contain a ':', it
 * is passed for expansion directly (the prefix is not prepended
 * first).  As an exception, if the prefix is the special name "a",
 * it will be expanded (by the store) as "rdf:type".
 *
 * Example: If the property prefix is "myprops:" and the property name
 * passed to getProperty is "some_property", the returned value from
 * getProperty will be the result of matching on the triple (myUri,
 * "myprops:some_property", ()).  Hopefully, the RDF store will have
 * already been told about the "myprops" prefix and will know how to
 * expand it.
 *
 * Example: If the property prefix is "http://example.com/property/"
 * and the property name passed to getProperty is "some_property", the
 * returned value from getProperty will be the result of matching on
 * the triple (myUri, "http://example.com/property/some_property", ()).
 *
 * Example: If the property prefix is "myprops:" and the property name
 * passed to getProperty is "yourprops:some_property", the returned
 * value from getProperty will be the result of matching on the triple
 * (myUri, "yourprops:some_property", ()).  The property prefix is not
 * used at all in this example because the property name contains ':'.
 */
class PropertyObject
{
public:
    /**
     * Construct a PropertyObject acting on the given Store, with the
     * default prefix for properties taken from the global default
     * (see setDefaultPropertyPrefix) and the given "subject" URI.
     */
    PropertyObject(Store *s, Uri myUri);

    /**
     * Construct a PropertyObject acting on the given Store, with the
     * default prefix for properties taken from the global default
     * (see setDefaultPropertyPrefix) and the given "subject" URI
     * (which will be prefix expanded).
     */
    PropertyObject(Store *s, QString myUri);

    /**
     * Construct a PropertyObject acting on the given Store, with the
     * default prefix for properties taken from the global default
     * (see setDefaultPropertyPrefix) and the given "subject" node.
     * This is provided so as to permit querying the properties of
     * blank nodes or nodes returned from other queries.
     */
    PropertyObject(Store *s, Node myNode);

    /**
     * Construct a PropertyObject acting on the given Store, with the
     * given default prefix (which will itself be prefix expanded) for
     * properties and the given "subject" URI.
     */
    PropertyObject(Store *s, QString propertyPrefix, Uri myUri);

    /**
     * Construct a PropertyObject acting on the given Store, with the
     * given default prefix for properties and the given "subject"
     * URI (which will be prefix expanded).
     */
    PropertyObject(Store *s, QString propertyPrefix, QString myUri);

    /**
     * Construct a PropertyObject acting on the given Store, with the
     * given default prefix for properties and the given node as its
     * subject.  This is provided so as to permit querying the
     * properties of blank nodes or nodes returned from other queries.
     */
    PropertyObject(Store *s, QString propertyPrefix, Node myNode);

    ~PropertyObject();

    /**
     * Return the node passed to the constructor (or derived from the
     * URI passed to the constructor).
     */
    Node getNode() const;

    /**
     * Return the rdf:type of my URI, if any.  If more than one is
     * defined, return the first one found.
     */
    Uri getObjectType() const;

    /**
     * Return the rdf:type of my URI, if any, querying through the
     * given transaction.  If more than one is defined, return the
     * first one found.
     */
    Uri getObjectType(Transaction *tx) const;

    /**
     * Return true if the property object has the given property.  That
     * is, if the store contains at least one triple whose subject and
     * predicate match those for my URI and the expansion of the given
     * property name.
     */
    bool hasProperty(QString name) const;

    /**
     * Return true if the property object has the given property,
     * querying through the given transaction.  That is, if the store
     * contains at least one triple whose subject and predicate match
     * those for my URI and the expansion of the given property name.
     */
    bool hasProperty(Transaction *tx, QString name) const;

    /**
     * Get the value of the given property.  That is, if the store
     * contains at least one triple whose subject and predicate match
     * those for my URI and the expansion of the given property name,
     * convert the object part of the first such matching triple to a
     * QVariant via Node::toVariant and return that value.  If there
     * is no such match, return QVariant().
     */
    QVariant getProperty(QString name) const;

    /**
     * Get the value of the given property, querying through the given
     * transaction.  That is, if the store contains at least one
     * triple whose subject and predicate match those for my URI and
     * the expansion of the given property name, convert the object
     * part of the first such matching triple to a QVariant via
     * Node::toVariant and return that value.  If there is no such
     * match, return QVariant().
     */
    QVariant getProperty(Transaction *tx, QString name) const;

    /**
     * Get the value of the given property as a list.  That is, if the
     * store contains at least one triple whose subject and predicate
     * match those for my URI and the expansion of the given property
     * name, convert the object parts of all such matching triples to
     * QVariant via Node::toVariant and return a list of the resulting
     * values.  If there is no such match, return an empty list.
     *
     * Note that the order of variants in the returned list is
     * arbitrary and may change from one call to the next.
     */
    QVariantList getPropertyList(QString name) const;

    /**
     * Get the value of the given property as a list, querying through
     * the given transaction.  That is, if the store contains at least
     * one triple whose subject and predicate match those for my URI
     * and the expansion of the given property name, convert the
     * object parts of all such matching triples to QVariant via
     * Node::toVariant and return a list of the resulting values.  If
     * there is no such match, return QVariant().
     *
     * Note that the order of variants in the returned list is
     * arbitrary and may change from one call to the next.
     */
    QVariantList getPropertyList(Transaction *tx, QString name) const;

    /**
     * Get the node for the given property.  That is, if the store
     * contains at least one triple whose subject and predicate match
     * those for my URI and the expansion of the given property name,
     * return the object part of the first such matching triple.  If
     * there is no such match, return Node().
     */
    Node getPropertyNode(QString name) const;
    
    /**
     * Get the node for the given property, querying through the given
     * transaction.  That is, if the store contains at least one
     * triple whose subject and predicate match those for my URI and
     * the expansion of the given property name, return the object
     * part of the first such matching triple.  If there is no such
     * match, return Node().
     */
    Node getPropertyNode(Transaction *tx, QString name) const;

    /**
     * Get the nodes for the given property.  That is, if the store
     * contains at least one triple whose subject and predicate match
     * those for my URI and the expansion of the given property name,
     * return the object parts of all such matching triples.  If there
     * is no such match, return an empty list.
     *
     * Note that the order of nodes in the returned list is arbitrary
     * and may change from one call to the next.
     */
    Nodes getPropertyNodeList(QString name) const;

    /**
     * Get the nodes for the given property, querying through the
     * given transaction.  That is, if the store contains at least one
     * triple whose subject and predicate match those for my URI and
     * the expansion of the given property name, return the object
     * parts of all such matching triples.  If there is no such match,
     * return an empty list.
     *
     * Note that the order of nodes in the returned list is arbitrary
     * and may change from one call to the next.
     *
     *!!! NB this is misnamed -- this looks up a set rather than a list
     */
    Nodes getPropertyNodeList(Transaction *tx, QString name) const;

    /**
     * Get the names of this object's properties beginning with our
     * property prefix.  That is, find all triples in the store whose
     * subject matches my URI and whose predicate begins with our
     * property prefix, and return a list of the remainder of their
     * predicate URIs following the property prefix.
     */
    QStringList getPropertyNames() const;

    /**
     * Get the names of this object's properties beginning with our
     * property prefix, querying through the given transaction.  That
     * is, find all triples in the store whose subject matches my URI
     * and whose predicate begins with our property prefix, and return
     * a list of the remainder of their predicate URIs following the
     * property prefix.
     */
    QStringList getPropertyNames(Transaction *tx) const;

    /**
     * Set the given property to the given value.  That is, first
     * remove from the store any triples whose subject and predicate
     * match those for my URI and the expansion of the given property
     * name, then insert a new triple whose object part is the result
     * of converting the given variant to a node via
     * Node::fromVariant.
     */
    void setProperty(QString name, QVariant value);

    /**
     * Set the given property to the given URI.  That is, first
     * remove from the store any triples whose subject and predicate
     * match those for my URI and the expansion of the given property
     * name, then insert a new triple whose object part is the URI.
     */
    void setProperty(QString name, Uri uri);

    /**
     * Set the given property to the given node.  That is, first
     * remove from the store any triples whose subject and predicate
     * match those for my URI and the expansion of the given property
     * name, then insert a new triple whose object part is the node.
     */
    void setProperty(QString name, Node node);

    /**
     * Set the given property to the given value through the given
     * transaction.  That is, first remove from the store any triples
     * whose subject and predicate match those for my URI and the
     * expansion of the given property name, then insert a new triple
     * whose object part is the result of converting the given variant
     * to a node via Node::fromVariant.
     */
    void setProperty(Transaction *tx, QString name, QVariant value);

    /**
     * Set the given property to the given URI through the given
     * transaction.  That is, first remove from the store any triples
     * whose subject and predicate match those for my URI and the
     * expansion of the given property name, then insert a new triple
     * whose object part is the URI.
     */
    void setProperty(Transaction *tx, QString name, Uri uri);

    /**
     * Set the given property to the given node through the given
     * transaction.  That is, first remove from the store any triples
     * whose subject and predicate match those for my URI and the
     * expansion of the given property name, then insert a new triple
     * whose object part is the node.
     */
    void setProperty(Transaction *tx, QString name, Node node);

    /**
     * Set the given property to the given values.  That is, first
     * remove from the store any triples whose subject and predicate
     * match those for my URI and the expansion of the given property
     * name, then insert a new triple for each variant in the value
     * list, whose object part is the result of converting that
     * variant to a node via Node::fromVariant.
     */
    void setPropertyList(QString name, QVariantList values);

    /**
     * Set the given property to the given values through the given
     * transaction.  That is, first remove from the store any triples
     * whose subject and predicate match those for my URI and the
     * expansion of the given property name, then insert a new triple
     * for each variant in the value list, whose object part is the
     * result of converting that variant to a node via
     * Node::fromVariant.
     */
    void setPropertyList(Transaction *tx, QString name, QVariantList values);

    /**
     * Set the given property to the given nodes.  That is, first
     * remove from the store any triples whose subject and predicate
     * match those for my URI and the expansion of the given property
     * name, then insert a new triple for each node in the list, whose
     * object part is that node.
     */
    void setPropertyList(QString name, Nodes nodes);

    /**
     * Set the given property to the given nodes through the given
     * transaction.  That is, first remove from the store any triples
     * whose subject and predicate match those for my URI and the
     * expansion of the given property name, then insert a new triple
     * for each node in the list, whose object part is that node.
     */
    void setPropertyList(Transaction *tx, QString name, Nodes nodes);

    /**
     * Remove the given property.  That is, remove from the store any
     * triples whose subject and predicate match those for my URI and
     * the expansion of the given property name.
     */
    void removeProperty(QString name);

    /**
     * Remove the given property.  That is, remove from the store any
     * triples whose subject and predicate match those for my URI and
     * the expansion of the given property name.
     */
    void removeProperty(Transaction *tx, QString name);

    /**
     * Return the Store object that will be used for modifications in
     * the given transaction.  If the transaction is not
     * NoTransaction, then the returned Store will simply be the
     * transaction itself; otherwise it will be the store that was
     * passed to the constructor.
     */
    Store *getStore(Transaction *tx) const;

    /**
     * Return the URI used for the "predicate" part of any triple
     * referring to the given property name.  See the general
     * PropertyObject documentation for details of how these names are
     * expanded.
     */
    Uri getPropertyUri(QString name) const;

    /**
     * Set the global default property prefix.  This will be used as
     * the prefix for all PropertyObjects subsequently constructed
     * using the two-argument (prefixless) constructors.
     */
    static void setDefaultPropertyPrefix(QString prefix);
    
private:
    Store *m_store;
    QString m_pfx;
    Uri m_upfx;
    Node m_node;
    static QString m_defaultPrefix;
};

/**
 * \class CacheingPropertyObject PropertyObject.h <dataquay/PropertyObject.h>
 *
 * CacheingPropertyObject is a helper class for managing RDF
 * properties of an object URI -- that is, triples that share a common
 * subject and possibly a common prefix for the predicate, and that
 * have only one value for each subject-predicate combination.
 *
 * This class caches results from the datastore and so may be faster
 * than PropertyObject, but it can only be used in contexts where it
 * is known that no other agent may be modifying the same set of
 * properties.  Its set of available functions is more limited than
 * PropertyObject also: it has no Transaction-based functions.
 *
 * See PropertyObject for individual method documentation.
 */
class CacheingPropertyObject
{
public:
    CacheingPropertyObject(Store *s, Uri myUri);
    CacheingPropertyObject(Store *s, QString myUri);
    CacheingPropertyObject(Store *s, QString propertyPrefix, Uri myUri);
    CacheingPropertyObject(Store *s, QString propertyPrefix, QString myUri);
    CacheingPropertyObject(Store *s, QString propertyPrefix, Node myUri);

    Uri getObjectType() const;

    bool hasProperty(QString name) const;

    QVariant getProperty(QString name) const;
    QVariantList getPropertyList(QString name) const;
    Node getPropertyNode(QString name) const;
    Nodes getPropertyNodeList(QString name) const;
    QStringList getPropertyNames() const;

    void setProperty(QString name, QVariant value);
    void setProperty(QString name, Uri value);
    void setProperty(QString name, Node node);
    void setPropertyList(QString name, QVariantList values);
    void setPropertyList(QString name, Nodes nodes);

    void removeProperty(QString name);

    Uri getPropertyUri(QString name) const;

private:
    PropertyObject m_po;
    typedef QHash<Uri, Nodes> Properties;
    mutable Properties m_cache; // note: value is never empty
    mutable bool m_cached;
    void encache() const;
};

}

#endif
