#include <QObject>
#include "StructureAttributeBase.h"
#include "MetadataStructureSection.h"
#include "MetadataStructureAttribute.h"

namespace Praaline {
namespace Core {

MetadataStructureAttribute::MetadataStructureAttribute(QObject *parent) :
    StructureAttributeBase(parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const QString &ID, QObject *parent) :
    StructureAttributeBase(ID, parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const QString &ID, const QString &name, const QString &description,
                                                       const DataType &datatype, int order, QObject *parent) :
    StructureAttributeBase(ID, name, description, datatype, order, parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const MetadataStructureAttribute *other, QObject *parent) :
    StructureAttributeBase(other, parent)
{
}

QString MetadataStructureAttribute::sectionID() const
{
    MetadataStructureSection *section = qobject_cast<MetadataStructureSection *>(this->parent());
    if (section) return section->ID();
    return QString();
}

} // namespace Core
} // namespace Praaline
