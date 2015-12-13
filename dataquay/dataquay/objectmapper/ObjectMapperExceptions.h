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

#ifndef _DATAQUAY_OBJECT_MAPPER_EXCEPTIONS_H_
#define _DATAQUAY_OBJECT_MAPPER_EXCEPTIONS_H_

#include <exception>
#include <QString>

namespace Dataquay
{

/**
 * \class UnknownTypeException ObjectMapperExceptions.h <dataquay/objectmapper/ObjectMapperExceptions.h>
 *
 * UnknownTypeException is an exception thrown by ObjectMapper classes
 * when asked to load an object from an RDF entity whose type cannot
 * be mapped to a C++ class type using the current TypeMapping.
 */
class UnknownTypeException : virtual public std::exception {
public:
    UnknownTypeException(QString type) throw() : m_type(type) { }
    virtual ~UnknownTypeException() throw() { }
    virtual const char *what() const throw() {
	return QString("Unknown type: %1").arg(m_type).toLocal8Bit().data();
    }
protected:
    QString m_type;
};

/**
 * \class ConstructionFailedException ObjectMapperExceptions.h <dataquay/objectmapper/ObjectMapperExceptions.h>
 *
 * ConstructionFailedException is an exception that may be thrown by
 * ObjectMapper classes if a class mapped from an RDF entity type
 * could not be constructed.
 */
class ConstructionFailedException : virtual public std::exception {
public:
    ConstructionFailedException(QString type) throw() : m_type(type) { }
    virtual ~ConstructionFailedException() throw() { }
    virtual const char *what() const throw() {
	return QString("Failed to construct type: %1")
	    .arg(m_type).toLocal8Bit().data();
    }
protected:
    QString m_type;
};

/**
 * \class NoUriException ObjectMapperExceptions.h <dataquay/objectmapper/ObjectMapperExceptions.h>
 *
 * NoUriException is an exception thrown by ObjectMapper classes when
 * asked to store a class that is expected to have been assigned a URI
 * already, if its URI property is absent.
 */
class NoUriException : virtual public std::exception {
public:
    NoUriException(QString oname, QString cname) throw() : m_oname(oname), m_cname(cname) { }
    virtual ~NoUriException() throw() { }
    virtual const char *what() const throw() {
        return QString("Object of name \"%1\" and class %2 lacks URI")
            .arg(m_oname).arg(m_cname).toLocal8Bit().data();
    }
protected:
    QString m_oname;
    QString m_cname;
};

}

#endif
