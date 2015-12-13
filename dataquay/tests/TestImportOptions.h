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

#ifndef _TEST_IMPORT_OPTIONS_H_
#define _TEST_IMPORT_OPTIONS_H_

#include <QObject>
#include <QtTest>

#include <dataquay/Node.h>
#include <dataquay/BasicStore.h>
#include <dataquay/RDFException.h>
#include <dataquay/TransactionalStore.h>
#include <dataquay/Connection.h>

namespace Dataquay {

class TestImportOptions : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {

	count = 0;
	store.setBaseUri(Uri("http://breakfastquay.com/rdf/dataquay/tests#"));

	// If TestBasicStore has passed, these should be no problem

	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			Uri("http://xmlns.com/foaf/0.1/name"),
			Node("Fred Jenkins"))));
	++count;

	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":age"),
			Node::fromVariant(QVariant(42)))));
	++count;

	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":has_some_uri"),
			Node::fromVariant(QVariant::fromValue
					  (Uri("http://breakfastquay.com/rdf/person/fred"))))));
	++count;

	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":has_some_local_uri"),
			Node::fromVariant(QVariant::fromValue
					  (store.expand(":pootle"))))));
	++count;

	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":likes_to_think_his_age_is"),
			Node::fromVariant(QVariant(21.9)))));
	++count;

	filename = "import-test.ttl";
	fileUrl = QUrl("file:" + filename);
	store.save(filename);
    }

    void normal() {
	store.clear();
	store.import(fileUrl, BasicStore::ImportIgnoreDuplicates);
	Triples tt = store.match(Triple());
	QCOMPARE(tt.size(), count);
    }

    void duplicatesFail() {
	store.clear();
	store.import(fileUrl, BasicStore::ImportFailOnDuplicates);
	bool caught = false;
	try {
	    store.import(fileUrl, BasicStore::ImportFailOnDuplicates);
	} catch (RDFDuplicateImportException) {
	    caught = true;
	}
	QVERIFY2(caught, "Failed to catch RDFDuplicateImportException when importing duplicate graph with ImportFailOnDuplicates");
	Triples tt = store.match(Triple());
	QCOMPARE(tt.size(), count);
    }

    void duplicatesIgnored() {
	store.clear();
	store.import(fileUrl, BasicStore::ImportFailOnDuplicates);
	store.import(fileUrl, BasicStore::ImportIgnoreDuplicates);
	// note -- the number of triples after import is only the same
	// as count because there are no blank nodes involved, so all
	// nodes are duplicates. Blank nodes synthesised during import
	// can't be identified as identical to existing blank nodes,
	// so they would load multiple times successfully leaving us
	// with semantically identical triples in the store
	Triples tt = store.match(Triple());
	QCOMPARE(tt.size(), count);
    }
	
    void duplicatesPermitted() {
	store.clear();
	store.import(fileUrl, BasicStore::ImportFailOnDuplicates);
	store.import(fileUrl, BasicStore::ImportPermitDuplicates);
	// we can say nothing about the number of triples in store
	// here as it all depends on the back end (this is not a
	// useful mode if the store already has something in it, it's
	// just the fastest)
    }
    
    void transactionalIsolation() {

	// testing only isolation during import, there is a separate
	// TransactionalStore test suite

	TransactionalStore ts(&store);
	Connection c(&ts);
	c.remove(Triple());
	c.commit();

	c.import(fileUrl, BasicStore::ImportIgnoreDuplicates);

        {
            Connection c2(&ts);
	    // unrelated connection, should be isolated
            QCOMPARE(c2.match(Triple()).size(), 0);
        }

        c.commit();

        {
            Connection c2(&ts);
            QCOMPARE(c2.match(Triple()).size(), count);
        }
    }        

private:
    BasicStore store;
    QString filename;
    QUrl fileUrl;
    int count;
};

}

#endif
