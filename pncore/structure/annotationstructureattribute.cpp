#include <QObject>
#include "structurebase.h"
#include "annotationstructureattribute.h"

AnnotationStructureAttribute::AnnotationStructureAttribute(QObject *parent) :
    StructureBase(parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const QString &ID, QObject *parent) :
    StructureBase(ID, parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const AnnotationStructureAttribute *other, QObject *parent) :
    StructureBase(other, parent)
{
    m_statLevelOfMeasurement = other->statLevelOfMeasurement();
}
