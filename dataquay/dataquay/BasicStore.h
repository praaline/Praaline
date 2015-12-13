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

#ifndef _DATAQUAY_BASIC_STORE_H_
#define _DATAQUAY_BASIC_STORE_H_

#include "Store.h"

namespace Dataquay
{
	
/**
 * \class BasicStore BasicStore.h <dataquay/BasicStore.h>
 *
 * BasicStore is an in-memory RDF data store implementing the Store
 * interface, providing add, remove, matching and query operations for
 * RDF triples and SPARQL, as well as export and import.
 *
 * BasicStore uses a Redland or Sord datastore internally, depending
 * on whether USE_REDLAND or USE_SORD was defined when Dataquay was
 * built.
 *
 * All operations are thread safe.
 */
class BasicStore : public Store
{
public:
    BasicStore();
    ~BasicStore();

    /**
     * Set the base URI for the store.  This is used to expand the
     * empty URI prefix when adding and querying triples, and is also
     * used as the document base URI when exporting.
     */
    void setBaseUri(Uri uri);

    /**
     * Retrieve the base URI for the store.
     */
    Uri getBaseUri() const;
    
    /**
     * Empty the store of triples.  Prefixes that have been added with
     * addPrefix are unaffected.
     *!!! hoist to Store()?
     */
    void clear();

    /**
     * Add a prefix/uri pair (an XML namespace, except that this class
     * doesn't directly deal in XML) for use in subsequent operations.
     * If the prefix has already been added, this overrides any uri
     * associated with it.
     *
     * Example: addPrefix("dc", "http://purl.org/dc/elements/1.1/") to
     * add a prefix for the Dublin Core namespace.
     *
     * The store always knows about the XSD and RDF namespaces.
     *
     * Note that the base URI is always available as the empty prefix.
     * For example, the URI ":blather" will be expanded to the base
     * URI plus "blather".
     */
    void addPrefix(QString prefix, Uri uri);

    // Store interface

    bool add(Triple t);
    bool remove(Triple t);

    void change(ChangeSet changes);
    void revert(ChangeSet changes);

    bool contains(Triple t) const;
    Triples match(Triple t) const;
    ResultSet query(QString sparql) const;

    Node complete(Triple t) const;

    Triple matchOnce(Triple t) const;
    Node queryOnce(QString sparql, QString bindingName) const;

    Uri getUniqueUri(QString prefix) const;
    Node addBlankNode();
    Uri expand(QString uri) const;

    void save(QString filename) const;
    void import(QUrl url, ImportDuplicatesMode idm, QString format = "");

    Features getSupportedFeatures() const;

    /**
     * Construct a new BasicStore from the RDF document at the given
     * URL.  May throw RDFException.  The returned BasicStore is owned
     * by the caller and must be deleted using delete when finished
     * with.  The return value is never NULL; all errors result in
     * exceptions.
     *
     * Note that the URL must be a URL, not just a filename
     * (i.e. local files need the file: prefix). The file URL will
     * also become the base URI of the store.
     *
     * If format is specified, it will be taken as the RDF parse
     * format (e.g. ntriples).  The set of supported format strings
     * depends on the underlying RDF library configuration.  The
     * default is to guess the format if possible.
     */
    static BasicStore *load(QUrl url, QString format = "");

private:
    class D;
    D *m_d;
};

}

#endif
    
