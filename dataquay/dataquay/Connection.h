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

#ifndef _DATAQUAY_CONNECTION_H_
#define _DATAQUAY_CONNECTION_H_

#include "Store.h"

namespace Dataquay
{
	
class TransactionalStore;
class Transaction;

/**
 * \class Connection Connection.h <dataquay/Connection.h>
 *
 * Connection provides a connection interface to TransactionalStore,
 * allowing it to be used in a convenient manner familiar to
 * programmers working with SQL databases.
 * 
 * Each processing thread may construct a Connection to a central
 * TransactionalStore.  The Connection will start a new Transaction on
 * the store when the first modifying function (add, remove, change or
 * revert) is called and will continue to use this Transaction for all
 * accesses to the store until either commit() or rollback() is called
 * on the Connection.
 *
 * Any read-only functions called on this class between a commit() or
 * rollback() and the next modifying function will be passed directly
 * to the store without any transaction.  Read-only functions called
 * while a transaction is in progress will be passed through the
 * current transaction, and so will read the effective state of the
 * store with the partial transaction in force.
 *
 * The Connection commits any active Transaction when it is deleted.
 * To avoid this, call rollback() before deletion.  No other
 * auto-commit functionality is provided -- if you want auto-commit,
 * use the TransactionalStore's own interface in AutoTransaction mode.
 *
 * Each Connection should be used in a single processing thread only.
 * Connection is not thread-safe.
 */
class Connection : public QObject, public Store
{
    Q_OBJECT

public:
    /**
     * Construct a Connection to the given TransactionalStore, through
     * which a series of transactions may be made in a single
     * processing thread.
     */
    Connection(TransactionalStore *ts);

    /**
     * Destroy the Connection, first committing any outstanding
     * transaction.  If you do not want any outstanding transaction to
     * be committed, call rollback() first.
     */
    ~Connection();

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

public slots:
    /**
     * Commit the outstanding Transaction, if any.
     */
    void commit();

    /**
     * Commit the outstanding Transaction, if any, and return the
     * changes committed.
     */
    ChangeSet commitAndObtain();

    /**
     * Roll back the outstanding Transaction, if any, and prepare to
     * begin a new Transaction the next time a modifying function is
     * called.
     */
    void rollback();

signals:
    /// Forwarded from TransactionalStore
    void transactionCommitted(const ChangeSet &);

    /// Forwarded from TransactionalStore
    void transactionCommitted();

private:
    class D;
    D *m_d;
};

}

#endif
