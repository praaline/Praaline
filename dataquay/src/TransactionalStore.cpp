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

#include "TransactionalStore.h"
#include "BasicStore.h"
#include "RDFException.h"
#include "Debug.h"

#include <QMutex>
#include <QMutexLocker>

#include <iostream>
#include <memory> // auto_ptr

using std::auto_ptr;

namespace Dataquay
{
	
class TransactionalStore::D
{
    /**
     * Current store context.
     *
     * TxContext: changes pending for the transaction are present in the
     * store (committing the transaction would be a no-op;
     * non-transactional queries would return incorrect results).
     *
     * NonTxContext: changes pending for the transaction are absent from
     * the store (store is ready for non-transactional queries;
     * committing the transaction would require reapplying changes).
     */
    enum Context { TxContext, NonTxContext };

public:
    D(TransactionalStore *ts, Store *store, DirectWriteBehaviour dwb) :
        m_ts(ts),
        m_store(store),
        m_dwb(dwb),
        m_currentTx(NoTransaction),
        m_context(NonTxContext) {
    }
    
    ~D() {
        if (m_currentTx != NoTransaction) {
            std::cerr << "WARNING: TransactionalStore deleted with transaction ongoing" << std::endl;
        }
    }

    Transaction *startTransaction() {
        QMutexLocker locker(&m_mutex);
        DEBUG << "TransactionalStore::startTransaction" << endl;
        if (m_currentTx != NoTransaction) {
            throw RDFTransactionError("ERROR: Attempt to start transaction when another transaction from the same thread is already in train");
        }
        Transaction *tx = new TSTransaction(this);
        m_currentTx = tx;
        return tx;
    }

    void commitTransaction(Transaction *tx) {
        ChangeSet cs;
        {
            QMutexLocker locker(&m_mutex);
            DEBUG << "TransactionalStore::commitTransaction" << endl;
            if (tx != m_currentTx) {
                throw RDFInternalError("Transaction integrity error");
            }
            enterTransactionContext();
            cs = m_currentTx->getChanges();
            // The store is now in transaction context, which means
            // its changes have been committed; resetting m_currentTx
            // now ensures they will remain committed.  Reset
            // m_context as well for symmetry with the initial state
            // in the constructor, though it shouldn't be necessary
            m_currentTx = NoTransaction;
            m_context = NonTxContext;
        }
        DEBUG << "TransactionalStore::commitTransaction: committed " << cs.size() << " change(s)" << endl;
        m_ts->transactionCommitted(cs);
        m_ts->transactionCommitted();
        DEBUG << "TransactionalStore::commitTransaction complete" << endl;
    }

    void rollbackTransaction(Transaction *tx) {
        QMutexLocker locker(&m_mutex);
        DEBUG << "TransactionalStore::rollbackTransaction" << endl;
        if (tx != m_currentTx) {
            throw RDFInternalError("Transaction integrity error");
        }
        leaveTransactionContext();
        // The store is now in non-transaction context, which means
        // the transaction's changes are uncommitted
        m_currentTx = NoTransaction;
        DEBUG << "TransactionalStore::rollbackTransaction complete" << endl;
    }

    class Operation
    {
    public:
        Operation(const D *d, const Transaction *tx) :
            m_d(d), m_tx(tx) {
            m_d->startOperation(m_tx);
        }
        ~Operation() {
            m_d->endOperation(m_tx);
        }
    private:
        const D *m_d;
        const Transaction *m_tx;
    };

    class NonTransactionalAccess
    {
    public:
        NonTransactionalAccess(D *d) :
            m_d(d) {
            m_d->startNonTransactionalAccess();
        }
        ~NonTransactionalAccess() {
            m_d->endNonTransactionalAccess();
        }
    private:
        D *m_d;
    };

    bool add(Transaction *tx, Triple t) {
        Operation op(this, tx);
        return m_store->add(t);
    }

    bool remove(Transaction *tx, Triple t) {
        Operation op(this, tx);
        return m_store->remove(t);
    }

    bool contains(const Transaction *tx, Triple t) const {
        Operation op(this, tx);
        return m_store->contains(t);
    }

    Triples match(const Transaction *tx, Triple t) const {
        Operation op(this, tx);
        return m_store->match(t);
    }

    ResultSet query(const Transaction *tx, QString sparql) const {
        Operation op(this, tx);
        return m_store->query(sparql);
    }

    Node complete(const Transaction *tx, Triple t) const {
        Operation op(this, tx);
        return m_store->complete(t);
    }        

    Triple matchOnce(const Transaction *tx, Triple t) const {
        Operation op(this, tx);
        return m_store->matchOnce(t);
    }

    Node queryOnce(const Transaction *tx, QString sparql,
                    QString bindingName) const {
        Operation op(this, tx);
        return m_store->queryOnce(sparql, bindingName);
    }

    Uri getUniqueUri(const Transaction *tx, QString prefix) const {
        Operation op(this, tx);
        return m_store->getUniqueUri(prefix);
    }

    Node addBlankNode(Transaction *tx) const {
        Operation op(this, tx);
        return m_store->addBlankNode();
    }

    Uri expand(QString uri) const {
        return m_store->expand(uri);
    }

    void save(const Transaction *tx, QString filename) const {
        Operation op(this, tx);
        m_store->save(filename);
    }
/*!!!
    void import(Transaction *tx, QUrl url, ImportDuplicatesMode idm, QString format) {
        Operation op(this, tx);
        m_store->import(url, idm, format);
    }
*/
    Features getSupportedFeatures() const {
        return m_store->getSupportedFeatures();
    }

    bool hasWrap() const {
        return m_dwb == AutoTransaction;
    }

    void startNonTransactionalAccess() {
        // This is only called from the containing TransactionalStore
        // when it wants to carry out a non-transactional read access.
        // Hence, it needs to take a lock and hold it until
        // endNonTransactionalAccess is called
        m_mutex.lock();
        DEBUG << "TransactionalStore::startNonTransactionalAccess" << endl;
        if (m_context == NonTxContext) DEBUG << "(note: already in non-tx context)" << endl;
        leaveTransactionContext();
        // return with mutex held
    }

    void endNonTransactionalAccess() {
        // We can remain in NonTxContext, since every transactional
        // access checks this via enterTransactionContext before doing
        // any work; this way is quicker if we may have multiple
        // non-transactional reads happening together.
        DEBUG << "TransactionalStore::endNonTransactionalAccess" << endl;
        m_mutex.unlock();
    }

    Store *getStore() { return m_store; }
    const Store *getStore() const { return m_store; }
    
private:
    // Most things are mutable here because the TransactionalStore
    // manipulates the Store extensively when entering and leaving
    // transaction context, which can happen on any supposedly
    // read-only access
    TransactionalStore *m_ts;
    mutable Store *m_store;
    DirectWriteBehaviour m_dwb;
    mutable QMutex m_mutex;
    const Transaction *m_currentTx;
    mutable Context m_context;

    void startOperation(const Transaction *tx) const {
        // This will succeed immediately if the mutex is already held
        // by this thread from a startTransaction call (because it is
        // a recursive mutex).  If another thread is performing a
        // transaction, then we have to block until the whole
        // transaction is complete
        m_mutex.lock();
        if (tx != m_currentTx) {
            throw RDFInternalError("Transaction integrity error");
        }
        enterTransactionContext();
    }

    void endOperation(const Transaction *tx) const {
        if (tx != m_currentTx) {
            throw RDFInternalError("Transaction integrity error");
        }            
        m_mutex.unlock();
    }

    void enterTransactionContext() const {
        // This is always called from within this class, with the
        // mutex held already
        if (m_context == TxContext) {
            return;
        }
        if (m_currentTx == NoTransaction) {
            return;
        }
        ChangeSet cs = m_currentTx->getChanges();
        if (!cs.empty()) {
            DEBUG << "TransactionalStore::enterTransactionContext: replaying" << endl;
            try {
                m_store->change(cs);
            } catch (RDFException &e) {
                throw RDFTransactionError(QString("Failed to enter transaction context.  Has the store been modified non-transactionally while a transaction was in progress?  Original error is: %1").arg(e.what()));
            }
        }
        m_context = TxContext;
    }

    void leaveTransactionContext() const {
        // This is always called from within this class, with the
        // mutex held already
        if (m_context == NonTxContext) {
            return;
        }
        if (m_currentTx == NoTransaction) {
            m_context = NonTxContext;
            return;
        }
        // N.B. This is the reason for Transaction::getChanges()
        // returning the change set prior to rollback, rather than
        // e.g. an empty change set, if called after a rollback
        ChangeSet cs = m_currentTx->getChanges();
        if (!cs.empty()) {
            try {
                m_store->revert(cs);
            } catch (RDFException &e) {
                throw RDFTransactionError(QString("Failed to leave transaction context.  Has the store been modified non-transactionally while a transaction was in progress?  Original error is: %1").arg(e.what()));
            }
        }
        m_context = NonTxContext;
    }        
};

class TransactionalStore::TSTransaction::D
{
public:
    D(TransactionalStore::TSTransaction *tx, TransactionalStore::D *td) :
        m_tx(tx), m_td(td), m_committed(false), m_abandoned(false) {
    }
    ~D() {
        if (!m_committed && !m_abandoned && !m_tx->getChanges().empty()) {
            // Although it's not a good idea for any code to try to
            // catch this exception and continue (better just to fix
            // the code!), we should at least make it possible -- so
            // we need to either commit or rollback, or else the next
            // transaction will stall
            m_td->rollbackTransaction(m_tx);
            throw RDFTransactionError(QString("Transaction deleted without having been committed or rolled back"));
        }
    }

    void abandon() const {
        if (m_abandoned || m_committed) return;
        DEBUG << "TransactionalStore::TSTransaction::abandon: Auto-rollback triggered by exception" << endl;
        m_td->rollbackTransaction(m_tx);
        m_abandoned = true;
    }
    
    void check() const {
        if (m_abandoned) {
            throw RDFTransactionError("Transaction used after being rolled back");
        }
        if (m_committed) {
            throw RDFTransactionError("Transaction used afted being committed");
        }
    }

    bool add(Triple t) {
        check();
        try {
            if (m_td->add(m_tx, t)) {
                m_changes.push_back(Change(AddTriple, t));
                return true;
            } else {
                return false;
            }
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    bool remove(Triple t) {
        check();
        try {
            // If some nodes are null, we need to remove all matching
            // triples -- we need to do that here instead of relying
            // on the underlying store (which does the same thing)
            // because we can't push the incomplete statement onto the
            // change set
            Triples tt;
            bool wild = false;
            if (t.a.type == Node::Nothing || 
                t.b.type == Node::Nothing ||
                t.c.type == Node::Nothing) {
                tt = m_td->match(m_tx, t);
                wild = true;
            } else {
                tt.push_back(t);
            }
            bool found = false;
            for (int i = 0; i < tt.size(); ++i) {
                if (m_td->remove(m_tx, tt[i])) {
                    m_changes.push_back(Change(RemoveTriple, tt[i]));
                    found = true;
                } else if (wild) {
                    throw RDFInternalError("Failed to remove matched statement in remove() with wildcards");
                }
            }
            return found;
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    void change(ChangeSet cs) {
        // this is all atomic anyway (as it's part of the
        // transaction), so unlike BasicStore we don't need a lock
        for (int i = 0; i < cs.size(); ++i) {
            ChangeType type = cs[i].first;
            Triple triple = cs[i].second;
            switch (type) {
            case AddTriple:
                if (!add(triple)) {
                    throw RDFException("Change add failed: triple is already in store", triple);
                }
                break;
            case RemoveTriple:
                if (!remove(cs[i].second)) {
                    throw RDFException("Change remove failed: triple is not in store", triple);
                }
                break;
            }
        }
    }

    void revert(ChangeSet cs) {
        // this is all atomic anyway (as it's part of the
        // transaction), so unlike BasicStore we don't need a lock
        for (int i = cs.size()-1; i >= 0; --i) {
            ChangeType type = cs[i].first;
            Triple triple = cs[i].second;
            switch (type) {
            case AddTriple:
                if (!remove(triple)) {
                    throw RDFException("Revert of add failed: triple is not in store", triple);
                }
                break;
            case RemoveTriple:
                if (!add(triple)) {
                    throw RDFException("Revert of remove failed: triple is already in store", triple);
                }
                break;
            }
        }
    }        

    bool contains(Triple t) const {
        check();
        try {
            return m_td->contains(m_tx, t);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Triples match(Triple t) const {
        check();
        try {
            return m_td->match(m_tx, t);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    ResultSet query(QString sparql) const {
        check();
        try {
            return m_td->query(m_tx, sparql);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Node complete(Triple t) const {
        check();
        try {
            return m_td->complete(m_tx, t);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Triple matchOnce(Triple t) const {
        check();
        try {
            return m_td->matchOnce(m_tx, t);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Node queryOnce(QString sparql, QString bindingName) const {
        check();
        try {
            return m_td->queryOnce(m_tx, sparql, bindingName);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Uri getUniqueUri(QString prefix) const {
        check();
        try {
            return m_td->getUniqueUri(m_tx, prefix);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Node addBlankNode() {
        check();
        try {
            return m_td->addBlankNode(m_tx);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    Uri expand(QString uri) const {
        return m_td->expand(uri);
    }

    void save(QString filename) const {
        check();
        try {
            return m_td->save(m_tx, filename);
        } catch (RDFException &) {
            abandon();
            throw;
        }
    }

    void import(QUrl url, ImportDuplicatesMode idm, QString format) {
        check();
        BasicStore *bs = 0;
        try {
            bs = BasicStore::load(url, format);
            Triples ts = bs->match(Triple());
            foreach (Triple t, ts) {
                bool added = m_td->add(m_tx, t);
                if (added) {
                    m_changes.push_back(Change(AddTriple, t));
                } else if (idm == ImportFailOnDuplicates) {
                    throw RDFDuplicateImportException("Duplicate statement encountered on import in ImportFailOnDuplicates mode");
                }
            }
            delete bs;
            bs = 0;
        } catch (RDFException &) {
            delete bs;
            abandon();
            throw;
        }
    }

    Features getSupportedFeatures() const {
        return m_td->getSupportedFeatures();
    }

    void commit() {
        check();
        DEBUG << "TransactionalStore::TSTransaction::commit: Committing" << endl;
        m_td->commitTransaction(m_tx);
        m_committed = true;
    }

    void rollback() {
        check();
        DEBUG << "TransactionalStore::TSTransaction::rollback: Abandoning" << endl;
        m_td->rollbackTransaction(m_tx);
        m_abandoned = true;
    }

    ChangeSet getCommittedChanges() const {
        if (m_committed) return m_changes;
        else return ChangeSet();
    }

    ChangeSet getChanges() const {
        return m_changes;
    }
        
private:
    TransactionalStore::TSTransaction *m_tx;
    TransactionalStore::D *m_td;
    ChangeSet m_changes;
    mutable bool m_committed;
    mutable bool m_abandoned;
};

TransactionalStore::TransactionalStore(Store *store, DirectWriteBehaviour dwb) :
    m_d(new D(this, store, dwb))
{
}

TransactionalStore::~TransactionalStore()
{
    delete m_d;
}

Transaction *
TransactionalStore::startTransaction()
{
    return m_d->startTransaction();
}

void
TransactionalStore::save(QString filename) const
{
    D::NonTransactionalAccess ntxa(m_d);
    m_d->getStore()->save(filename);
}

void
TransactionalStore::import(QUrl url, ImportDuplicatesMode idm, QString format)
{
    if (!m_d->hasWrap()) {
        throw RDFException("TransactionalStore::import() called without Transaction");
    }
    auto_ptr<Transaction> tx(startTransaction());
    return tx->import(url, idm, format);
}

TransactionalStore::Features
TransactionalStore::getSupportedFeatures() const
{
    return m_d->getStore()->getSupportedFeatures();
}

bool
TransactionalStore::add(Triple t)
{
    if (!m_d->hasWrap()) {
        throw RDFException("TransactionalStore::add() called without Transaction");
    }
    // auto_ptr here is very useful to ensure destruction on exceptions
    auto_ptr<Transaction> tx(startTransaction());
    return tx->add(t);
}

bool
TransactionalStore::remove(Triple t)
{
    if (!m_d->hasWrap()) {
        throw RDFException("TransactionalStore::remove() called without Transaction");
    }
    auto_ptr<Transaction> tx(startTransaction());
    return tx->remove(t);
}

void
TransactionalStore::change(ChangeSet cs)
{
    if (!m_d->hasWrap()) {
        throw RDFException("TransactionalStore::change() called without Transaction");
    }
    auto_ptr<Transaction> tx(startTransaction());
    tx->change(cs);
}

void
TransactionalStore::revert(ChangeSet cs)
{
    if (!m_d->hasWrap()) {
        throw RDFException("TransactionalStore::revert() called without Transaction");
    }
    auto_ptr<Transaction> tx(startTransaction());
    tx->revert(cs);
}

bool
TransactionalStore::contains(Triple t) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->contains(t);
}
    
Triples
TransactionalStore::match(Triple t) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->match(t);
}

ResultSet
TransactionalStore::query(QString s) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->query(s);
}

Node
TransactionalStore::complete(Triple t) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->complete(t);
}

Triple
TransactionalStore::matchOnce(Triple t) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->matchOnce(t);
}

Node
TransactionalStore::queryOnce(QString s, QString b) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->queryOnce(s, b);
}

Uri
TransactionalStore::getUniqueUri(QString prefix) const
{
    D::NonTransactionalAccess ntxa(m_d);
    return m_d->getStore()->getUniqueUri(prefix);
}

Node
TransactionalStore::addBlankNode()
{
    if (!m_d->hasWrap()) {
        throw RDFException("TransactionalStore::addBlankNode() called without Transaction");
    }
    auto_ptr<Transaction> tx(startTransaction());
    return tx->addBlankNode();
}

Uri
TransactionalStore::expand(QString uri) const
{
    return m_d->expand(uri);
}

TransactionalStore::TSTransaction::TSTransaction(TransactionalStore::D *td) :
    m_d(new D(this, td))
{
}

TransactionalStore::TSTransaction::~TSTransaction()
{
    delete m_d;
}

bool
TransactionalStore::TSTransaction::add(Triple t)
{
    return m_d->add(t);
}

bool
TransactionalStore::TSTransaction::remove(Triple t)
{
    return m_d->remove(t);
}

void
TransactionalStore::TSTransaction::change(ChangeSet cs)
{
    m_d->change(cs);
}

void
TransactionalStore::TSTransaction::revert(ChangeSet cs)
{
    m_d->revert(cs);
}

bool
TransactionalStore::TSTransaction::contains(Triple t) const
{
    return m_d->contains(t);
}

Triples
TransactionalStore::TSTransaction::match(Triple t) const
{
    return m_d->match(t);
}

ResultSet
TransactionalStore::TSTransaction::query(QString sparql) const
{
    return m_d->query(sparql);
}

Node
TransactionalStore::TSTransaction::complete(Triple t) const
{
    return m_d->complete(t);
}

Triple
TransactionalStore::TSTransaction::matchOnce(Triple t) const
{
    return m_d->matchOnce(t);
}

Node
TransactionalStore::TSTransaction::queryOnce(QString sparql,
                                                  QString bindingName) const
{
    return m_d->queryOnce(sparql, bindingName);
}

Uri
TransactionalStore::TSTransaction::getUniqueUri(QString prefix) const
{
    return m_d->getUniqueUri(prefix);
}

Node
TransactionalStore::TSTransaction::addBlankNode()
{
    return m_d->addBlankNode();
}

Uri
TransactionalStore::TSTransaction::expand(QString uri) const
{
    return m_d->expand(uri);
}

void
TransactionalStore::TSTransaction::save(QString filename) const
{
    return m_d->save(filename);
}

void
TransactionalStore::TSTransaction::import(QUrl url, ImportDuplicatesMode idm, QString format)
{
    m_d->import(url, idm, format);
}

TransactionalStore::TSTransaction::Features
TransactionalStore::TSTransaction::getSupportedFeatures() const
{
    return m_d->getSupportedFeatures();
}

void
TransactionalStore::TSTransaction::commit()
{
    m_d->commit();
}

void
TransactionalStore::TSTransaction::rollback()
{
    m_d->rollback();
}

ChangeSet
TransactionalStore::TSTransaction::getCommittedChanges() const
{
    return m_d->getCommittedChanges();
}

ChangeSet
TransactionalStore::TSTransaction::getChanges() const
{
    return m_d->getChanges();
}

}

