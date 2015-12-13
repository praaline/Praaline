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

#ifndef _DATAQUAY_EXCEPTION_H_
#define _DATAQUAY_EXCEPTION_H_

#include <QString>
#include <exception>

namespace Dataquay
{

class Uri;
class Node;
class Triple;

/**
 * \class RDFException RDFException.h <dataquay/RDFException.h>
 *
 * RDFException is an exception that results from incorrect usage of
 * the RDF store interface or unsuitable data provided to a function.
 * For example, this exception would be thrown in response to trying
 * to add an incomplete triple to the store.
 */
class RDFException : virtual public std::exception
{
public:
    RDFException(QString message) throw();
    RDFException(QString message, QString data) throw();
    RDFException(QString message, const Uri &uri) throw();
    RDFException(QString message, const Node &node) throw();
    RDFException(QString message, const Triple &triple) throw();
    RDFException(const RDFException &e) throw();
    RDFException &operator=(const RDFException &e) throw();
    virtual ~RDFException() throw();
    virtual const char *what() const throw() { return m_message; }

protected:
    char *m_message;
    void setMessage(QString m);
};

/**
 * \class RDFIncompleteURI RDFException.h <dataquay/RDFException.h>
 * 
 * RDFIncompleteURI is thrown when an attempt is made to construct a
 * Uri from an incomplete URI string, such as a relative URI or a
 * string with no scheme. Relative URIs should be represented as plain
 * strings, and expanded by the store using its base URI into Uri
 * objects.
 */
class RDFIncompleteURI : virtual public RDFException
{
public:
    RDFIncompleteURI(QString message, QString data) throw() :
        RDFException(message, data) { }
    virtual ~RDFIncompleteURI() throw() { }
};

/**
 * \class RDFInternalError RDFException.h <dataquay/RDFException.h>
 *
 * RDFInternalError is an exception that results from an internal
 * error in the RDF store.
 */
class RDFInternalError : virtual public RDFException
{
public:
    RDFInternalError(QString message, QString data = "") throw() :
        RDFException(message, data) { }
    RDFInternalError(QString message, const Uri &data) throw() :
        RDFException(message, data) { }
    virtual ~RDFInternalError() throw() { }
};

/**
 * \class RDFUnsupportedError RDFException.h <dataquay/RDFException.h>
 *
 * RDFUnsupportedError is an exception that results from an attempt to
 * use a feature that is not supported or not configured in the
 * current build.
 */
class RDFUnsupportedError : virtual public RDFException
{
public:
    RDFUnsupportedError(QString message, QString data = "") throw() :
        RDFException(message, data) { }
    RDFUnsupportedError(QString message, const Uri &data) throw() :
        RDFException(message, data) { }
    virtual ~RDFUnsupportedError() throw() { }
};

/**
 * \class RDFTransactionError RDFException.h <dataquay/RDFException.h>
 *
 * RDFTransactionError is an exception that results from incorrect use
 * of a Transaction, for example using a Transaction object after it
 * has been committed.
 */
class RDFTransactionError : virtual public RDFException
{
public:
    RDFTransactionError(QString message, QString data = "") throw() :
        RDFException(message, data) { }
    virtual ~RDFTransactionError() throw() { }
};

/**
 * \class RDFDuplicateImportException RDFException.h <dataquay/RDFException.h>
 *
 * RDFDuplicateImportException is an exception that results from an
 * import into a store from an RDF document in ImportFailOnDuplicates
 * mode, where the document contains a triple that already exists in
 * the store.
 */
class RDFDuplicateImportException : virtual public RDFException
{
public:
    RDFDuplicateImportException(QString message, QString data = "") throw() :
        RDFException(message, data) { }
    RDFDuplicateImportException(QString message, const Triple &t) throw() :
        RDFException(message, t) { }
    virtual ~RDFDuplicateImportException() throw() { }
};

}

#endif

