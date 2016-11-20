#include <QObject>
#include "StructureBase.h"
#include "AnnotationStructureAttribute.h"

namespace Praaline {
namespace Core {

AnnotationStructureAttribute::AnnotationStructureAttribute(QObject *parent) :
    StructureBase(parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const QString &ID, const QString &name, const QString &description,
                                                           const DataType &datatype, int order, bool indexed, const QString &nameValueList,
                                                           QObject *parent) :
    StructureBase(ID, name, description, datatype, order, indexed, nameValueList, parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const AnnotationStructureAttribute *other, QObject *parent) :
    StructureBase(other, parent)
{
    m_statLevelOfMeasurement = other->statLevelOfMeasurement();
}

} // namespace Core
} // namespace Praaline
