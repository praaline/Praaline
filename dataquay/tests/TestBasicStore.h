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

#ifndef _TEST_BASIC_STORE_H_
#define _TEST_BASIC_STORE_H_

#include <dataquay/Node.h>
#include <dataquay/BasicStore.h>
#include <dataquay/RDFException.h>

#include <QObject>
#include <QtTest>

namespace Dataquay {

class TestBasicStore : public QObject
{
    Q_OBJECT
    
private slots:

    void initTestCase() {
	store.setBaseUri(Uri("http://breakfastquay.com/rdf/dataquay/tests#"));
	base = store.getBaseUri().toString();
	count = 0;
	fromFred = 0;
        usingKnows = 0;
	toAlice = 0;
    }

    void makeUri() {

        // create absolute URI
        QString s("http://breakfastquay.com/rdf/dataquay/");
        Uri uri(s);
        QCOMPARE(uri.length(), s.length());

        // fail to create relative or bogus URI in a few different ways

        try {
            Uri uri("/rdf/dataquay");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }

        try {
            Uri uri("#x");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }

        try {
            Uri uri("x");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }

        try {
            Uri uri("<http://breakfastquay.com/rdf/dataquay/>");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }

        // "a" is a special case
        uri = Uri("a");
        QCOMPARE(uri, Uri::rdfTypeUri());

        // succeed in creating relative file URI, Uri should fix it for us
        uri = Uri("file:a");
        QVERIFY(uri.toString() == "file://a");

        uri = Uri("file:/a/b");
        QVERIFY(uri.toString() == "file:///a/b");

        // but we don't like any other prefix without //
        try {
            uri = Uri("blah:a");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }
        try {
            uri = Uri("blah:/a");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }

        // succeed in creating absolute URI through store.expand
        uri = store.expand(":relative");
        QCOMPARE(uri.length(), store.getBaseUri().length() + 8);

        // but fail in turning bogus URI into good one
        try {
            uri = store.expand("bogus");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }

        // and fail in expanding a relative URI through a store with
        // no base URI

        BasicStore s1;
        try {
            uri = s1.expand(":relative");
            QVERIFY(0);
        } catch (RDFIncompleteURI &) {
            QVERIFY(1);
        }
    }

    void simpleAdd() {

	// check triple can be added
	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			Uri("http://xmlns.com/foaf/0.1/name"),
			Node("Fred Jenkins"))));
	++count;
	++fromFred;

        // alternative Triple constructor
	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			Uri("http://xmlns.com/foaf/0.1/knows"),
			store.expand(":alice"))));
	++count;
	++fromFred;
        ++usingKnows;
	++toAlice;
    }

    void simpleLookup() {
	// check triple just added can be found again
	QVERIFY(store.contains
		(Triple(store.expand(":fred"),
			Node(Uri("http://xmlns.com/foaf/0.1/name")),
			Node("Fred Jenkins"))));
    }

    void simpleAbsentLookup() {
	// check absent triple lookups are correctly handled
	QVERIFY(!store.contains
		(Triple(store.expand(":fred"),
			Node(Uri("http://xmlns.com/foaf/0.1/name")),
			Node("Fred Johnson"))));
    }

    void addFromVariantInt() {
        // variant conversion
        QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":age"),
			Node::fromVariant(QVariant(42)))));
        ++count;
        ++fromFred;
	Triples triples = store.match
	    (Triple(store.expand(":fred"),
		    store.expand(":age"),
		    Node()));
	QCOMPARE(triples.size(), 1);
	QCOMPARE(triples[0].c.toVariant().toInt(), 42);
    }	

    void addFromVariantURI() {
        // variant conversion
	Uri fredUri("http://breakfastquay.com/rdf/person/fred");
	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":has_some_uri"),
			Node::fromVariant
			(QVariant::fromValue(fredUri)))));
        ++count;
        ++fromFred;

	QVERIFY(store.add
		(Triple(store.expand(":fred"),
                        store.expand(":has_some_local_uri"),
			Node::fromVariant
			(QVariant::fromValue(store.expand(":pootle"))))));
        ++count;
        ++fromFred;

	Triples triples = store.match
	    (Triple(store.expand(":fred"),
		    store.expand(":has_some_uri"),
		    Node()));
	QCOMPARE(triples.size(), 1);
	QCOMPARE(Uri(triples[0].c.value), fredUri);

	triples = store.match
	    (Triple(store.expand(":fred"),
		    store.expand(":has_some_local_uri"),
		    Node()));
	QCOMPARE(triples.size(), 1);
	QCOMPARE(triples[0].c.toVariant().value<Uri>(), store.expand(":pootle"));
    }

    void addFromVariantFloat() {
        // variant conversion
        QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":likes_to_think_his_age_is"),
			Node::fromVariant(QVariant(21.9)))));
        ++count;
        ++fromFred;
    }

    void addFromVariantBool() {
        // variant conversion
        QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":is_sadly_deluded"),
			Node::fromVariant(true))));
        ++count;
        ++fromFred;
        Triples triples = store.match
	    (Triple(store.expand(":fred"),
		    store.expand(":is_sadly_deluded"),
		    Node()));
	QCOMPARE(triples.size(), 1);
	QCOMPARE(triples[0].c.toVariant().toBool(), true);
    }

    void addFromVariantList() {
        // variant conversion
        QStringList colours;
        colours << "turquoise";
        colours << "red";
        colours << "black";
	QCOMPARE(colours.size(), 3);
	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			store.expand(":favourite_colours_are"),
                        Node::fromVariant(QVariant(colours)))));
        ++count;
        ++fromFred;
	Triples triples = store.match
	    (Triple(store.expand(":fred"),
		    store.expand(":favourite_colours_are"),
		    Node()));
	QCOMPARE(triples.size(), 1);
	QStringList retrievedColours = triples[0].c.toVariant().toStringList();
	QCOMPARE(colours, retrievedColours);
    }

    void addWithRdfTypeBuiltin() {
        // rdf:type builtin
	QVERIFY(store.add
		(Triple(store.expand(":fred"),
			Uri("a"),
			store.expand(":person"))));
	QVERIFY(store.contains
		(Triple(store.expand(":fred"),
			store.expand("rdf:type"),
			store.expand(":person"))));
        ++count;
        ++fromFred;
    }

    void addUsingPrefix() {
	// prefix expansion
        store.addPrefix("foaf", Uri("http://xmlns.com/foaf/0.1/"));
	QVERIFY(store.add
		(Triple(store.expand(":alice"),
			store.expand("foaf:knows"),
			store.expand(":fred"))));
	QVERIFY(store.contains
		(Triple(store.expand(":alice"),
			Node(Uri("http://xmlns.com/foaf/0.1/knows")),
			store.expand(":fred"))));
        QVERIFY(store.add
                (Triple(store.expand(":alice"),
                        store.expand("foaf:name"),
                        Node(QString("Alice Banquet")))));              
        ++usingKnows;
        ++count;
        ++count;
    }

    void addDuplicate() {
        // we try to add a triple that is a differently-expressed
        // duplicate of an already-added one -- this should be ignored
        // (returning false) and we do not increment our count
	QVERIFY(!store.add
		(Triple(Uri(base + "alice"),
			Uri("http://xmlns.com/foaf/0.1/name"),
			Node(QString("Alice Banquet")))));

        // now we try to add a triple a bit like an existing one but
        // differing in prefix -- this should succeed, and we do increment
        // our count, although this is not a very useful statement
	QVERIFY(store.add
		(Triple(store.expand(":alice"),
			Uri("http://xmlns.com/foaf/0.1/knows"),
			store.expand("foaf:fred"))));
        ++usingKnows;
        ++count;
    }

    void addBlanks() {
        // things involving blank nodes
        Node blankNode = store.addBlankNode();
        QVERIFY(store.add
		(Triple(store.expand(":fred"),
			Uri("http://xmlns.com/foaf/0.1/maker"),
			blankNode)));
        ++count;
        ++fromFred;

	QVERIFY(store.add
		(Triple(blankNode,
			store.expand("foaf:name"),
			Node("Omnipotent Being"))));
        ++count;
    }

    void addBlankPredicateFail() {
	// can't have a blank node as predicate
	Node anotherBlank = store.addBlankNode();
	try {
	    QVERIFY(!store.add
		    (Triple(store.expand(":fred"),
			    anotherBlank,
			    Node("this_statement_is_incomplete"))));
	} catch (RDFException &) {
	    QVERIFY(1);
	}
    
    }

    void matchCounts() {
	// Must run after adds. Check match-all, and matches with each
	// of S, P and O primary
	QCOMPARE(store.match(Triple()).size(), count);
	QCOMPARE(store.match
		 (Triple(store.expand(":fred"), Node(), Node())).size(),
		 fromFred);
	QCOMPARE(store.match
		 (Triple(Node(), store.expand("foaf:knows"), Node())).size(),
		 usingKnows);
	QCOMPARE(store.match
		 (Triple(Node(), Node(), store.expand(":alice"))).size(),
		 toAlice);
    }

    void compareTriples() {

	// check empty Triples match
	QVERIFY(Triples().matches(Triples()));

	// check two identical searches return matching (non-empty) results
        Triples t1 = store.match(Triple(store.expand(":fred"), Node(), Node()));
        Triples t2 = store.match(Triple(store.expand(":fred"), Node(), Node()));
	QVERIFY(t1.size() > 0);
	QVERIFY(!t1.matches(Triples()));
	QVERIFY(t1.matches(t2));
	QVERIFY(t2.matches(t1));

	// check Triples matches itself in a different order
	t2 = Triples();
        foreach (Triple t, t1) t2.push_front(t);
	QVERIFY(t1.matches(t2));
	QVERIFY(t2.matches(t1));

	// check two different searches return non-matching results
	t2 = store.match(Triple(store.expand(":alice"), Node(), Node()));
        QVERIFY(!t1.matches(t2));
        QVERIFY(!t2.matches(t1));
    }

    void sliceTriples() {
        Triples tt = store.match(Triple());
        QCOMPARE(tt.size(), count);
        Nodes nna = tt.subjects();
        QCOMPARE(nna.size(), count);
        foreach (Node n, nna) {
            QVERIFY(n.type == Node::URI || n.type == Node::Blank);
        }
        Nodes nnb = tt.predicates();
        QVERIFY(nnb != nna);
        QCOMPARE(nnb.size(), count);
        foreach (Node n, nnb) {
            QVERIFY(n.type == Node::URI);
        }
        Nodes nnc = tt.objects();
        QVERIFY(nnc != nna);
        QCOMPARE(nnc.size(), count);
    }

    void query() {
	
        QString q = QString(" SELECT ?a "
                            " WHERE { :fred foaf:knows ?a } ");
        ResultSet results;

	// We cannot perform queries without an absolute base URI,
	// it seems, because the query engine takes relative URIs
	// as relative to the query URI and we can't override that
	// without an absolute prefix for the base URI

	try {
	    results = store.query(q);
	    QCOMPARE(results.size(), 1);

	    Node v = store.queryOnce(q, "a");
	    QString expected = base + "alice";
	    QCOMPARE(v.type, Node::URI);
	    QCOMPARE(v.value, expected);

	} catch (RDFUnsupportedError &e) {
#if (QT_VERSION >= 0x050000)
	    QSKIP("SPARQL queries not supported by current store backend");
#else
	    QSKIP("SPARQL queries not supported by current store backend",
                  SkipSingle);
#endif
	}
    }

    void complete() {
        
        Node n = store.complete
            (Triple(store.expand(":alice"),
                    store.expand("foaf:name"),
                    Node()));

        QCOMPARE(n, Node("Alice Banquet"));

        n = store.complete
            (Triple(store.expand(":alice"),
                    Node(),
                    Node("Alice Banquet")));
        
        QCOMPARE(n, Node(store.expand("foaf:name")));

        n = store.complete
            (Triple(Node(),
                    store.expand("foaf:name"),
                    Node("Alice Banquet")));

        QCOMPARE(n, Node(store.expand(":alice")));

        try {
            n = store.complete
                (Triple(store.expand(":alice"),
                        store.expand("foaf:name"),
                        Node("Alice Banquet")));
            QVERIFY(0);
        } catch (RDFException &) {
            // can't complete a complete triple
            QVERIFY(1);
        }

        try {
            n = store.complete
                (Triple(Node(),
                        store.expand("foaf:name"),
                        Node()));
            QVERIFY(0);
        } catch (RDFException &) {
            // can't complete an underspecified triple
            QVERIFY(1);
        }
    }

    void saveAndLoad() {
	
        store.save("test.ttl");

        BasicStore *store2 = BasicStore::load(QUrl("file:test.ttl"));
	QVERIFY(store2);

	store2->save("test2.ttl");
	
	QCOMPARE(store2->match(Triple()).size(), count);
	QCOMPARE(store2->match
		 (Triple(store.expand(":fred"), Node(), Node())).size(),
		 fromFred);
	QCOMPARE(store2->match
		 (Triple(Node(), Node(), store.expand(":alice"))).size(),
		 toAlice);
	
	delete store2;
	
	store.clear();
	store.import(QUrl("file:test2.ttl"),
		     BasicStore::ImportFailOnDuplicates);
	
	QCOMPARE(store.match(Triple()).size(), count);
	QCOMPARE(store.match
		 (Triple(store.expand(":fred"), Node(), Node())).size(),
		 fromFred);
	QCOMPARE(store.match
		 (Triple(Node(), Node(), store.expand(":alice"))).size(),
		 toAlice);
    }	

    //!!! todo: files with explicit @base in file

    void loadRelative() {

        // Make test file without a base URI

        QFile f("test3.ttl");
        QVERIFY(f.open(QFile::WriteOnly | QFile::Truncate));
        QTextStream ts(&f);
        ts << "@prefix : <#> ." << endl << ":thing a :wotsit ." << endl;
        ts.flush();
        f.close();

        // If a file has no base URI, importing it into an existing
        // store with a base should result in URLs relative to that
        // base

        BasicStore s1;
        s1.setBaseUri(Uri("http://wox/"));
        s1.import(QUrl("file:test3.ttl"), BasicStore::ImportIgnoreDuplicates);
        Triple t = s1.matchOnce(Triple(Node(), Uri("a"), Node()));
        QCOMPARE(t.a, Node(Uri("http://wox/#thing")));
        QCOMPARE(t.c, Node(Uri("http://wox/#wotsit")));

        // And loading it should result in URLs relative to the file
        // URL

        BasicStore *s2 = BasicStore::load(QUrl("file://test3.ttl"));
        t = s2->matchOnce(Triple(Node(), Uri("a"), Node()));
        QCOMPARE(t.a, Node(Uri("file://test3.ttl#thing")));
        QCOMPARE(t.c, Node(Uri("file://test3.ttl#wotsit")));
    }

    void loadMultiBase() {
        // save two stores with different base URIs to files, check we
        // can reload them into the same store (i.e. that we resolve
        // the CURIEs properly relative to file base URI not target
        // store base URI on loading)
        BasicStore *otherStore = new BasicStore;
        otherStore->setBaseUri(Uri("http://breakfastquay.com/rdf/dataquay/tests/other#"));
        // this must be a triple we have already added to the first
        // store, using a local URI -- we are checking here for
        // duplicates on import
	QVERIFY(otherStore->add
		(Triple(otherStore->expand(":fred"),
			Node(Uri("http://xmlns.com/foaf/0.1/name")),
			Node("Fred Jenkins"))));

        // and this is to test that the reloaded :fred, despite having
        // the same name, is a different individual from the :fred in
        // our first store
        QVERIFY(otherStore->add
		(Triple(otherStore->expand(":fred"),
			otherStore->expand(":age"),
			Node::fromVariant(QVariant(3)))));
        
        store.save("multi-a.ttl");
        otherStore->save("multi-b.ttl");

        BasicStore *target = new BasicStore;
        target->setBaseUri(store.getBaseUri());
        target->import(QUrl("file:multi-a.ttl"), BasicStore::ImportIgnoreDuplicates);
        QVERIFY(target);

        QCOMPARE(target->getBaseUri(), store.getBaseUri());

        target->import(QUrl("file:multi-b.ttl"), Store::ImportFailOnDuplicates);

        QVERIFY(target->contains
                (Triple(store.expand(":fred"),
                        Uri("http://xmlns.com/foaf/0.1/name"),
                        Node("Fred Jenkins"))));
            
        QVERIFY(target->contains
                (Triple(Uri("http://breakfastquay.com/rdf/dataquay/tests#fred"),
                        Uri("http://xmlns.com/foaf/0.1/name"),
                        Node("Fred Jenkins"))));
            
        QVERIFY(target->contains
                (Triple(Node(Uri("http://breakfastquay.com/rdf/dataquay/tests/other#fred")),
                        Node(Uri("http://xmlns.com/foaf/0.1/name")),
                        Node("Fred Jenkins"))));
            
        QVERIFY(target->contains
                (Triple(Uri("http://breakfastquay.com/rdf/dataquay/tests/other#fred"),
                        Uri("http://breakfastquay.com/rdf/dataquay/tests/other#age"),
                        Node("3", store.expand("xsd:integer")))));
            
        QVERIFY(target->contains
                (Triple(Node(Uri("http://breakfastquay.com/rdf/dataquay/tests#fred")),
                        store.expand(":age"),
                        Node("42", store.expand("xsd:integer")))));
    }

    void loadCompetingBlanks() {

        // import from two files which use the same blank genid for
        // different purposes and establish that the two don't
        // conflict with each other

        {
            QFile f("test-blank-1.ttl");
            QVERIFY(f.open(QFile::WriteOnly | QFile::Truncate));
            QTextStream ts(&f);
            ts << "@prefix : <#> ." << endl;
            ts << ":thing :has _:genid1 ." << endl;
            ts << "_:genid1 :name \"weevil\" ." << endl;
            ts.flush();
            f.close();
        }

        {
            QFile f("test-blank-2.ttl");
            QVERIFY(f.open(QFile::WriteOnly | QFile::Truncate));
            QTextStream ts(&f);
            ts << "@prefix : <#> ." << endl;
            ts << ":other :has _:genid1 ." << endl;
            ts << "_:genid1 :name \"squidlet\" ." << endl;
            ts.flush();
            f.close();
        }

        BasicStore *s = BasicStore::load(QUrl("file:test-blank-1.ttl"));
        s->import(QUrl("file:test-blank-2.ttl"), Store::ImportFailOnDuplicates);

        Triple t1 = s->matchOnce(Triple(Node(), Node(), Node("weevil")));
        Triple t2 = s->matchOnce(Triple(Node(), Node(), Node("squidlet")));
        QVERIFY(t1.a != t2.a);

        delete s;
    }

    void remove() {
	// check we can remove a triple
	QVERIFY(store.remove
		(Triple(store.expand(":fred"),
			Uri("http://xmlns.com/foaf/0.1/knows"),
			store.expand(":alice"))));
	--count;
	--fromFred;
	--toAlice;

	// check we can't remove a triple that does not exist in store
        QVERIFY(!store.remove
		(Triple(store.expand(":fred"),
			Uri("http://xmlns.com/foaf/0.1/knows"),
			store.expand(":tammy"))));

        Triples triples = store.match(Triple());
	QCOMPARE(triples.size(), count);
    }

    void removeMatch() {
	// check we can remove triples matching wildcard pattern
	QVERIFY(store.remove(Triple(store.expand(":fred"), Node(), Node())));
        Triples triples = store.match(Triple());
	QCOMPARE(triples.size(), count - fromFred);
        
	QVERIFY(store.remove(Triple(Node(), Node(), Node())));
        triples = store.match(Triple());
	QCOMPARE(triples.size(), 0);
    }

private:
    BasicStore store;
    QString base;
    int count;
    int fromFred;
    int usingKnows;
    int toAlice;
};

}

#endif
