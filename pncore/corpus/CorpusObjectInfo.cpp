#include "CorpusObjectInfo.h"

namespace Praaline {
namespace Core {

CorpusObjectInfo::CorpusObjectInfo(
        CorpusObject::Type type, const QString &ID, const QString &parentID, const QString &name, const QString &description) :
    m_type(type), m_ID(ID), m_parentID(parentID), m_name(name), m_description(description)
{
}

CorpusObject::Type CorpusObjectInfo::type() const
{
    return m_type;
}

QString CorpusObjectInfo::ID() const
{
    return m_ID;
}

QString CorpusObjectInfo::parentID() const
{
    return m_parentID;
}

QString CorpusObjectInfo::name() const
{
    return m_name;
}

QString CorpusObjectInfo::description() const
{
    return m_description;
}

QString CorpusObjectInfo::createdBy() const
{
    return m_createdBy;
}

QDateTime CorpusObjectInfo::createdTimestamp() const
{
    return m_createdTimestamp;
}

QString CorpusObjectInfo::lastUpdatedBy() const
{
    return m_lastUpdatedBy;
}

QDateTime CorpusObjectInfo::lastUpdatedTimestamp() const
{
    return m_lastUpdatedTimestamp;
}

void CorpusObjectInfo::setCreated(const QString &createdBy, const QDateTime &createdTimestamp)
{
    m_createdBy = createdBy;
    m_createdTimestamp = createdTimestamp;
}

void CorpusObjectInfo::setUpdated(const QString &updatedBy, const QDateTime &updatedTimestamp)
{
    m_lastUpdatedBy = updatedBy;
    m_lastUpdatedTimestamp = updatedTimestamp;
}

} // namespace Core
} // namespace Praaline

