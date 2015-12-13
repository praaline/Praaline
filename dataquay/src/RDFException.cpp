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

#include "RDFException.h"

#include "Uri.h"
#include "Node.h"
#include "Triple.h"
#include "Debug.h"

#include <QTextStream>

#include <cstring>
#include <cstdlib>

namespace Dataquay
{

RDFException::RDFException(QString message) throw() :
    m_message(0)
{
    setMessage(message);
}

RDFException::RDFException(QString message, QString data) throw() :
    m_message(0)
{
    setMessage(QString("%1 [with string \"%2\"]").arg(message).arg(data));
}

RDFException::RDFException(QString message, const Uri &uri) throw() :
    m_message(0)
{
    setMessage(QString("%1 [with URI <%2>]").arg(message).arg(uri.toString()));
}

RDFException::RDFException(QString message, const Node &node) throw() :
    m_message(0)
{
    QString s;
    QTextStream ts(&s);
    ts << node;
    setMessage(QString("%1 [with node <%2>]").arg(message).arg(s));
}

RDFException::RDFException(QString message, const Triple &triple) throw() :
    m_message(0)
{
    QString s;
    QTextStream ts(&s);
    ts << triple;
    setMessage(QString("%1 [with triple <%2>]").arg(message).arg(s));
}

RDFException::RDFException(const RDFException &e) throw()
{
    m_message = strdup(e.m_message);
}

RDFException &
RDFException::operator=(const RDFException &e) throw()
{
    if (&e == this) return *this;
    free(m_message);
    m_message = strdup(e.m_message);
    return *this;
}

RDFException::~RDFException() throw() {
    free(m_message);
}

void 
RDFException::setMessage(QString m)
{
    if (m_message) free(m_message);
    m_message = strdup(m.toLocal8Bit().data());
    DEBUG << "RDFException: " << m_message << endl;
}

}
