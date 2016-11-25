#ifndef MOCADBSERIALISERMETADATASTRUCTURE_H
#define MOCADBSERIALISERMETADATASTRUCTURE_H

#include <QSqlDatabase>
#include "structure/MetadataStructure.h"

namespace Praaline {
namespace Core {

class MocaDBSerialiserMetadataStructure
{
public:
    static bool initialiseMetadataStructureTables(QSqlDatabase &db);
    static bool loadMetadataStructure(QPointer<MetadataStructure> structure, QSqlDatabase &db);

    static bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute, QSqlDatabase &db);
    static bool updateMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> updatedAttribute, QSqlDatabase &db);
    static bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID, QSqlDatabase &db);
    static bool retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const DataType &newDataType, QSqlDatabase &db);
    static bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID, QSqlDatabase &db);

private:
    MocaDBSerialiserMetadataStructure() {}
};

} // namespace Core
} // namespace Praaline

#endif // MOCADBSERIALISERMETADATASTRUCTURE_H
