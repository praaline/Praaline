#include <QObject>
#include "StructureAttributeBase.h"
#include "AnnotationStructureAttribute.h"

namespace Praaline {
namespace Core {

AnnotationStructureAttribute::AnnotationStructureAttribute(QObject *parent) :
    StructureAttributeBase(parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const QString &ID, const QString &name, const QString &description,
                                                           const DataType &datatype, int order, bool indexed, const QString &nameValueList,
                                                           QObject *parent) :
    StructureAttributeBase(ID, name, description, datatype, order, indexed, nameValueList, parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const AnnotationStructureAttribute *other, QObject *parent) :
    StructureAttributeBase(other, parent)
{
    m_statLevelOfMeasurement = other->statLevelOfMeasurement();
}

} // namespace Core
} // namespace Praaline
