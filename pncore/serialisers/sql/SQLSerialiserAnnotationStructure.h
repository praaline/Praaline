#ifndef SQLSERIALISERANNOTATIONSTRUCTURE_H
#define SQLSERIALISERANNOTATIONSTRUCTURE_H

#include <QPointer>
#include <QSqlDatabase>
#include "structure/AnnotationStructure.h"
#include "SQLSchemaProxyBase.h"

namespace Praaline {
namespace Core {

class SQLSerialiserAnnotationStructure : public SQLSchemaProxyBase
{
public:
    static bool initialiseAnnotationStructureTables(QSqlDatabase &db);

    static bool loadAnnotationStructure(QPointer<AnnotationStructure> structure, QSqlDatabase &db);
    static bool saveAnnotationStructure(QPointer<AnnotationStructure> structure, QSqlDatabase &db);

    static bool createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel, QSqlDatabase &db);
    static bool updateAnnotationLevel(QPointer<AnnotationStructureLevel> updatedLevel, QSqlDatabase &db);
    static bool renameAnnotationLevel(const QString &levelID, const QString &newLevelID, QSqlDatabase &db);
    static bool deleteAnnotationLevel(const QString &levelID, QSqlDatabase &db);

    static bool createAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> newAttribute, QSqlDatabase &db);
    static bool updateAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> updatedAttribute, QSqlDatabase &db);
    static bool renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID, QSqlDatabase &db);
    static bool retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDataType, QSqlDatabase &db);
    static bool deleteAnnotationAttribute(const QString &levelID, const QString &attributeID, QSqlDatabase &db);

private:
    SQLSerialiserAnnotationStructure() {}
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERANNOTATIONSTRUCTURE_H
