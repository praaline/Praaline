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

#ifndef _DATAQUAY_TRANSACTIONAL_STORE_H_
#define _DATAQUAY_TRANSACTIONAL_STORE_H_

#include "Transaction.h"

namespace Dataquay
{
	
/**
 * \class TransactionalStore TransactionalStore.h <dataquay/TransactionalStore.h>
 *
 * TransactionalStore is an RDF data store implementing the Store
 * interface, providing transaction support as a wrapper around a
 * non-transactional store such as a BasicStore.
 *
 * Write access to the store is permitted only in the context of a
 * transaction.  If you call a modifying function directly on
 * TransactionalStore, the store will either throw RDFException (if
 * set to NoAutoTransaction) or create a single-use Transaction object
 * for the duration of that modification (if set to AutoTransaction).
 * Note that the latter behaviour will deadlock if a transaction is in
 * progress already.
 *
 * Read access may be carried out through a Transaction, in which case
 * the read state will reflect the changes made so far in that
 * transaction, or directly on the TransactionalStore, in which case
 * the read will be isolated from any pending transaction.
 *
 * Call startTransaction to obtain a new Transaction object and start
 * its transaction; use the Transaction's Store interface for all
 * accesses associated with that transaction; call commit on the
 * Transaction object once done, to finish the transaction, and then
 * delete the object; or call Transaction::rollback() if you decide
 * you do not wish to commit it.
 *
 * TransactionalStore is thread-safe.
 *
 *!!! ... but the individual Transactions that you get from it are _not_ thread-safe -- document this (it's probably acceptable) or fix it
 */
class TransactionalStore : public QObject, public Store
{
    Q_OBJECT

public:
    /**
     * DirectWriteBehaviour controls how TransactionalStore responds
     * when called directly (not through a Transaction) for a write
     * operation (add, remove, change, or revert).
     *
     * NoAutoTransaction (the default) means that an RDF exception
     * will be thrown whenever a write is attempted without a
     * transaction.
     *
     * AutoTransaction means that a Transaction object will be
     * created, used for the single access, and then closed.  This may
     * cause a deadlock if another transaction is already ongoing
     * elsewhere.
     */
    enum DirectWriteBehaviour {
        NoAutoTransaction,
        AutoTransaction
    };

    /**
     * Create a TransactionalStore operating on the given (presumably
     * non-transactional) data store.
     *
     * Nothing in the code prevents the given _underlying_ store being
     * used non-transactionally elsewhere at the same time.  Don't do
     * that: once you have set up a transactional store, you should
     * use it for all routine accesses to the underlying store.
     */
    TransactionalStore(Store *store, DirectWriteBehaviour dwb = NoAutoTransaction);
    
    /**
     * Delete the TransactionalStore.  This does _not_ delete the
     * underlying Store that is being wrapped (the one that was passed
     * to the constructor).
     */
    ~TransactionalStore();

    /**
     * Start a transaction and obtain a Transaction through which to
     * carry out its operations.  Once the transaction is complete,
     * you must call commit on the Transaction object to finish the
     * transaction, and then you must delete the object.
     */
    Transaction *startTransaction();

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

signals:
    /**
     * Emitted after a transaction has been committed.  Note that the
     * transaction lock on the store is unlocked before the signal is
     * emitted, so that in a multi-threaded context it is possible
     * that other users of the store may have carried out further
     * transactions before this signal can be acted on.
     */
    void transactionCommitted(const ChangeSet &cs);

    /**
     * Emitted after a transaction has been committed.  Note that the
     * transaction lock on the store is unlocked before the signal is
     * emitted, so that in a multi-threaded context it is possible
     * that other users of the store may have carried out further
     * transactions before this signal can be acted on.
     */
    void transactionCommitted();

private:
    class D;
    D *m_d;

    class TSTransaction : public Transaction
    {
    public:
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

        // Transaction interface
        void commit();
        void rollback();
        ChangeSet getCommittedChanges() const;
        ChangeSet getChanges() const;

        TSTransaction(TransactionalStore::D *td);
        virtual ~TSTransaction();

    private:
        TSTransaction(const TSTransaction &);
        TSTransaction &operator=(const TSTransaction &);
        class D;
        D *m_d;
    };
};

}

#endif
