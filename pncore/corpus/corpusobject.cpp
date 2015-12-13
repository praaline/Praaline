#include <QObject>
#include <QVariant>
#include <QDynamicPropertyChangeEvent>
#include "corpusobject.h"

CorpusObject::CorpusObject(QObject *parent) :
    QObject(parent), m_ID(QString()), m_originalID(QString()), m_dirty(true), m_new(true)
{
    setNew(true);
    setDirty(true);
}

CorpusObject::CorpusObject(const QString &ID, QObject *parent) :
    QObject(parent), m_ID(ID), m_originalID(ID), m_dirty(false), m_new(false)
{
    setNew(true);
    setDirty(true);
}

void CorpusObject::setID(const QString &ID)
{
    if (m_ID != ID) {
        QString oldID = m_ID;
        m_ID = ID;
        m_dirty = true;
        emit changedID(oldID, m_ID);
    }
}

void CorpusObject::setCorpusID(const QString &corpusID)
{
    if (m_corpusID != corpusID) {
        m_corpusID = corpusID;
        m_dirty = true;
    }
}

void CorpusObject::setDirty(bool value)
{
    m_dirty = value;
    if (value == false) m_originalID = m_ID;
}

QVariant CorpusObject::property(const QString &name) const
{
    return QObject::property(name.toLatin1().constData());
}

bool CorpusObject::setProperty(const QString &name, const QVariant &value)
{
    setDirty(true);
    return QObject::setProperty(name.toLatin1().constData(), value);
}

bool CorpusObject::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::DynamicPropertyChange) {
        m_dirty = true;
    }
    return QObject::eventFilter(obj, event);
}

void CorpusObject::copyPropertiesFrom(CorpusObject *other)
{
    if (!other) return;
    foreach (const QByteArray &propertyName, other->dynamicPropertyNames())
        setProperty(propertyName.constData(), other->property(propertyName.constData()));
    setDirty(true);
}
