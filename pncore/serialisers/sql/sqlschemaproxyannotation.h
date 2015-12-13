#ifndef SQLSCHEMAPROXYANNOTATION_H
#define SQLSCHEMAPROXYANNOTATION_H

#include <QSqlDatabase>
#include "sqlschemaproxybase.h"
#include "structure/annotationstructure.h"

class SQLSchemaProxyAnnotation : public SQLSchemaProxyBase
{
public:
    static bool createSchema(AnnotationStructure *structure, QSqlDatabase &db);
    static bool createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel, QSqlDatabase &db);
    static bool renameAnnotationLevel(QString levelID, QString newLevelID, QSqlDatabase &db);
    static bool deleteAnnotationLevel(QString levelID, QSqlDatabase &db);
    static bool createAnnotationAttribute(QString levelID, QPointer<AnnotationStructureAttribute> newAttribute, QSqlDatabase &db);
    static bool renameAnnotationAttribute(QString levelID, QString attributeID, QString newAttributeID, QSqlDatabase &db);
    static bool deleteAnnotationAttribute(QString levelID, QString attributeID, QSqlDatabase &db);
    static bool retypeAnnotationAttribute(QString levelID, QString attributeID, QString newDatatype, int newDatalength, QSqlDatabase &db);

private:
    SQLSchemaProxyAnnotation();
};

#endif // SQLSCHEMAPROXYANNOTATION_H
