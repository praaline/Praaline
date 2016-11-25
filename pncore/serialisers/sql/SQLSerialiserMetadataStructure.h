#ifndef SQLSERIALISERMETADATASTRUCTURE_H
#define SQLSERIALISERMETADATASTRUCTURE_H

#include <QSqlDatabase>
#include "structure/MetadataStructure.h"
#include "SQLSchemaProxyBase.h"

namespace Praaline {
namespace Core {

class SQLSerialiserMetadataStructure : public SQLSchemaProxyBase
{
public:
    static bool initialiseMetadataStructureTables(QSqlDatabase &db);
    static bool createMetadataSchema(QPointer<MetadataStructure> structure, QSqlDatabase &db);

    static bool loadMetadataStructure(QPointer<MetadataStructure> structure, QSqlDatabase &db);

    static bool createMetadataSection(QPointer<MetadataStructureSection> newSection, QSqlDatabase &db);
    static bool updateMetadataSection(QPointer<MetadataStructureSection> updatedSection, QSqlDatabase &db);
    static bool deleteMetadataSection(const QString &sectionID, QSqlDatabase &db);

    static bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute, QSqlDatabase &db);
    static bool updateMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> updatedAttribute, QSqlDatabase &db);
    static bool renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID, QSqlDatabase &db);
    static bool retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID,
                                        const DataType &oldDataType, const DataType &newDataType, QSqlDatabase &db);
    static bool deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID, QSqlDatabase &db);

private:
    SQLSerialiserMetadataStructure() {}
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERMETADATASTRUCTURE_H
