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

#ifndef _DATAQUAY_TRANSACTION_H_
#define _DATAQUAY_TRANSACTION_H_

#include "Store.h"

namespace Dataquay
{

/**
 * \class Transaction Transaction.h <dataquay/Transaction.h>
 *
 * Transaction is an abstract interface for classes that read and
 * write an RDF Store within the context of an atomic operation such
 * as an editing command.  The Transaction interface provides the same
 * editing operations as the Store interface; it subclasses from Store
 * and may be interchanged with Store in most contexts.  To obtain a
 * Transaction, use TransactionalStore.
 */
class Transaction : public Store
{
public:
    /**
     * Delete this transaction object.  You must either commit or roll
     * back the transaction before deleting it.
     * 
     * The destructor will throw RDFTransactionError if the
     * transaction has been used but not committed or rolled back.
     * Such a situation indicates a straightforward coding oversight:
     * fix the code, rather than catching the exception.
     */
    virtual ~Transaction() { }
    
    /**
     * Commit this transaction.  Changes made during the transaction
     * will be committed to the store, atomically with respect to
     * other active transactions.
     *
     * You should not attempt to use the Transaction object again
     * (except to call getChanges or to delete it) after this call is
     * made.  Any further call to the transaction's Store interface
     * will throw an RDFTransactionError.
     */
    virtual void commit() = 0;
    
    /**
     * Roll back this transaction.  All changes made during the
     * transaction will be discarded.
     *
     * You should not attempt to use the Transaction object again
     * (except to delete it) after this call is made.  Any further
     * call to the transaction's Store interface will throw an
     * RDFTransactionError.
     */
    virtual void rollback() = 0;

    /**
     * Return the ChangeSet committed in this transaction.  If the
     * transaction has not yet been committed (or has been rolled
     * back), this will be empty.
     *
     * (After a transaction has been committed, you can in principle
     * revert it in its entirety by calling Store::revert() with this
     * change set.)
     */
    virtual ChangeSet getCommittedChanges() const = 0;

    /**
     * Return the ChangeSet for this transaction to date.  This
     * returns all changes provisionally made (but not necessarily
     * committed) during the transaction.
     *
     * If the transaction has been rolled back, this will return the
     * changes that were accumulated prior to the roll back, i.e. the
     * changes that were then rolled back.
     *
     * If the transaction has been committed, this will return the
     * same ChangeSet as getCommittedChanges().
     */
    virtual ChangeSet getChanges() const = 0;

protected:
    Transaction() { }

private:
    Transaction(const Transaction &);
    Transaction &operator=(const Transaction &);
};

extern Transaction *const NoTransaction;

}

#endif
