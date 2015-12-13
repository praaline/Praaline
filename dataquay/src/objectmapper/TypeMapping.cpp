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

#include "objectmapper/TypeMapping.h"
#include "objectmapper/ObjectMapperExceptions.h"

#include <QHash>

namespace Dataquay
{

static Uri defaultTypePrefix("http://breakfastquay.com/rdf/dataquay/objectmapper/type/");
static Uri defaultPropertyPrefix("http://breakfastquay.com/rdf/dataquay/objectmapper/property/");
static Uri defaultRelationshipPrefix("http://breakfastquay.com/rdf/dataquay/objectmapper/relationship/");

class TypeMapping::D
{
public:
    D() :
        m_typePrefix(defaultTypePrefix),
        m_propertyPrefix(defaultPropertyPrefix),
        m_relationshipPrefix(defaultRelationshipPrefix) {
    }

    Uri getObjectTypePrefix() const {
        return m_typePrefix;
    }

    void setObjectTypePrefix(Uri prefix) {
        m_typePrefix = prefix;
    }

    Uri getPropertyPrefix() const {
        return m_propertyPrefix;
    }

    void setPropertyPrefix(Uri prefix) { 
        m_propertyPrefix = prefix;
    }

    Uri getRelationshipPrefix() const {
        return m_relationshipPrefix;
    }

    void setRelationshipPrefix(Uri prefix) { 
        m_relationshipPrefix = prefix;
    }

    void addTypeMapping(QString className, Uri uri) {
        m_typeMap[uri] = className;
        m_typeRMap[className] = uri;
    }

    bool getTypeUriForClass(QString className, Uri &uri) const {
	QHash<QString, Uri>::const_iterator i = m_typeRMap.find(className);
	if (i != m_typeRMap.end()) {
	    uri = *i;
	    return true;
	}
	return false;
    }

    bool getClassForTypeUri(Uri typeUri, QString &className) const {
	QHash<Uri, QString>::const_iterator i = m_typeMap.find(typeUri);
	if (i != m_typeMap.end()) {
	    className = *i;
	    return true;
	}
	return false;
    }

    QString synthesiseClassForTypeUri(Uri typeUri) const {
        QString s;
        if (getClassForTypeUri(typeUri, s)) {
            return s;
        }
        s = typeUri.toString();
        if (!s.startsWith(m_typePrefix.toString())) {
            throw UnknownTypeException(s);
        }
        s = s.right(s.length() - m_typePrefix.length());
        s.replace('/', "::");
        return s;
    }

    Uri synthesiseTypeUriForClass(QString className) const {
        Uri typeUri;
        if (getTypeUriForClass(className, typeUri)) {
            return typeUri;
        }
        typeUri = Uri(QString(m_typePrefix.toString() + className)
                      .replace("::", "/"));
        return typeUri;
    }

    void addTypeUriPrefixMapping(QString className, Uri prefix) {
        m_typeUriPrefixMap[className] = prefix;
    }

    bool getUriPrefixForClass(QString className, Uri &prefix) const {
	QHash<QString, Uri>::const_iterator i = m_typeUriPrefixMap.find(className);
	if (i != m_typeUriPrefixMap.end()) {
	    prefix = *i;
	    return true;
	}
	return false;
    }

    void addPropertyMapping(QString className, QString propertyName, Uri uri) {
        m_propertyMap[className][uri] = propertyName;
        m_propertyRMap[className][propertyName] = uri;
    }

    bool getPropertyUri(QString className, QString propertyName, Uri &uri) const {
	QHash<QString, QHash<QString, Uri> >::const_iterator j = 
	    m_propertyRMap.find(className);
	if (j != m_propertyRMap.end()) {
	    QHash<QString, Uri>::const_iterator i = j->find(propertyName);
	    if (i != j->end()) {
		uri = *i;
		return true;
	    }
	}
	return false;
    }

    bool getPropertyName(QString className, Uri uri, QString &propertyName) const {
	QHash<QString, QHash<Uri, QString> >::const_iterator j = 
	    m_propertyMap.find(className);
	if (j != m_propertyMap.end()) {
	    QHash<Uri, QString>::const_iterator i = j->find(uri);
	    if (i != j->end()) {
		propertyName = *i;
		return true;
	    }
	}
	return false;
    }

private:
    Uri m_typePrefix;
    Uri m_propertyPrefix;
    Uri m_relationshipPrefix;
    QHash<Uri, QString> m_typeMap;
    QHash<QString, Uri> m_typeRMap;
    QHash<QString, Uri> m_typeUriPrefixMap;
    QHash<QString, QHash<Uri, QString> > m_propertyMap;
    QHash<QString, QHash<QString, Uri> > m_propertyRMap;
};

TypeMapping::TypeMapping() :
    m_d(new D())
{
}

TypeMapping::TypeMapping(const TypeMapping &tm) :
    m_d(new D(*tm.m_d))
{
}

TypeMapping &
TypeMapping::operator=(const TypeMapping &tm)
{
    if (this != &tm) {
	delete m_d;
	m_d = new D(*tm.m_d);
    }
    return *this;
}

TypeMapping::~TypeMapping()
{
    delete m_d;
}

void
TypeMapping::setObjectTypePrefix(Uri prefix)
{
    m_d->setObjectTypePrefix(prefix);
}

Uri
TypeMapping::getObjectTypePrefix() const
{
    return m_d->getObjectTypePrefix();
}

void
TypeMapping::setPropertyPrefix(Uri prefix)
{
    m_d->setPropertyPrefix(prefix);
}

Uri
TypeMapping::getPropertyPrefix() const
{
    return m_d->getPropertyPrefix();
}

void
TypeMapping::setRelationshipPrefix(Uri prefix)
{
    m_d->setRelationshipPrefix(prefix);
}

Uri
TypeMapping::getRelationshipPrefix() const
{
    return m_d->getRelationshipPrefix();
}

void
TypeMapping::addTypeMapping(QString className, Uri uri)
{
    m_d->addTypeMapping(className, uri);
}

bool
TypeMapping::getTypeUriForClass(QString className, Uri &uri) const
{
    return m_d->getTypeUriForClass(className, uri);
}

bool
TypeMapping::getClassForTypeUri(Uri uri, QString &className) const
{
    return m_d->getClassForTypeUri(uri, className);
}

Uri
TypeMapping::synthesiseTypeUriForClass(QString className) const
{
    return m_d->synthesiseTypeUriForClass(className);
}

QString
TypeMapping::synthesiseClassForTypeUri(Uri typeUri) const
{
    return m_d->synthesiseClassForTypeUri(typeUri);
}

void
TypeMapping::addTypeUriPrefixMapping(QString className, Uri prefix)
{
    m_d->addTypeUriPrefixMapping(className, prefix);
}

bool
TypeMapping::getUriPrefixForClass(QString className, Uri &prefix) const
{
    return m_d->getUriPrefixForClass(className, prefix);
}

void
TypeMapping::addPropertyMapping(QString className, QString propertyName, Uri uri)
{
    m_d->addPropertyMapping(className, propertyName, uri);
}

bool
TypeMapping::getPropertyUri(QString className, QString propertyName, Uri &uri) const
{
    return m_d->getPropertyUri(className, propertyName, uri);
}

bool
TypeMapping::getPropertyName(QString className, Uri propertyUri, QString &propertyName) const
{
    return m_d->getPropertyName(className, propertyUri, propertyName);
}

}

