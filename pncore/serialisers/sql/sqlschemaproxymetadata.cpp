#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "sqlschemaproxymetadata.h"

SQLSchemaProxyMetadata::SQLSchemaProxyMetadata()
{
}

bool createNewSchema(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db)
{
    QString tableName, sqlBase, sqlKeys, sqlMetadata;
    if (what == CorpusObject::Type_Communication) {
        tableName = "communication";
        sqlBase = "CREATE TABLE communication ( "
                "   communicationID varchar(64) NOT NULL DEFAULT '', "
                "   corpusID varchar(64), "
                "   communicationName varchar(128) ";
        sqlKeys = ", PRIMARY KEY (communicationID) )";
    }
    else if (what == CorpusObject::Type_Speaker) {
        tableName = "speaker";
        sqlBase = "CREATE TABLE speaker ( "
                "   speakerID varchar(64) NOT NULL DEFAULT '', "
                "   corpusID varchar(64), "
                "   speakerName varchar(128) ";
        sqlKeys = ", PRIMARY KEY (speakerID) )";
    }
    else if (what == CorpusObject::Type_Recording) {
        tableName = "recording";
        sqlBase = "CREATE TABLE recording ( "
                "   recordingID varchar(64) NOT NULL DEFAULT '', "
                "   communicationID varchar(64), "
                "   recordingName varchar(128), "
                "   filename varchar(256), "
                "   format varchar(32), "
                "   duration bigint(20), "
                "   channels tinyint(4) DEFAULT 1, "
                "   sampleRate int(11), "
                "   precisionBits tinyint(2), "
                "   bitRate int(11), "
                "   encoding varchar(256), "
                "   fileSize int(11), "
                "   checksumMD5 varchar(64) ";
        sqlKeys = ", PRIMARY KEY (recordingID) )";
    }
    else if (what == CorpusObject::Type_Annotation) {
        tableName = "annotation";
        sqlBase = "CREATE TABLE annotation ( "
                "   annotationID varchar(64) NOT NULL DEFAULT '', "
                "   communicationID varchar(64), "
                "   recordingID varchar(64), "
                "   annotationName varchar(128) ";
        sqlKeys = ", PRIMARY KEY (annotationID) )";
    }
    else if (what == CorpusObject::Type_Participation) {
        tableName = "participation";
        sqlBase = "CREATE TABLE participation ( "
                "   corpusID varchar(64), "
                "   communicationID varchar(64) NOT NULL DEFAULT '', "
                "   speakerID varchar(64) NOT NULL DEFAULT '', "
                "   role varchar(128) ";
        sqlKeys = ", PRIMARY KEY (communicationID, speakerID) )";
    }
    else return false;
    QSqlQuery q(db), qdel(db);
    qdel.prepare(QString("DROP TABLE IF EXISTS %1").arg(tableName));
    qdel.exec();
    if (qdel.lastError().isValid()) return false;
    foreach (MetadataStructureSection *section, structure->sections(what)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            sqlMetadata.append(", ").append(attribute->ID()).append(" ").append(attribute->datatype());
            if (attribute->datalength() > 0)
                sqlMetadata.append("(").append(QString::number(attribute->datalength())).append(")");
            sqlMetadata.append(" ");
        }
    }
    q.prepare(QString("%1 %2 %3").arg(sqlBase).arg(sqlMetadata).arg(sqlKeys));
    q.exec();
    if (q.lastError().isValid()) {
        return false;
    }
    return true;
}

// static
void SQLSchemaProxyMetadata::createSchema(MetadataStructure *structure, QSqlDatabase &db)
{
    db.transaction();
    if (!createNewSchema(structure, CorpusObject::Type_Communication, db)) { db.rollback(); return; }
    if (!createNewSchema(structure, CorpusObject::Type_Speaker, db)) { db.rollback(); return; }
    if (!createNewSchema(structure, CorpusObject::Type_Recording, db)) { db.rollback(); return; }
    if (!createNewSchema(structure, CorpusObject::Type_Annotation, db)) { db.rollback(); return; }
    if (!createNewSchema(structure, CorpusObject::Type_Participation, db)) { db.rollback(); return; }
    db.commit();
}

QString getTableName(CorpusObject::Type type)
{
    if (type == CorpusObject::Type_Communication)       return "communication";
    else if (type == CorpusObject::Type_Speaker)        return "speaker";
    else if (type == CorpusObject::Type_Recording)      return "recording";
    else if (type == CorpusObject::Type_Annotation)     return "annotation";
    else if (type == CorpusObject::Type_Participation)  return "participation";
    return QString();
}

// static
bool SQLSchemaProxyMetadata::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute, QSqlDatabase &db)
{
    QString tableName = getTableName(type);
    if (tableName.isEmpty()) return false;
    if (!newAttribute) return false;
    QString datatype = newAttribute->datatype();
    if (newAttribute->datatype() == "varchar") datatype.append(QString("(%1)").arg(newAttribute->datalength()));
    return addColumnToTable(tableName, newAttribute->ID(), datatype, db);
}

// static
bool SQLSchemaProxyMetadata::renameMetadataAttribute(CorpusObject::Type type, QString attributeID, QString newAttributeID, QSqlDatabase &db)
{
    QString tableName = getTableName(type);
    if (tableName.isEmpty()) return false;
    return renameColumn(tableName, attributeID, newAttributeID, db);
}

// static
bool SQLSchemaProxyMetadata::deleteMetadataAttribute(CorpusObject::Type type, QString attributeID, QSqlDatabase &db)
{
    QString tableName = getTableName(type);
    if (tableName.isEmpty()) return false;
    return deleteColumn(tableName, attributeID, db);
}


