#include <QObject>
#include "structurebase.h"
#include "annotationstructureattribute.h"

AnnotationStructureAttribute::AnnotationStructureAttribute(QObject *parent) :
    StructureBase(parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const QString &ID, const QString &name, const QString &description,
                                                           const QString &datatype, int datalength, bool indexed, const QString &nameValueList,
                                                           QObject *parent) :
    StructureBase(ID, name, description, datatype, datalength, indexed, nameValueList, parent)
{
}

AnnotationStructureAttribute::AnnotationStructureAttribute(const AnnotationStructureAttribute *other, QObject *parent) :
    StructureBase(other, parent)
{
    m_statLevelOfMeasurement = other->statLevelOfMeasurement();
}

