#ifndef SQLSERIALISERMETADATASTRUCTURE_H
#define SQLSERIALISERMETADATASTRUCTURE_H

#include "structure/MetadataStructure.h"
#include <QSqlDatabase>

namespace Praaline {
namespace Core {

class SQLSerialiserMetadataStructure
{
public:
    static void initialiseMetadataStructureTables(QSqlDatabase &db);
    static bool write(MetadataStructure *structure, QSqlDatabase &db);
    static bool writePartial(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db);
    static bool read(MetadataStructure *structure, QSqlDatabase &db);
    static bool readPartial(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db);
private:
    SQLSerialiserMetadataStructure();
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERMETADATASTRUCTURE_H
