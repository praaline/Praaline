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

#ifndef _DATAQUAY_STORE_H_
#define _DATAQUAY_STORE_H_

#include "Triple.h"

#include <QList>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QSet>

namespace Dataquay
{

/// A mapping from key to node, used to list results for a set of result keys.
typedef QHash<QString, Node> Dictionary;

/// A list of Dictionary types, used to contain a sequence of query results.
typedef QList<Dictionary> ResultSet;

enum ChangeType {
    AddTriple,
    RemoveTriple
};

/// An add or remove operation specified by add/remove token and triple.
typedef QPair<ChangeType, Triple> Change;

/// A sequence of add/remove operations such as may be enacted by a transaction.
typedef QList<Change> ChangeSet;


/**
 * \class Store Store.h <dataquay/Store.h>
 *
 * Store is an abstract interface for Dataquay RDF data stores.
 */
class Store
{
public:
    /**
     * Add a triple to the store.  Return false if the triple was
     * already in the store.  (Dataquay does not permit duplicate
     * triples in a store.)  Throw RDFException if the triple can not
     * be added for some other reason.
     */
    virtual bool add(Triple t) = 0;
    
    /**
     * Remove a triple from the store.  If some nodes in the triple are
     * Nothing nodes, remove all matching triples.  Return false if no
     * matching triple was found in the store.  Throw RDFException if
     * removal failed for some other reason.
     */
    virtual bool remove(Triple t) = 0;

    /**
     * Atomically apply the sequence of add/remove changes described
     * in the given ChangeSet.  Throw RDFException if any operation
     * fails for any reason (including duplication etc).
     */
    virtual void change(ChangeSet changes) = 0;
    
    /**
     * Atomically apply the sequence of add/remove changes described
     * in the given ChangeSet, in reverse (ie removing adds and
     * adding removes, in reverse order).  Throw RDFException if any
     * operation fails for any reason (including duplication etc).
     */
    virtual void revert(ChangeSet changes) = 0;
    
    /**
     * Return true if the store contains the given triple, false
     * otherwise.  Throw RDFException if the triple is not complete or if
     * the test failed for any other reason.
     */
    virtual bool contains(Triple t) const = 0;
    
    /**
     * Return all triples matching the given wildcard triple.  A node
     * of type Nothing in any part of the triple matches any node in
     * the data store.  Return an empty list if there are no matches; may
     * throw RDFException if matching fails in some other way.
     */
    virtual Triples match(Triple t) const = 0;

    /**
     * Run a SPARQL query against the store and return its results.
     * Any prefixes added previously using addQueryPrefix will be
     * available in this query without needing to be declared in the
     * SPARQL given here (equivalent to writing "PREFIX prefix: <uri>"
     * for each prefix,uri pair set with addPrefix).
     *
     * May throw RDFException.
     *
     * Note that the RDF store must have an absolute base URI (rather
     * than the default "#") in order to perform queries, as relative
     * URIs in the query will be interpreted relative to the query
     * base rather than the store and without a proper base URI there
     * is no way to override that internally.
     */
    virtual ResultSet query(QString sparql) const = 0;

    /**
     * Given a triple in which any two nodes are specified and the
     * other is a wildcard node of type Nothing, return a node that
     * can be substituted for the Nothing node in order to complete a
     * triple that exists in the store. If no matching triple can be
     * found, return a null node.  If more than one triple matches,
     * the returned value may arbitrarily be from any of them. May
     * throw RDFException.
     */
    virtual Node complete(Triple t) const = 0;

    /**
     * Return a triple from the store that matches the given wildcard
     * triple, or the empty triple if none matches.  A node of type
     * Nothing in any part of the triple matches any node in the data
     * store.  If more than one triple matches, the returned value may
     * arbitrarily be any of them.  May throw RDFException.
     */
    virtual Triple matchOnce(Triple t) const = 0;

    /**
     * Run a SPARQL query against the store and return the node of
     * the first result for the given query binding.  This is a
     * shorthand for use with queries that are only expected to have
     * one result.  May throw RDFException.
     */
    virtual Node queryOnce(QString sparql, QString bindingName) const = 0;
    
    /**
     * Get a new URI, starting with the given prefix (e.g. ":event_"),
     * that does not currently exist within this store.  The URI will
     * be prefix expanded.
     */
    virtual Uri getUniqueUri(QString prefix) const = 0;

    /**
     * Create and return a new blank node.  This node can only be
     * referred to using the given Node object, and only during its
     * lifetime within this instance of the store.
     */
    virtual Node addBlankNode() = 0;

    /**
     * Expand the given URI (which may use local namespaces) and
     * prefix-expand it, returning the result as a Uri.  (The Uri
     * class cannot be used to store URIs that have unexpanded
     * namespace prefixes.)
     *
     * The set of available prefixes for expansion depends on the
     * subclass implementation.  See, for example,
     * BasicStore::addPrefix.
     */
    virtual Uri expand(QString uri) const = 0;

    /** 
     * Export the store to an RDF/TTL file with the given filename.
     * If the file already exists, it will if possible be overwritten.
     * May throw RDFException, FileOperationFailed, FailedToOpenFile,
     * etc.
     *
     * Note that unlike import (which takes a URL argument), save
     * takes a simple filename with no file:// prefix.
     */
    virtual void save(QString filename) const = 0;

    /**
     * ImportDuplicatesMode determines the outcome when an import
     * operation encounters a triple in the imported data set that
     * already exists in the store.
     *
     * ImportIgnoreDuplicates: Any duplicate of a triple that is
     * already in the store is discarded without comment.
     *
     * ImportFailOnDuplicates: Import will fail with an
     * RDFDuplicateImportException if any duplicate of a triple
     * already in the store is found, and nothing will be imported.
     *
     * ImportPermitDuplicates: No tests for duplicate triples will be
     * carried out, and the behaviour when duplicates are imported
     * will depend on the underlying store implementation (which may
     * merge them or store them as separate duplicate triples).  This
     * is usually inadvisable: besides its unpredictability, this
     * class does not generally handle duplicate triples well in other
     * contexts.
     */
    enum ImportDuplicatesMode {
        ImportIgnoreDuplicates,
        ImportFailOnDuplicates,
        ImportPermitDuplicates
    };

    /**
     * Import the RDF document found at the given URL into the current
     * store (in addition to its existing contents).  Its behaviour
     * when a triple is encountered that already exists in the store
     * is controlled by the ImportDuplicatesMode.
     * 
     * May throw RDFException or RDFDuplicateImportException.
     *
     * Note that the URL must be a URL, not just a filename
     * (i.e. local files need the file: prefix).
     *
     * If format is specified, it will be taken as the RDF parse
     * format (e.g. ntriples).  The set of supported format strings
     * depends on the underlying RDF library configuration.  The
     * default is to guess the format if possible.
     */
    virtual void import(QUrl url, ImportDuplicatesMode idm, QString format = "") = 0;

    /**
     * Feature defines the set of optional features a Store
     * implementation may support.  Code that uses Store should check
     * that the features it requires are available before trying to
     * make use of them.
     *
     * ModifyFeature: The store can be modified (triples can be added
     * to it).  All current Store implementations support this feature.
     *
     * QueryFeature: The store supports SPARQL queries through the
     * query and queryOnce methods.  A store that does not support
     * queries will throw RDFUnsupportedError when these functions are
     * called.
     *
     * RemoteImportFeature: The store can import URLs that represent
     * network resources as well as URLs referring to files on the
     * local disc.  The extent to which this feature is actually
     * available may also depend on the configuration of the
     * underlying RDF library.  A store that does not support remote
     * URLs will fail as if the resource was absent, when asked to
     * load one.
     */
    enum Feature {
        ModifyFeature,
        QueryFeature,
        RemoteImportFeature
    };
    
    typedef QSet<Feature> Features;

    /** 
     * Retrieve the set of optional features supported by this Store
     * implementation.
     */
    virtual Features getSupportedFeatures() const = 0;

protected:
    virtual ~Store() { }
};

}

QDataStream &operator<<(QDataStream &out, const Dataquay::ChangeType &);
QDataStream &operator>>(QDataStream &in, Dataquay::ChangeType &);

#endif

