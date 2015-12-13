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

#ifndef _TEST_DATATYPES_H_
#define _TEST_DATATYPES_H_

#include <dataquay/Node.h>
#include <dataquay/BasicStore.h>
#include <dataquay/RDFException.h>

#include <QObject>
#include <QtTest>

/* StreamableValueType is a type that can be streamed to QDataStream
 * and thus converted automatically to QVariant, but that will not be
 * registered with a Node encoder -- so Node must use an "unknown
 * type" datatype
 */
enum StreamableValueType { ValueA = 0, ValueB, ValueC, ValueD, ValueE };

/* NonStreamableValueType is a type that cannot be streamed to
 * QDataStream, but that we will register with a Node encoder -- so
 * Node can give it the proper datatype
 */
enum NonStreamableValueType { ValueF = 0, ValueG, ValueH, ValueI, ValueJ };

Q_DECLARE_METATYPE(StreamableValueType)
Q_DECLARE_METATYPE(NonStreamableValueType)

extern QDataStream &operator<<(QDataStream &out, StreamableValueType v);
extern QDataStream &operator>>(QDataStream &in, StreamableValueType &v);

namespace Dataquay {

struct NonStreamableEncoder : public Node::VariantEncoder {
    QVariant toVariant(const QString &s) {
	if (s == "F") return QVariant::fromValue(ValueF);
	else if (s == "G") return QVariant::fromValue(ValueG);
	else if (s == "H") return QVariant::fromValue(ValueH);
	else if (s == "I") return QVariant::fromValue(ValueI);
	else if (s == "J") return QVariant::fromValue(ValueJ);
	else return QVariant();
    }
    QString fromVariant(const QVariant &v) {
	NonStreamableValueType nsv = v.value<NonStreamableValueType>();
	switch (nsv) {
	case ValueF: return "F";
	case ValueG: return "G";
	case ValueH: return "H";
	case ValueI: return "I";
	case ValueJ: return "J";
	default: return "";
	}
    }
};

class TestDatatypes : public QObject
{
    Q_OBJECT

private:
    Uri nsvDtUri;

private slots:
    void initTestCase() {
	store.setBaseUri(Uri("http://breakfastquay.com/rdf/dataquay/tests#"));
	qRegisterMetaType<StreamableValueType>("StreamableValueType");
	qRegisterMetaType<NonStreamableValueType>("NonStreamableValueType");
	qRegisterMetaTypeStreamOperators<StreamableValueType>("StreamableValueType");
	nsvDtUri = Uri("http://breakfastquay.com/rdf/dataquay/test/nonstreamable");
	Node::registerDatatype(nsvDtUri,
			       "NonStreamableValueType",
			       new NonStreamableEncoder());
    }

    void basicConversions() {
	Node n("Fred Jenkins", Uri());
	QVariant v = n.toVariant();
	QCOMPARE(v.userType(), (int)QMetaType::QString);
	QCOMPARE(Node::fromVariant(v).datatype, n.datatype);

	Triple t(store.expand(":fred"),
		 Uri("http://xmlns.com/foaf/0.1/name"),
		 n);
	QVERIFY(store.add(t));

	t.c = Node();
	Triple t0(store.matchOnce(t));
	Node n0 = t.c;
	QCOMPARE(n0.datatype, n.datatype);
	
	n = Node("1", store.expand("xsd:integer"));
	v = n.toVariant();
	QCOMPARE(v.userType(), (int)QMetaType::Long);

	n0 = Node::fromVariant(v);
	QCOMPARE(n0.datatype, n.datatype);

	t = Triple(store.expand(":fred"),
		   store.expand(":number_of_jobs"),
		   n);
	QVERIFY(store.add(t));

	t.c = Node();
	t0 = store.matchOnce(t);
	n0 = t0.c;
	QCOMPARE(n0.datatype, n.datatype);
    }

    void streamableTypeConversions() {
	StreamableValueType sv = ValueC;
	QVERIFY(QMetaType::type("StreamableValueType") > 0);

	// first some tests on basic QVariant storage just to ensure
	// we've registered the type correctly and our understanding
	// of expected behaviour is correct

	QVariant svv = QVariant::fromValue<StreamableValueType>(sv);
	QCOMPARE(svv.userType(), (int)QMetaType::type("StreamableValueType"));
	QCOMPARE(svv.value<StreamableValueType>(), sv);

	// we registered in initTestCase stream operators necessary to
	// permit Node-QVariant conversion using an opaque "unknown
	// type" node datatype and the standard QVariant datastream
	// streaming: test conversion that way
	
	Node n = Node::fromVariant(svv);
	
//!!! no -- the type is actually encodedVariantTypeURI from Node.cpp, and this is what we should expect -- but it's not public! we can't test it -- fix this
//    QCOMPARE(n.datatype, Uri());

	Triple t = Triple(store.expand(":fred"),
			  store.expand(":has_some_value"),
			  n);
	QVERIFY(store.add(t));

	t.c = Node();
	Triple t0 = store.matchOnce(t);
	Node n0 = t0.c;
	QCOMPARE(n0.datatype, n.datatype);
	
	QVariant v0 = n0.toVariant();
	QCOMPARE(v0.userType(), svv.userType());
	QCOMPARE(v0.value<StreamableValueType>(), svv.value<StreamableValueType>());
    }

    void nonStreamableTypeConversions() {
	NonStreamableValueType nsv = ValueJ;
	QVERIFY(QMetaType::type("NonStreamableValueType") > 0);

	// as above but using NonStreamableValueType with a
	// registered encoder, rather than StreamableValueType with no
	// encoder but a stream operator instead

	QVariant nsvv = QVariant::fromValue<NonStreamableValueType>(nsv);
	QCOMPARE(nsvv.userType(), (int)QMetaType::type("NonStreamableValueType"));
	QCOMPARE(nsvv.value<NonStreamableValueType>(), nsv);

	Node n = Node::fromVariant(nsvv);
	QCOMPARE(n.datatype, nsvDtUri);

	Triple t = Triple(store.expand(":fred"),
			  store.expand(":has_some_other_value"),
			  n);
	QVERIFY(store.add(t));

	t.c = Node();
	Triple t0 = store.matchOnce(t);
	Node n0 = t0.c;
	QCOMPARE(n0.datatype, n.datatype);
	
	QVariant v0 = n0.toVariant();
	QCOMPARE(v0.userType(), nsvv.userType());
	QCOMPARE(v0.value<NonStreamableValueType>(), nsvv.value<NonStreamableValueType>());
    }

    void explicitlyTypedRetrieval() {
	QVariant nsvv = QVariant::fromValue<NonStreamableValueType>(ValueJ);
	Node n = Node::fromVariant(nsvv);

	n.datatype = Uri();
	QVariant v0 = n.toVariant();
	// having reset the datatype, this should be the default
	QCOMPARE(v0.userType(), (int)QMetaType::QString);

	// now convert with datatype prompting
	v0 = n.toVariant(QMetaType::type("NonStreamableValueType"));
	QCOMPARE(v0.userType(), nsvv.userType());
    }

private:
    BasicStore store;
};

}

#endif
