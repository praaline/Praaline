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

#ifndef _DATAQUAY_NODE_H_
#define _DATAQUAY_NODE_H_

namespace Dataquay {
class Node;
}

// Declare this early, to avoid any problems with instantiation order
// arising from inclusion "races".  If this is not found when
// compiling, hoist this header so that it is included earlier than
// any other header that includes <QHash>
extern unsigned int qHash(const Dataquay::Node &);

#include "Uri.h"

#include <QString>
#include <QVariant>

class QDataStream;
class QTextStream;

namespace Dataquay
{

/**
 * \class Node Node.h <dataquay/Node.h>
 *
 * Node represents a single RDF node, with conversions to and from
 * variant types.
 */
class Node
{
public:
    /**
     * Node type.
     */
    enum Type { Nothing, URI, Literal, Blank };

    /**
     * Construct a node with no node type (used for example as an
     * undefined node when pattern matching a triple).
     */
    Node() : type(Nothing), value() { }

    /**
     * Construct a node with a URI node type and the given URI.
     *
     * Note that relative URIs (using namespace prefixes) must be
     * expanded before they be represented in a Uri.  Call
     * Store::expand() to achieve this.
     *
     * (One basic principle of this RDF library is that we use QString
     * to represent URIs that may be local or namespace prefixed, and
     * Uri to represent expanded or canonical URIs.)
     */
    Node(Uri u) : type(URI), value(u.toString()) { }

    /**
     * Construct a literal node with the given value, and with no
     * defined datatype.
     */
    Node(QString v) : type(Literal), value(v) { }

    /**
     * Construct a literal node with the given value and datatype.
     */
    Node(QString v, Uri dt) : type(Literal), value(v), datatype(dt) { }

    Node(const Node &n) :
        type(n.type), value(n.value), datatype(n.datatype) {
    }

    Node &operator=(const Node &n) {
        type = n.type; value = n.value; datatype = n.datatype;
        return *this;
    }

    ~Node() { }

    /**
     * Convert a QVariant to a Node.
     *
     * Simple QVariant types (integer, etc) are converted to literal
     * Nodes whose values are encoded as XSD datatypes, with the
     * node's value storing the XSD representation and the node's
     * datatype storing the XSD datatype URI.
     *
     * QVariants containing \ref Uri are converted to URI nodes.  Note
     * that URIs using namespace prefixes will need to be expanded
     * before they can safely be represented in a Uri or Uri QVariant.
     * Call Store::expand() to achieve this.  In general you should
     * ensure that URIs are expanded when placed in a Node object
     * rather than being stored in prefixed form.
     *
     * For QVariants whose types have been registered using
     * registerDatatype, the registered VariantEncoder's fromVariant
     * method will be used to convert the variant to a string which
     * will be stored in a literal node.
     *
     * Other QVariants, including complex structures, are converted
     * into literals containing an encoded representation which may be
     * converted back again using toVariant but cannot be directly
     * read from or interchanged using the node's value.  These types
     * are given a specific fixed datatype URI.
     */
    static Node fromVariant(const QVariant &v);

    /**
     * Convert a Node to a QVariant.
     *
     * See fromVariant for details of the conversion.
     *
     * Note that URI nodes are returned as variants with user type
     * corresponding to Uri, not as QString variants.  This may result
     * in invalid Uris if the URIs were not properly expanded on
     * construction (see the notes about fromVariant).
     */
    QVariant toVariant() const;

    /**
     * Convert a Node to a QVariant, with a nudge for the variant
     * type, used to override the default variant type corresponding
     * to the node's datatype.  This is marginally simpler than
     * setting the datatype on the node and then converting, and so
     * may be convenient in situations where the proper RDF datatype
     * is missing.
     *
     * Meaningful results still depend on having the proper encoder
     * available (i.e. the type name whose QMetaType id is metaTypeId
     * must have been registered using registerDatatype, if it is not
     * one of the types with built-in support).  If no encoder is
     * found, a QString variant will be returned instead.
     */
    QVariant toVariant(int metaTypeId) const;

    bool operator<(const Node &n) const {
        if (type != n.type) return type < n.type;
        if (value != n.value) return value < n.value;
        if (datatype != n.datatype) return datatype < n.datatype;
        return false;
    }

    /**
     * VariantEncoder is an abstract interface for classes that can
     * convert between QVariant and strings for storage in literal
     * Node objects.
     */
    struct VariantEncoder {
        virtual ~VariantEncoder() { }
        
        /**
         * Convert a string to a variant.  The VariantEncoder is
         * expected to be know the node type from which the string has
         * been obtained.
         */
        virtual QVariant toVariant(const QString &n) = 0;

        /**
         * Convert a variant to a string.  The specific VariantEncoder
         * is expected to know the node type which is the target of
         * the conversion.
         */
        virtual QString fromVariant(const QVariant &v) = 0;
    };

    /**
     * Register an association between a particular datatype URI and a
     * type which can be stored in a QVariant.  This can be used to
     * provide meaningful conversions between literal nodes and
     * QVariant objects in addition to the built-in types.
     *
     * For conversions from variant, once an association has been made
     * via this call, a subsequent call to Node::fromVariant given a
     * variant whose type is that of the given variantTypeName will
     * return a Node of the specified datatype.  If an encoder is also
     * given, it will be used for the QVariant-to-string conversion
     * needed to produce a value string in the correct form for the
     * RDF datatype.
     *
     * For conversions to variant, the encoder is required.  If it is
     * provided, then a subsequent call to Node::toVariant on a node
     * of the given datatype will result in a variant of the specified
     * type, produced by calling the encoder's toVariant method with
     * the node's value string as argument.
     */
    static void registerDatatype(Uri datatype,
                                 QString variantTypeName,
                                 VariantEncoder *encoder = 0);

    /**
     * Retrieve the datatype URI that has been associated with the
     * given variant type using \ref registerDatatype.  If no such
     * association has been made or the variant type is unknown,
     * return the empty Uri.
     */
    static Uri getDatatype(QString variantTypeName);

    /**
     * Retrieve the variant type that has been associated with the
     * given datatype Uri using \ref registerDatatype.  If no such
     * association has been made, return an empty string.
     */
    static QString getVariantTypeName(Uri datatype);
    
    Type type;
    QString value;
    Uri datatype;
};

/**
 * A list of node types.
 */
typedef QList<Node> Nodes;

bool operator==(const Node &a, const Node &b);
bool operator!=(const Node &a, const Node &b);

QDataStream &operator<<(QDataStream &out, const Node &);
QDataStream &operator>>(QDataStream &in, Node &);

std::ostream &operator<<(std::ostream &out, const Node &);
QTextStream &operator<<(QTextStream &out, const Node &);

}

#endif

