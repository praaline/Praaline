#include "CorpusObjectInfo.h"

namespace Praaline {
namespace Core {

CorpusObjectInfo::CorpusObjectInfo() :
    m_type(CorpusObject::Type_Undefined)
{
    m_isNew = true;
    m_isDirty = true;
}

CorpusObjectInfo::CorpusObjectInfo(CorpusObject::Type type) :
    m_type(type)
{
    m_isNew = true;
    m_isDirty = true;
}

CorpusObjectInfo::CorpusObjectInfo(const CorpusObjectInfo &other) :
    m_type(other.m_type), m_attributes(other.m_attributes)
{
    m_isNew = true;
    m_isDirty = true;
}

} // namespace Core
} // namespace Praaline

