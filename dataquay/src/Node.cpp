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

#include "Node.h"

#include "Debug.h"

#include <QDataStream>
#include <QTextStream>
#include <QTime>
#include <QByteArray>
#include <QMetaType>
#include <QMutex>
#include <QHash>

namespace Dataquay
{

static const Uri encodedVariantTypeURI
("http://breakfastquay.com/dataquay/datatype/encodedvariant");

static const Uri xsdPrefix
("http://www.w3.org/2001/XMLSchema#");


struct StandardVariantEncoder : public Node::VariantEncoder {
    QString fromVariant(const QVariant &v) {
        return v.toString();
    }
};

struct LongVariantEncoder : public StandardVariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<long>(s.toLong());
    }
};

struct ULongVariantEncoder : public StandardVariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<unsigned long>(s.toULong());
    }
};

struct DoubleVariantEncoder : public StandardVariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<double>(s.toDouble());
    }
};

struct StringVariantEncoder : public StandardVariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<QString>(s);
    }
};

struct BoolVariantEncoder : public Node::VariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<bool>((s == "true") ||
                                         (s == "1"));
    }
    QString fromVariant(const QVariant &v) {
        return (v.toBool() ? "true" : "false");
    }
};

struct UriVariantEncoder : public Node::VariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<Uri>(Uri(s));
    }
    QString fromVariant(const QVariant &v) {
        return v.value<Uri>().toString();
    }
};

struct QUrlVariantEncoder : public Node::VariantEncoder {
    QVariant toVariant(const QString &s) {
        return QVariant::fromValue<QUrl>(QUrl(s));
    }
    QString fromVariant(const QVariant &v) {
        return v.value<QUrl>().toString();
    }
};

// Type maps to be used when converting from Node to Variant and
// Variant to Node, respectively.  These are not symmetrical -- for
// example, we convert xsd:string to QString, but convert QString to
// an untyped literal ("literal" and "literal"^^xsd:string compare
// differently and most people just use "literal", so we're more
// likely to achieve interoperable results if we don't type plain
// strings).  Similarly we convert both double and float to
// xsd:decimal, but always convert xsd:decimal to double.  However, we
// do currently impose symmetry for user-provided types (that is, our
// registerDatatype method adds the datatype to both maps).

class DatatypeMetatypeAssociation
{
public:
    static DatatypeMetatypeAssociation *instance() {
        static DatatypeMetatypeAssociation *inst = 0;
        static QMutex mutex;
        mutex.lock();
        if (inst == 0) inst = new DatatypeMetatypeAssociation();
        mutex.unlock();
        return inst;
    }
    
    int getMetatypeId(Uri dt) {
        DatatypeMetatypeMap::const_iterator i = datatypeMetatypeMap.find(dt);
        if (i != datatypeMetatypeMap.end()) return i->first;
        return 0;
    }

    bool getDatatype(int mt, Uri &dt) {
        MetatypeDatatypeMap::const_iterator i = metatypeDatatypeMap.find(mt);
        if (i != metatypeDatatypeMap.end()) {
            dt = i->first;
            return true;
        }
        return false;
    }

    Node::VariantEncoder *getEncoder(Uri dt) {
        DatatypeMetatypeMap::const_iterator i = datatypeMetatypeMap.find(dt);
        if (i != datatypeMetatypeMap.end()) return i->second;
        return 0;
    }

    Node::VariantEncoder *getEncoder(int id) {
        MetatypeDatatypeMap::const_iterator i = metatypeDatatypeMap.find(id);
        if (i != metatypeDatatypeMap.end()) return i->second;
        return 0;
    }

    void registerDatatype(Uri dt, int id, Node::VariantEncoder *enc) {
        datatypeMetatypeMap[dt] = QPair<int, Node::VariantEncoder *>(id, enc);
    }

    void registerDatatype(int id, Uri dt, Node::VariantEncoder *enc) {
        metatypeDatatypeMap[id] = QPair<Uri, Node::VariantEncoder *>(dt, enc);
    }

    void registerXsd(QString name, int id, Node::VariantEncoder *enc) {
        registerDatatype(Uri(xsdPrefix.toString() + name), id, enc);
    }

    void registerXsd(int id, QString name, Node::VariantEncoder *enc) {
        registerDatatype(id, Uri(xsdPrefix.toString() + name), enc);
    }

private:
    DatatypeMetatypeAssociation() {
        registerXsd("string", QMetaType::QString, new StringVariantEncoder());
        registerXsd("boolean", QMetaType::Bool, new BoolVariantEncoder());
        registerXsd("int", QMetaType::Int, new LongVariantEncoder());
        registerXsd("long", QMetaType::Long, new LongVariantEncoder());
        registerXsd("integer", QMetaType::Long, new LongVariantEncoder());
        registerXsd("unsignedInt", QMetaType::UInt, new ULongVariantEncoder());
        registerXsd("nonNegativeInteger", QMetaType::ULong, new ULongVariantEncoder());
        registerXsd("float", QMetaType::Float, new DoubleVariantEncoder());
        registerXsd("double", QMetaType::Double, new DoubleVariantEncoder());
        registerXsd("decimal", QMetaType::Double, new DoubleVariantEncoder());

        registerXsd(QMetaType::Bool, "boolean", new BoolVariantEncoder());
        registerXsd(QMetaType::Int, "integer", new LongVariantEncoder());
        registerXsd(QMetaType::Long, "integer", new LongVariantEncoder());
        registerXsd(QMetaType::UInt, "integer", new ULongVariantEncoder());
        registerXsd(QMetaType::ULong, "integer", new ULongVariantEncoder());
        registerXsd(QMetaType::Float, "decimal", new DoubleVariantEncoder());
        registerXsd(QMetaType::Double, "decimal", new DoubleVariantEncoder());

        // Not necessary in normal use, because URIs are stored in URI
        // nodes and handled separately rather than being stored in
        // literal nodes... but necessary if an untyped literal is
        // presented for conversion via toVariant(Uri::metaTypeId())
        registerDatatype(Uri::metaTypeId(), Uri(), new UriVariantEncoder());

        // Similarly, although no datatype is associated with QUrl, it
        // could be presented when trying to convert a URI Node using
        // an explicit variant type target (e.g. to assign RDF URIs to
        // QUrl properties rather than Uri ones)
        registerDatatype(QMetaType::QUrl, Uri(), new QUrlVariantEncoder());

        // QString is a known variant type, but has no datatype when
        // writing (we write strings as untyped literals because
        // that's what seems most useful).  We already registered it
        // with xsd:string for reading.
        registerDatatype(QMetaType::QString, Uri(), new StringVariantEncoder());
    }

    typedef QHash<Uri, QPair<int, Node::VariantEncoder *> > DatatypeMetatypeMap;
    DatatypeMetatypeMap datatypeMetatypeMap;

    typedef QHash<int, QPair<Uri, Node::VariantEncoder *> > MetatypeDatatypeMap;
    MetatypeDatatypeMap metatypeDatatypeMap;
};

void
Node::registerDatatype(Uri dt, QString typeName, VariantEncoder *enc)
{
    QByteArray ba = typeName.toLocal8Bit();
    int id = QMetaType::type(ba.data());
    if (id <= 0) {
        std::cerr << "WARNING: Node::registerDatatype: Type name \""
                  << typeName.toLocal8Bit().data() << "\" is unknown to QMetaType, "
                  << "cannot register it here" << std::endl;
        return;
    }
    DatatypeMetatypeAssociation::instance()->registerDatatype(dt, id, enc);
    DatatypeMetatypeAssociation::instance()->registerDatatype(id, dt, enc);
}
    
Uri
Node::getDatatype(QString typeName)
{
    QByteArray ba = typeName.toLocal8Bit();
    int id = QMetaType::type(ba.data());
    if (id <= 0) return Uri();
    Uri dt;
    if (DatatypeMetatypeAssociation::instance()->getDatatype(id, dt)) return dt;
    return Uri();
}

QString
Node::getVariantTypeName(Uri datatype)
{
    int id = DatatypeMetatypeAssociation::instance()->getMetatypeId(datatype);
    if (id > 0) return QMetaType::typeName(id);
    else return QString();
}

Node
Node::fromVariant(const QVariant &v)
{
    DEBUG << "Node::fromVariant: QVariant type is " << v.userType()
          << " (" << int(v.userType()) << "), variant is " << v << endl;

    if (Uri::hasUriType(v)) {
        return Node(v.value<Uri>());
    }
    if (v.type() == QVariant::Url) {
        QString s = v.toUrl().toString();
        if (Uri::isCompleteUri(s)) {
            return Node(Uri(s));
        }
    }

    int id = v.userType();
    
    DatatypeMetatypeAssociation *a = DatatypeMetatypeAssociation::instance();
    Uri datatype;
    if (a->getDatatype(id, datatype)) {
        
        Node n;
        n.type = Literal;
        n.datatype = datatype;

        VariantEncoder *encoder = a->getEncoder(id);

        if (encoder) {
            n.value = encoder->fromVariant(v);
        } else {
            n.value = v.toString();
        }

        return n;

    } else {

        // unknown type: use opaque encoding
        QByteArray b;
        QDataStream ds(&b, QIODevice::WriteOnly);
        ds << v;
        
        Node n;
        n.type = Literal;
        n.datatype = encodedVariantTypeURI;
        n.value = QString::fromLatin1(qCompress(b).toBase64());        
        return n;
    }
}

QVariant
Node::toVariant() const
{
    if (type == URI) {
        return QVariant::fromValue(Uri(value));
    }

    if (type == Nothing || type == Blank) {
        return QVariant();
    }

    if (datatype == Uri()) {
        return QVariant::fromValue<QString>(value);
    }
    
    if (datatype == encodedVariantTypeURI) {
        // Opaque encoding used for "unknown" types.  If this is
        // encoding is in use, we must decode from it even if the type
        // is actually known
        QByteArray benc = value.toLatin1();
        QByteArray b = qUncompress(QByteArray::fromBase64(benc));
        QDataStream ds(&b, QIODevice::ReadOnly);
        QVariant v;
        ds >> v;
        return v;
    }

    DatatypeMetatypeAssociation *a = DatatypeMetatypeAssociation::instance();
    int id = a->getMetatypeId(datatype);
        
    if (id > 0) {
        
        VariantEncoder *encoder = a->getEncoder(datatype);

        if (encoder) {
            return encoder->toVariant(value);
        } else {
            // datatype present, but no encoder: can do nothing
            // interesting with this, convert as string
            return QVariant::fromValue<QString>(value);
        }
        
    } else {
        
        // unknown datatype, but not "unknown type" encoding;
        // convert as a string
        return QVariant::fromValue<QString>(value);
    }
}

QVariant
Node::toVariant(int metatype) const
{
    VariantEncoder *encoder =
        DatatypeMetatypeAssociation::instance()->getEncoder(metatype);

    if (!encoder) {
        std::cerr << "WARNING: Node::toVariant: Unsupported metatype id "
                  << metatype << " (\"" << QMetaType::typeName(metatype)
                  << "\"), register it with registerDatatype please"
                  << std::endl;
        return QVariant();
    }

    return encoder->toVariant(value);
}

bool
operator==(const Node &a, const Node &b)
{
    if (a.type == Node::Nothing &&
        b.type == Node::Nothing) return true;
    if (a.type == b.type &&
        a.value == b.value &&
        a.datatype == b.datatype) return true;
    return false;
}

bool
operator!=(const Node &a, const Node &b)
{
    return !operator==(a, b);
}

QDataStream &
operator<<(QDataStream &out, const Node &n)
{
    return out << (int)n.type << n.value << n.datatype;
}

QDataStream &
operator>>(QDataStream &in, Node &n)
{
    int t;
    in >> t >> n.value >> n.datatype;
    n.type = (Node::Type)t;
    return in;
}

std::ostream &
operator<<(std::ostream &out, const Node &n)
{
    switch (n.type) {
    case Node::Nothing:
        out << "[]";
        break;
    case Node::URI:
        if (n.value == "") {
            out << "[empty-uri]";
        } else {
            out << "<" << n.value.toLocal8Bit().data() << ">";
        }
        break;
    case Node::Literal:
        out << "\"" << n.value.toLocal8Bit().data() << "\"";
        if (n.datatype != Uri()) out << "^^" << n.datatype;
        break;
    case Node::Blank:
        out << "[blank " << n.value.toLocal8Bit().data() << "]";
        break;
    }
    return out;
}

QTextStream &
operator<<(QTextStream &out, const Node &n)
{
    switch (n.type) {
    case Node::Nothing:
        out << "[]";
        break;
    case Node::URI:
        if (n.value == "") {
            out << "[empty-uri]";
        } else {
            out << "<" << n.value << ">";
        }
        break;
    case Node::Literal:
        out << "\"" << n.value << "\"";
        if (n.datatype != Uri()) out << "^^" << n.datatype;
        break;
    case Node::Blank:
        out << "[blank " << n.value << "]";
        break;
    }
    return out;
}

}

unsigned int
qHash(const Dataquay::Node &n)
{
    switch (n.type) {
    case Dataquay::Node::URI:
        return qHash(n.value);
    case Dataquay::Node::Literal:
        return qHash(n.value + n.datatype.toString());
    case Dataquay::Node::Blank:
        return qHash(n.value);
    default:
        return qHash("");
    }
}


