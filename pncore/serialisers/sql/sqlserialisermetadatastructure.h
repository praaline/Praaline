#ifndef SQLSERIALISERMETADATASTRUCTURE_H
#define SQLSERIALISERMETADATASTRUCTURE_H

#include "structure/metadatastructure.h"
#include <QSqlDatabase>

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

#endif // SQLSERIALISERMETADATASTRUCTURE_H
