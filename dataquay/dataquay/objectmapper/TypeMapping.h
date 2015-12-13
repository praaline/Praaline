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

#ifndef _DATAQUAY_TYPE_MAPPING_H_
#define _DATAQUAY_TYPE_MAPPING_H_

#include "../Uri.h"

#include <QString>

namespace Dataquay
{

/**
 * TypeMapping describes a set of relationships between RDF entity and
 * property URIs, and C++ class and QObject property names.  The
 * TypeMapping is referred to by ObjectMapper, ObjectStorer and
 * ObjectLoader when mapping between RDF entities and C++ objects.
 *
 * For example, say we have a class MyApplication::Person, with a
 * QObject string property called "name".  We want the class to be
 * stored as a URI with RDF type foaf:Person, and the property to be
 * stored as a relationship for that URI of type foaf:name.  These
 * URIs need to be expanded in order to be used with TypeMapping; we
 * assume here that "store" points to a Store which is aware of the
 * "foaf" prefix and can perform that expansion for us through
 * Store::expand.
 *
 * \code
 * TypeMapping tm;
 * tm.addTypeMapping("MyApplication::Person", store->expand("foaf:Person"));
 * tm.addPropertyMapping("MyApplication::Person", "name", store->expand("foaf:name"));
 * objectStorer->setTypeMapping(tm);
 * \endcode
 *
 * Now if objectStorer is used to store an object of class
 * MyApplication::Person, it will do so as a foaf:Person.  Similarly,
 * whenever ObjectLoader is asked to load a foaf:Person, it will
 * create a MyApplication::Person object to do so.
 *
 * TypeMapping is re-entrant, but not thread-safe.
 * 
 * 
 *!!! TODO: Review method names
 */
class TypeMapping
{
public:
    /**
     * Construct a TypeMapping using default URIs throughout.
     */
    TypeMapping();
    
    TypeMapping(const TypeMapping &);
    TypeMapping &operator=(const TypeMapping &);
    ~TypeMapping();

    /**
     * Set the prefix for synthetic type URIs.  This is the prefix
     * used when asked to generate or convert type URIs for which no
     * specific mapping has been set using setTypeMapping.  See also
     * \ref synthesiseTypeUriForClass and \ref synthesiseClassForTypeUri.
     */
    void setObjectTypePrefix(Uri prefix);

    /**
     * Retrieve the prefix for synthetic type URIs.
     */
    Uri getObjectTypePrefix() const;

    /**
     * Set the prefix for synthetic property URIs.  This is the prefix
     * used when generating URIs for QObject properties for which no
     * specific mapping has been set using addPropertyMapping, and
     * when converting such URIs back to QObject properties.
     */
    void setPropertyPrefix(Uri prefix);

    /**
     * Retrieve the prefix for synthetic property URIs.
     */
    Uri getPropertyPrefix() const;

    /**
     * Set the prefix for ObjectMapper-specific property URIs.  This
     * is the prefix used for object relationship properties such as
     * "parent" or "follows".
     */
    void setRelationshipPrefix(Uri prefix);

    /**
     * Retrieve the prefix for ObjectMapper-specific property URIs.
     */
    Uri getRelationshipPrefix() const;

    /**
     * Add a specific mapping from class name to entity URI.
     */
    void addTypeMapping(QString className, Uri uri);

    /**
     * Retrieve the URI that has been set for the given class name
     * using \ref addTypeMapping, returning it in uri.  Return true if
     * such a URI was found, false otherwise.
     */
    bool getTypeUriForClass(QString className, Uri &uri) const;

    /**
     * Retrieve the C++ class name that has been set for the given
     * entity URI using \ref addTypeMapping, returning it in
     * className.  Return true if such a name was found, false
     * otherwise.
     */
    bool getClassForTypeUri(Uri uri, QString &className) const;

    /**
     * Return a URI for the RDF entity type corresponding to the given
     * C++ class.
     *
     * If a specific URI has been set for this class name using \ref
     * addTypeMapping (i.e. if \ref getTypeUriForClass would return a
     * result for this class name), the result will be that URI.
     *
     * Otherwise, the result will consist of the synthetic type URI
     * prefix (set using setObjectTypePrefix, or the default synthetic
     * type URI prefix if none has been set) followed by the class
     * name, with all namespace separators ("::") replaced by slashes
     * ("/").
     */
    Uri synthesiseTypeUriForClass(QString className) const;

    /**
     * Return a C++ class name corresponding to the given RDF entity
     * type URI.
     *
     * If a specific class name has been set for this URI using \ref
     * addTypeMapping (i.e. if \ref getClassForTypeUri would return a
     * result for this URI), the result will be that class name.
     *
     * Otherwise, the result will consist of the URI with the
     * synthetic type URI prefix (set using setObjectTypePrefix, or
     * the default synthetic type URI prefix if none has been set)
     * stripped off the beginning, and with subsequent slashes ("/")
     * replaced by namespace separators ("::").
     *
     * Throws UnknownTypeException if no specific class name is
     * available and the URI does not begin with the synthetic type
     * URI prefix.
     */
    QString synthesiseClassForTypeUri(Uri uri) const;

    /**
     * Add a mapping between class name and the common parts of any
     * URIs that are automatically generated when storing instances of
     * that class that have no URI property defined.
     * 
     * For example, a mapping from "MyNamespace::MyClass" to
     * "http://mydomain.com/resource/" would ensure that automatically
     * generated "unique" URIs for instances that class all started
     * with that URI prefix.  Note that the prefix itself is also
     * subject to namespace prefix expansion when stored.
     *
     * (If no prefix mapping was given for this example, its generated
     * URIs would start with ":mynamespace_myclass_".)
     *
     * Generated URIs are only checked for uniqueness within the store
     * being exported to and cannot be guaranteed to be globally
     * unique.  For this reason, caution should be exercised in the
     * use of this function.
     *
     * Note that in principle the object mapper could use this when
     * loading, to infer class types for URIs in the store that have
     * no rdf:type.  In practice that does not happen -- the object
     * mapper will not generate a class for URIs without rdf:type.
     */
    void addTypeUriPrefixMapping(QString className, Uri prefix);

    /**
     * Retrieve the URI prefix set for the given className using \ref
     * addTypeUriPrefixMapping, if any, returning it in prefix.
     * Return true if such a prefix was found, false otherwise.
     */
    bool getUriPrefixForClass(QString className, Uri &prefix) const;

    /**
     * Add a specific mapping for the given QObject property in the
     * given C++ class, to an RDF property URI.
     *
     * Note that distinct properties of the same class must map to
     * distinct URIs.  If two properties of a class map to the same
     * URI, ObjectLoader will not be able to distinguish between them
     * (it does not attempt to resolve ambiguities using the type of
     * the argument, for example).
     */
    void addPropertyMapping(QString className, QString propertyName, Uri uri);

    /**
     * Retrieve the URI that has been set for the given property in
     * the given class using \ref addPropertyMapping, returning it in
     * uri.  Return true if such a URI was found, false otherwise.
     */
    bool getPropertyUri(QString className, QString propertyName, Uri &uri) const;

    /**
     * Retrieve the name of the property for which the given URI has
     * been set in the given class using \ref addPropertyMapping,
     * returning it in propertyName.  Return true if such a property
     * was found, false otherwise.
     */
    bool getPropertyName(QString className, Uri propertyUri, QString &propertyName) const;

    //!!! Note no property equivalents of
    //!!! synthesiseClassForTypeUri/synthesiseTypeUriForClass -- do we want them? if only to have somewhere to put the documentation?

private:
    class D;
    D *m_d;
};

}

#endif
