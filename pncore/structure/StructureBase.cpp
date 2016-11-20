#include "StructureBase.h"

namespace Praaline {
namespace Core {

StructureBase::StructureBase(QObject *parent) :
    QObject(parent), m_datatype(DataType::Invalid), m_order(0), m_indexed(false)
{
}

StructureBase::StructureBase(const QString &ID, QObject *parent) :
    QObject(parent), m_ID(ID), m_datatype(DataType::Invalid), m_order(0), m_indexed(false)
{
    m_name = ID;
}

StructureBase::StructureBase(const QString &ID, const QString &name, const QString &description,
                             const DataType &datatype, int order, bool indexed, const QString &nameValueList,
                             QObject *parent) :
    QObject(parent), m_ID(ID), m_name(name), m_description(description), m_datatype(datatype),
    m_order(order), m_indexed(indexed), m_nameValueList(nameValueList)
{
    if (name.isEmpty()) m_name = ID;
}

StructureBase::StructureBase(const StructureBase *other, QObject *parent) :
    QObject(parent), m_datatype(DataType::invalid())
{
    if (!other) return;
    m_ID = other->ID();
    m_name = other->name();
    m_description = other->description();
    m_datatype = other->datatype();
    m_order = other->order();
    m_indexed = other->indexed();
    m_nameValueList = other->nameValueList();
}

} // namespace Core
} // namespace Praaline
