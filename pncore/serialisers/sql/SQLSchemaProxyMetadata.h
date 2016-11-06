#ifndef SQLSCHEMAPROXYMETADATA_H
#define SQLSCHEMAPROXYMETADATA_H

#include <QSqlDatabase>
#include "SQLSchemaProxyBase.h"
#include "structure/MetadataStructure.h"

namespace Praaline {
namespace Core {

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

} // namespace Core
} // namespace Praaline

#endif // SQLSCHEMAPROXYMETADATA_H
