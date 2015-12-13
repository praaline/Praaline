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

#ifndef _TEST_TRANSACTIONAL_STORE_H_
#define _TEST_TRANSACTIONAL_STORE_H_

#include <dataquay/Node.h>
#include <dataquay/BasicStore.h>
#include <dataquay/RDFException.h>
#include <dataquay/TransactionalStore.h>
#include <dataquay/Connection.h>

#include <QObject>
#include <QtTest>

namespace Dataquay {

class TestTransactionalStore : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
	store.setBaseUri(Uri("http://breakfastquay.com/rdf/dataquay/tests#"));
	ts = new TransactionalStore(&store);
    }

    void init() {
	store.clear();
    }

    void simpleAdds() {
	Transaction *t = ts->startTransaction();
	int added = 0;
	QVERIFY(addThings(t, added));
	
	// pause to test transactional isolation
	Triples triples = ts->match(Triple());
	QCOMPARE(triples.size(), 0);

	// do it again, just to check internal state isn't being bungled
	triples = ts->match(Triple());
	QCOMPARE(triples.size(), 0);

	// and check that reads *through* the transaction return the
	// partial state as expected
	triples = t->match(Triple());
	QCOMPARE(triples.size(), added);

	t->commit();
	triples = ts->match(Triple());
	QCOMPARE(triples.size(), added);

	// this is the bogus value we added and then removed in addThings
	QVERIFY(!ts->contains(Triple(store.expand(":fred"),
                                     store.expand(":age"),
                                     Node::fromVariant(QVariant(43)))));
    
	// this is the value we actually retained
	QVERIFY(ts->contains(Triple(store.expand(":fred"),
                                    store.expand(":age"),
                                    Node::fromVariant(QVariant(42)))));

	delete t;
    }

    void simpleRollback() {
	
	Transaction *t = ts->startTransaction();
	int added = 0;
	QVERIFY(addThings(t, added));
	
	t->rollback();
	delete t;

	Triples triples = ts->match(Triple());
	QCOMPARE(triples.size(), 0);
    }	

    void autoRollback() {
	// automatic rollback triggered by an exception in e.g. add
	
	Transaction *t = ts->startTransaction();
	int added = 0;
	QVERIFY(addThings(t, added));
	
	// Add incomplete statement to provoke an exception
	try {
	    t->add(Triple(Node(),
			  Uri("http://xmlns.com/foaf/0.1/name"),
			  Node("this_statement_is_incomplete")));
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}
	
	// Now everything should fail on this tx
	try {
	    t->add(Triple(store.expand(":fred2"),
			  store.expand(":is_sadly_deluded"),
			  Node::fromVariant(true)));
	    QVERIFY2(0, "add succeeded after auto-rollback, should have failed");
	} catch (RDFException) {
	    QVERIFY(1);
	}

	try {
	    (void)t->match(Triple());
	    QVERIFY2(0, "match succeeded after auto-rollback, should have failed");
	} catch (RDFException) {
	    QVERIFY(1);
	}

	// including commit
	try {
	    t->commit();
	    QVERIFY2(0, "commit succeeded after auto-rollback, should have failed");
	} catch (RDFException) {
	    QVERIFY(1);
	}

	// and rollback!
	try {
	    t->rollback();
	    QVERIFY2(0, "rollback succeed after auto-rollback, should have failed");
	} catch (RDFException) {
	    QVERIFY(1);
	}

	delete t;
    }

    void emptyTx() {
	Transaction *t = ts->startTransaction();
	t->commit();
	delete t;
	t = ts->startTransaction();
	t->rollback();
	delete t;
    }	

    void mustCommitOrRollback() {
	Transaction *t = ts->startTransaction();
	int added = 0;
	QVERIFY(addThings(t, added));

	try {
	    delete t;
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}

	// but check that this doesn't prevent any further
	// transactions from happening
	t = ts->startTransaction();
	t->rollback();
	delete t;
    }

    void noConcurrentTxInThread() {
	Transaction *t = ts->startTransaction();
	try {
	    Transaction *tt = ts->startTransaction();
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}
	t->rollback();
	delete t;
    }

    void consecutiveTxInThread() {
	Transaction *t = ts->startTransaction();
	t->commit();
	delete t;
	Transaction *tt = ts->startTransaction();
	tt->commit();
	delete tt;
    }

    void cannotUseFinishedTx() {
	Transaction *t = ts->startTransaction();
	int added = 0;
	QVERIFY(addThings(t, added));

	// Test that we can't use the transaction after a rollback
	t->rollback();
	try {
	    QVERIFY(!t->add(Triple(store.expand(":fred2"),
				   Uri("http://xmlns.com/foaf/0.1/knows"),
				   store.expand(":samuel"))));
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}

	delete t;
	t = ts->startTransaction();
	QVERIFY(addThings(t, added));

	//... or a commit
	t->commit();
	try {
	    Triples triples = t->match(Triple());
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}

	// and that we can't commit or rollback twice
	delete t;
	t = ts->startTransaction();
	t->commit();
	try {
	    t->commit();
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}
	delete t;
	t = ts->startTransaction();
	t->rollback();
	try {
	    t->rollback();
	    QVERIFY(0);
	} catch (RDFException) {
	    QVERIFY(1);
	}
	delete t;
    }
	
    void changesets() {
	Transaction *t = ts->startTransaction();
	int added = 0;
	QVERIFY(addThings(t, added));

	ChangeSet changes = t->getChanges();
	QVERIFY(!changes.empty());
	
	ChangeSet cchanges = t->getCommittedChanges();
	QVERIFY(cchanges.empty());
	
	t->commit();

	cchanges = t->getCommittedChanges();
	QCOMPARE(cchanges, changes);

	t = ts->startTransaction();
	t->revert(changes);
	t->commit();
	delete t;

	Triples triples = ts->match(Triple());
	QCOMPARE(triples.size(), 0);

	t = ts->startTransaction();
	t->change(changes);
	t->commit();
	delete t;

	triples = ts->match(Triple());
	QCOMPARE(triples.size(), added);

	// this is the bogus value we added and then removed in addThings
	QVERIFY(!ts->contains(Triple(store.expand(":fred"),
				     store.expand(":age"),
				     Node::fromVariant(QVariant(43)))));
    
	// this is the value we actually retained
	QVERIFY(ts->contains(Triple(store.expand(":fred"),
				    store.expand(":age"),
				    Node::fromVariant(QVariant(42)))));
    }

    void simpleConnection() {

	Connection *c = new Connection(ts);
	int added = 0;
	QVERIFY(addThings(c, added));

        // query on connection
        Triples triples = c->match(Triple());
        QCOMPARE(triples.size(), added);

        // query on store
        triples = ts->match(Triple());
        QCOMPARE(triples.size(), 0);

        // query on a different connection
        {
            Connection c2(ts);
            triples = c2.match(Triple());
	    QCOMPARE(triples.size(), 0);
        }

        c->commit();

        triples = c->match(Triple());
        QCOMPARE(triples.size(), added);

        triples = ts->match(Triple());
        QCOMPARE(triples.size(), added);

        c->add(Triple(store.expand(":fred"),
                     store.expand(":likes_to_think_his_age_is"),
                     Node::fromVariant(QVariant(21.9))));

        ++added;

        triples = c->match(Triple());
        QCOMPARE(triples.size(), added);

        triples = ts->match(Triple());
        QCOMPARE(triples.size(), added-1);

        c->commit();

        triples = c->match(Triple());
        QCOMPARE(triples.size(), added);

        triples = ts->match(Triple());
        QCOMPARE(triples.size(), added);

        // test implicit commit on dtor
        for (int i = 0; i < triples.size(); ++i) {
            c->remove(triples[i]);
        }

	delete c;

        triples = ts->match(Triple());
        QCOMPARE(triples.size(), 0);
    }

private:
    BasicStore store;
    TransactionalStore *ts;

    bool addThings(Store *t, int &added) {
	added = 0;
	// These add calls are things we've tested in testBasicStore already
	if (!t->add(Triple(store.expand(":fred"),
			   Uri("http://xmlns.com/foaf/0.1/name"),
			   Node("Fred Jenkins")))) return false;
	++added;
	if (!t->add(Triple(store.expand(":fred"),
			   Uri("http://xmlns.com/foaf/0.1/knows"),
			   store.expand(":alice")))) return false;
	++added;
	t->add(Triple(store.expand(":fred"),
		      store.expand(":age"),
		      Node::fromVariant(QVariant(43))));
	++added;
	if (!t->remove(Triple(store.expand(":fred"),
			      store.expand(":age"),
			      Node()))) return false;
	--added;
	if (!t->add(Triple(store.expand(":fred"),
			   store.expand(":age"),
			   Node::fromVariant(QVariant(42))))) return false;
	++added;
	return true;
    }
};

}

#endif
