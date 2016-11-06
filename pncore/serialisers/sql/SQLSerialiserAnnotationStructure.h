#ifndef SQLSERIALISERANNOTATIONSTRUCTURE_H
#define SQLSERIALISERANNOTATIONSTRUCTURE_H

#include <QSqlDatabase>
#include "structure/AnnotationStructure.h"

namespace Praaline {
namespace Core {

class SQLSerialiserAnnotationStructure
{
public:
    static void initialiseAnnotationStructureTables(QSqlDatabase &db);
    static bool write(AnnotationStructure *structure, QSqlDatabase &db);
    static bool read(AnnotationStructure *structure, QSqlDatabase &db);

private:
    SQLSerialiserAnnotationStructure();
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERANNOTATIONSTRUCTURE_H
