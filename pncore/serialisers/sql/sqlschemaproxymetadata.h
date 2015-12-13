#ifndef SQLSCHEMAPROXYMETADATA_H
#define SQLSCHEMAPROXYMETADATA_H

#include <QSqlDatabase>
#include "sqlschemaproxybase.h"
#include "structure/metadatastructure.h"

class SQLSchemaProxyMetadata : public SQLSchemaProxyBase
{
public:
    static void createSchema(MetadataStructure *structure, QSqlDatabase &db);
    static bool createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute, QSqlDatabase &db);
    static bool renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID, QSqlDatabase &db);
    static bool deleteMetadataAttribute(CorpusObject::Type type, QString attributeID, QSqlDatabase &db);

private:
    SQLSchemaProxyMetadata();
};

#endif // SQLSCHEMAPROXYMETADATA_H
