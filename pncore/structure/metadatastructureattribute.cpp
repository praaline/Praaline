#include <QObject>
#include "structurebase.h"
#include "metadatastructureattribute.h"

MetadataStructureAttribute::MetadataStructureAttribute(QObject *parent) :
    StructureBase(parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const QString &ID, QObject *parent) :
    StructureBase(ID, parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const QString &ID, const QString &name, const QString &description,
                                                       const QString &datatype, int datalength, QObject *parent) :
    StructureBase(ID, name, description, datatype, datalength, parent)
{
}

MetadataStructureAttribute::MetadataStructureAttribute(const MetadataStructureAttribute *other, QObject *parent) :
    StructureBase(other, parent)
{
}
