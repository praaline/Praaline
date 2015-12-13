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

#ifndef _DATAQUAY_URI_H_
#define _DATAQUAY_URI_H_

namespace Dataquay {
class Uri;
}

// Declare this early, to avoid any problems with instantiation order
// arising from inclusion "races"
extern unsigned int qHash(const Dataquay::Uri &u);

#include <QString>
#include <QUrl>

#include <QMetaType>

#include <iostream>

class QDataStream;
class QTextStream;
class QVariant;

namespace Dataquay
{

/**
 * Uri represents a single URI.  It is a very thin wrapper around a
 * string.  Its purpose is to allow us to distinguish between
 * abbreviated URIs (CURIEs) which may be subject to prefix expansion
 * (represented by strings) and full URIs (represented by Uri).
 *
 * In terms of the Turtle syntax, anything written within angle
 * brackets is a Uri, while a bare string in URI context is not: it
 * should be stored as a QString and converted to a Uri using
 * Store::expand().  For example, <http://xmlns.com/foaf/0.1/name> is
 * a Uri, while foaf:name is just a string.  Never store the latter
 * form in a Uri object without expanding it first.
 *
 * Dataquay uses Uri in preference to QUrl because the latter is
 * relatively slow to convert to and from string forms.  Uri imposes
 * no overhead over a string, it simply aids type safety.
 */
class Uri
{
public:
    /**
     * Construct an empty (invalid, null) URI.
     */
    Uri() {
    }

    /**
     * Construct a URI from the given string, which is expected to
     * contain the text of a complete well-formed absolute URI.
     *
     * As special cases, file: URIs are allowed to be relative
     * ("file:x" will be expanded to "file://x" automatically) and "a"
     * will be expanded to the rdf:type URI.
     *
     * This will throw RDFIncompleteURI if given an incomplete or
     * relative URI, so use care when constructing Uri objects from
     * unvalidated input.  Call isComplete() if you wish to test
     * whether a URI string will be accepted before constructing.
     *
     * To construct a Uri from an abbreviated or relative URI via
     * prefix or base expansion, use Store::expand().
     *
     * This constructor is intentionally marked explicit; no silent
     * conversion is available.
     */
    explicit Uri(const QString &s) : m_uri(s) {
	checkComplete();
    }

    /**
     * Construct a URI from the given QUrl, which is expected to
     * contain a complete well-formed URI.  May throw
     * RDFIncompleteURI.
     */
    explicit Uri(const QUrl &u) : m_uri(u.toString()) {
	checkComplete();
    }
    ~Uri() {
    }

    inline QString toString() const { return m_uri; }
    inline QUrl toUrl() const { return QUrl(m_uri); }
    inline int length() const { return m_uri.length(); }

    QString scheme() const;

    bool operator==(const Uri &u) const;
    inline bool operator!=(const Uri &u) const { return !operator==(u); }
    inline bool operator<(const Uri &u) const { return m_uri < u.m_uri; }
    inline bool operator>(const Uri &u) const { return u < *this; }

    /**
     * Return true if the given string contains a complete URI,
     * i.e. if it is possible to construct a Uri object from it.
     */
    static bool isCompleteUri(QString s);

    static QString metaTypeName();
    static int metaTypeId();

    /**
     * Return true if the given variant has metatype metatypeId().
     */
    static bool hasUriType(const QVariant &);

    /**
     * Return the rdf:type URI.
     */
    static Uri rdfTypeUri();
    
private:
    void checkComplete();
    QString m_uri;
    static bool canBeComplete(QString &s);
};

typedef QList<Uri> UriList;

QDataStream &operator<<(QDataStream &out, const Uri &);
QDataStream &operator>>(QDataStream &in, Uri &);

std::ostream &operator<<(std::ostream &out, const Uri &);
QTextStream &operator<<(QTextStream &out, const Uri &);

}

Q_DECLARE_METATYPE(Dataquay::Uri)

#endif
