#ifndef SQLSERIALISERANNOTATIONSTRUCTURE_H
#define SQLSERIALISERANNOTATIONSTRUCTURE_H

#include <QSqlDatabase>
#include "structure/annotationstructure.h"

class SQLSerialiserAnnotationStructure
{
public:
    static void initialiseAnnotationStructureTables(QSqlDatabase &db);
    static bool write(AnnotationStructure *structure, QSqlDatabase &db);
    static bool read(AnnotationStructure *structure, QSqlDatabase &db);

private:
    SQLSerialiserAnnotationStructure();
};

#endif // SQLSERIALISERANNOTATIONSTRUCTURE_H
