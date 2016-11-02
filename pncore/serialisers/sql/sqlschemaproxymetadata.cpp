#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "QSqlMigrator/QSqlMigrator.h"
#include "sqlschemaproxymetadata.h"

using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

SQLSchemaProxyMetadata::SQLSchemaProxyMetadata()
{
}

bool createNewSchema(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db)
{
    QString tableName, sqlBase, sqlKeys, sqlMetadata;
    ColumnList columns;
    if (what == CorpusObject::Type_Communication) {
        tableName = "communication";
        columns << Column("communicationID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("corpusID", SqlType(SqlType::VarChar, 64)) <<
                   Column("communicationName", SqlType(SqlType::VarChar, 128));
    }
    else if (what == CorpusObject::Type_Speaker) {
        tableName = "speaker";
        columns << Column("speakerID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("corpusID", SqlType(SqlType::VarChar, 64)) <<
                   Column("speakerName", SqlType(SqlType::VarChar, 128));
    }
    else if (what == CorpusObject::Type_Recording) {
        tableName = "recording";
        columns << Column("recordingID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("communicationID", SqlType(SqlType::VarChar, 64)) <<
                   Column("recordingName", SqlType(SqlType::VarChar, 128)) <<
                   Column("filename", SqlType(SqlType::VarChar, 256)) <<
                   Column("format", SqlType(SqlType::VarChar, 32)) <<
                   Column("duration", SqlType::BigInt) <<
                   Column("channels", SqlType::SmallInt, "1") <<
                   Column("sampleRate", SqlType::Integer) <<
                   Column("precisionBits", SqlType::SmallInt) <<
                   Column("bitRate", SqlType::Integer) <<
                   Column("encoding", SqlType(SqlType::VarChar, 256)) <<
                   Column("fileSize", SqlType::BigInt) <<
                   Column("checksumMD5", SqlType(SqlType::VarChar, 64));
    }
    else if (what == CorpusObject::Type_Annotation) {
        tableName = "annotation";
        columns << Column("annotationID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("communicationID", SqlType(SqlType::VarChar, 64)) <<
                   Column("recordingID", SqlType(SqlType::VarChar, 64)) <<
                   Column("annotationName", SqlType(SqlType::VarChar, 64));
    }
    else if (what == CorpusObject::Type_Participation) {
        tableName = "participation";
        columns << Column("corpusID", SqlType(SqlType::VarChar, 64)) <<
                   Column("communicationID", SqlType(SqlType::VarChar, 64), "", Column::NotNullable) <<
                   Column("speakerID", SqlType(SqlType::VarChar, 64), "", Column::NotNullable) <<
                   Column("role", SqlType(SqlType::VarChar, 128));
    }
    else return false;
    // Drop table if exists
//    QSqlQuery q(db), qdel(db);
//    qdel.prepare(QString("DROP TABLE IF EXISTS %1").arg(tableName));
//    qdel.exec();
//    if (qdel.lastError().isValid()) return false;
    foreach (MetadataStructureSection *section, structure->sections(what)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {

            //


            sqlMetadata.append(", ").append(attribute->ID()).append(" ").append(attribute->datatype());
            if (attribute->datalength() > 0)
                sqlMetadata.append("(").append(QString::number(attribute->datalength())).append(")");
            sqlMetadata.append(" ");
        }
    }
//    q.prepare(QString("%1 %2 %3").arg(sqlBase).arg(sqlMetadata).arg(sqlKeys));
//    q.exec();
//    if (q.lastError().isValid()) {
//        return false;
//    }
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


