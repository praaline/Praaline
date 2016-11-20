#include "CorpusObjectInfo.h"

namespace Praaline {
namespace Core {

CorpusObjectInfo::CorpusObjectInfo(const QString &ID, const QString &corpusID, const QString &name, CorpusObject::Type type) :
    m_ID(ID), m_corpusID(corpusID), m_name(name), m_type(type)
{
}

QString CorpusObjectInfo::ID() const
{
    return m_ID;
}

QString CorpusObjectInfo::corpusID() const
{
    return m_corpusID;
}

QString CorpusObjectInfo::name() const
{
    return m_name;
}

CorpusObject::Type CorpusObjectInfo::type() const
{
    return m_type;
}

} // namespace Core
} // namespace Praaline

