#include <QObject>
#include "StructureBase.h"
#include "MetadataStructureAttribute.h"

namespace Praaline {
namespace Core {

MetadataStructureAttribute::MetadataStructureAttribute(QObject *parent) :
    StructureBase(parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const QString &ID, QObject *parent) :
    StructureBase(ID, parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const QString &ID, const QString &name, const QString &description,
                                                       const DataType &datatype, QObject *parent) :
    StructureBase(ID, name, description, datatype, parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const MetadataStructureAttribute *other, QObject *parent) :
    StructureBase(other, parent)
{
}

} // namespace Core
} // namespace Praaline
