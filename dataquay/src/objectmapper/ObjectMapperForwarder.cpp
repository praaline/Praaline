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

#include "objectmapper/ObjectMapperForwarder.h"
#include "objectmapper/ObjectMapper.h"

#include <QMetaProperty>
#include "../Debug.h"

namespace Dataquay
{

ObjectMapperForwarder::ObjectMapperForwarder(ObjectMapper *m, QObject *o) :
    QObject(m),
    m_mapper(m),
    m_source(o)
{
    for (int i = 0; i < o->metaObject()->propertyCount(); ++i) {
            
        QMetaProperty property = o->metaObject()->property(i);
            
        if (!property.isStored() ||
            !property.isReadable() ||
            !property.isWritable()) {
            continue;
        }
            
        if (!property.hasNotifySignal()) {
            DEBUG << "ObjectMapperForwarder: No notify signal for property " << property.name() << endl;
            continue;
        }
        
        // Signals can be connected to slots with fewer arguments, so
        // long as the arguments they do have match.  So we connect
        // the property notify signal to our universal
        // property-changed slot, and use the sender() of that to
        // discover which object's property has changed.
        // Unfortunately, we don't then know which property it was
        // that changed, as the identity of the signal that activated
        // the slot is not available to us.  The annoying part of this
        // is that Qt does actually store the identity of the signal
        // in the same structure as used for the sender() object data;
        // it just doesn't (at least as of Qt 4.5) make it available
        // through the public API.

        QString sig = QString("%1%2").arg(QSIGNAL_CODE)
#if (QT_VERSION >= 0x050000)
            .arg(property.notifySignal().methodSignature().data());
#else
            .arg(property.notifySignal().signature());
#endif
        QByteArray ba = sig.toLocal8Bit();
        
        if (!connect(o, ba.data(), this, SLOT(objectModified()))) {
            std::cerr << "ObjectMapperForwarder: Failed to connect notify signal" << std::endl;
        }
    }
    
    connect(o, SIGNAL(destroyed(QObject *)), this, SLOT(objectDestroyed()));
}

void
ObjectMapperForwarder::objectDestroyed()
{
    m_mapper->objectDestroyed(m_source);
}

void
ObjectMapperForwarder::objectModified()
{
    m_mapper->objectModified(m_source);
}

}

