#include <QDebug>
#include <QString>
#include <QList>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "QSqlMigrator/QSqlMigrator.h"
using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

#include "SQLSerialiserSystem.h"
#include "SQLSerialiserBase.h"
#include "SQLSerialiserMetadataStructure.h"

namespace Praaline {
namespace Core {

CorpusObject::Type corpusObjectTypeFromCode(const QString &code)
{
    CorpusObject::Type objectType = CorpusObject::Type_Undefined;
    if      (code == "X") objectType = CorpusObject::Type_Corpus;
    else if (code == "C") objectType = CorpusObject::Type_Communication;
    else if (code == "S") objectType = CorpusObject::Type_Speaker;
    else if (code == "R") objectType = CorpusObject::Type_Recording;
    else if (code == "A") objectType = CorpusObject::Type_Annotation;
    else if (code == "P") objectType = CorpusObject::Type_Participation;
    return objectType;
}

QString corpusObjectCodeFromType(const CorpusObject::Type type)
{
    if      (type == CorpusObject::Type_Corpus)         return "X";
    else if (type == CorpusObject::Type_Communication)  return "C";
    else if (type == CorpusObject::Type_Speaker)        return "S";
    else if (type == CorpusObject::Type_Recording)      return "R";
    else if (type == CorpusObject::Type_Annotation)     return "A";
    else if (type == CorpusObject::Type_Participation)  return "P";
    return QString();
}

// static
bool SQLSerialiserMetadataStructure::initialiseMetadataStructureTables(QSqlDatabase &db)
{
    Migrations::Migration initializeMetadataStructure;
    Table::Builder tableMetadataSections("praalineMetadataSections");
    tableMetadataSections
            << Column("objectType", SqlType(SqlType::Char, 1), "", Column::Primary)
            << Column("sectionID", SqlType(SqlType::VarChar, 32), "", Column::Primary)
            << Column("name", SqlType(SqlType::VarChar, 255))
            << Column("description", SqlType(SqlType::VarChar, 1024));
    initializeMetadataStructure.add(new CreateTable(tableMetadataSections));
    Table::Builder tableMetadataAttributes("praalineMetadataAttributes");
    tableMetadataAttributes
            << Column("objectType", SqlType(SqlType::Char, 1), "", Column::Primary)
            << Column("attributeID", SqlType(SqlType::VarChar, 32), "", Column::Primary)
            << Column("sectionID", SqlType(SqlType::VarChar, 32))
            << Column("name", SqlType(SqlType::VarChar, 255))
            << Column("description", SqlType(SqlType::VarChar, 1024))
            << Column("datatype", SqlType(SqlType::VarChar, 32))
            << Column("length", SqlType::Integer)
            << Column("isIndexed", SqlType::Boolean)
            << Column("nameValueList", SqlType(SqlType::VarChar, 32))
            << Column("mandatory", SqlType::Boolean)
            << Column("order", SqlType::Integer);
    initializeMetadataStructure.add(new CreateTable(tableMetadataAttributes));

    SQLSerialiserBase::applyMigration("initializeMetadataStructure", &initializeMetadataStructure, db);
}

bool createNewSchema(MetadataStructure *structure, CorpusObject::Type what, QSqlDatabase &db)
{
    Migrations::Migration initializeTable;
    QString tableName;
    ColumnList columns;
    if (what == CorpusObject::Type_Corpus) {
        tableName = "corpus";
        columns << Column("corpusID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("corpusName", SqlType(SqlType::VarChar, 128)) <<
                   Column("description", SqlType(SqlType::VarChar, 128));
    }
    else if (what == CorpusObject::Type_Communication) {
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
                   Column("communicationID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("speakerID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
                   Column("role", SqlType(SqlType::VarChar, 128));
    }
    else return false;

    foreach (MetadataStructureSection *section, structure->sections(what)) {
        foreach (MetadataStructureAttribute *attribute, section->attributes()) {
            columns << Column(attribute->ID(), SqlType(attribute->datatype()));
        }
    }

    Table::Builder table(tableName, columns);
    initializeTable.add(new CreateTable(table));
    return SQLSerialiserBase::applyMigration(QString("initialize_%1").arg(tableName),
                                             &initializeTable, db);
}

// static
bool SQLSerialiserMetadataStructure::createMetadataSchema(QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    db.transaction();
    if (!createNewSchema(structure, CorpusObject::Type_Corpus, db))         { db.rollback(); return false; }
    if (!createNewSchema(structure, CorpusObject::Type_Communication, db))  { db.rollback(); return false; }
    if (!createNewSchema(structure, CorpusObject::Type_Speaker, db))        { db.rollback(); return false; }
    if (!createNewSchema(structure, CorpusObject::Type_Recording, db))      { db.rollback(); return false; }
    if (!createNewSchema(structure, CorpusObject::Type_Annotation, db))     { db.rollback(); return false; }
    if (!createNewSchema(structure, CorpusObject::Type_Participation, db))  { db.rollback(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSerialiserMetadataStructure::loadMetadataStructure(QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    if (!structure) return false;
    QSqlQuery q1(db), q2(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM praalineMetadataSections");
    q2.setForwardOnly(true);
    q2.prepare("SELECT * FROM praalineMetadataAttributes WHERE sectionID = :sectionID ORDER BY order");
    //
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
    structure->clearAll();
    while (q1.next()) {
        CorpusObject::Type objectType = corpusObjectTypeFromCode(q1.value("objectType").toString());
        MetadataStructureSection *section = new MetadataStructureSection(q1.value("sectionID").toString(),
                                                                         q1.value("name").toString(),
                                                                         q1.value("description").toString());
        q2.bindValue(":sectionID", section->ID());
        q2.exec();
        while (q2.next()) {
            MetadataStructureAttribute *attribute = new MetadataStructureAttribute();
            attribute->setID(q2.value("attributeID").toString());
            attribute->setName(q2.value("name").toString());
            attribute->setDescription(q2.value("description").toString());
            attribute->setDatatype(DataType(q2.value("datatype").toString()));
            attribute->setDatatype(DataType(attribute->datatype().base(), q2.value("length").toInt()));
            if (q2.value("isIndexed").toInt() > 0) attribute->setIndexed(true); else attribute->setIndexed(false);
            attribute->setNameValueList(q2.value("nameValueList").toString());
            attribute->setMandatory(q2.value("mandatory").toBool());
            attribute->setOrder(q2.value("order").toInt());
            attribute->setParent(section);
            section->addAttribute(attribute);
        }
        section->setParent(structure);
        structure->addSection(objectType, section);
    }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::createMetadataSection(QPointer<MetadataStructureSection> newSection, QSqlDatabase &db)
{
    if (!newSection) return false;
    QSqlQuery q(db);
    q.prepare("INSERT INTO praalineMetadataSections (sectionID, name, description) VALUES (:sectionID, :name, :description) ");
    q.bindValue(":sectionID", newSection->ID());
    q.bindValue(":name", newSection->name());
    q.bindValue(":description", newSection->description());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::updateMetadataSection(QPointer<MetadataStructureSection> updatedSection, QSqlDatabase &db)
{
    if (!updatedSection) return false;
    QSqlQuery q(db);
    q.prepare("UPDATE praalineMetadataSections SET name=:name, description=:description WHERE sectionID=:sectionID ");
    q.bindValue(":sectionID", updatedSection->ID());
    q.bindValue(":name", updatedSection->name());
    q.bindValue(":description", updatedSection->description());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::deleteMetadataSection(const QString &sectionID, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserMetadataStructure::createMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> newAttribute,
                                                             QSqlDatabase &db)
{
    QString tableName = SQLSerialiserSystem::tableNameForCorpusObjectType(type);
    if (tableName.isEmpty()) return false;
    if (!newAttribute) return false;
    bool result = addColumnToTable(tableName, newAttribute->ID(), newAttribute->datatype(), db);
    if (!result) return false;
    QSqlQuery q(db);
    q.prepare("INSERT INTO praalineMetadataAttributes "
              "(objectType, attributeID, sectionID, name, description, datatype, length, isIndexed, nameValueList, mandatory, order) "
              "VALUES (:objectType, :attributeID, :sectionID, :name, :description, :datatype, :length, :isIndexed, :nameValueList, :mandatory, :order) ");
    q.bindValue(":objectType", corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", newAttribute->ID());
    q.bindValue(":sectionID", newAttribute->sectionID());
    q.bindValue(":name", newAttribute->name());
    q.bindValue(":description", newAttribute->description());
    q.bindValue(":datatype", newAttribute->datatypeString());
    q.bindValue(":length", newAttribute->datatypePrecision());
    q.bindValue(":isIndexed", (newAttribute->indexed()) ? 1 : 0);
    q.bindValue(":nameValueList", newAttribute->nameValueList());
    q.bindValue(":mandatory", (newAttribute->mandatory()) ? 1 : 0);
    q.bindValue(":order", newAttribute->order());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::updateMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> updatedAttribute,
                                                             QSqlDatabase &db)
{
    if (!updatedAttribute) return false;
    QSqlQuery q(db);
    q.prepare("UPDATE praalineMetadataAttributes SET sectionID=:sectionID, name=:name, description=:description, "
              "nameValueList=:nameValueList, mandatory=:mandatory, order=:order "
              "WHERE objectType=:objectType AND attributeID=:attributeID");
    q.bindValue(":objectType", corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", updatedAttribute->ID());
    q.bindValue(":sectionID", updatedAttribute->sectionID());
    q.bindValue(":name", updatedAttribute->name());
    q.bindValue(":description", updatedAttribute->description());
    q.bindValue(":nameValueList", updatedAttribute->nameValueList());
    q.bindValue(":mandatory", (updatedAttribute->mandatory()) ? 1 : 0);
    q.bindValue(":order", updatedAttribute->order());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::renameMetadataAttribute(CorpusObject::Type type, const QString &attributeID, const QString &newAttributeID,
                                                             QSqlDatabase &db)
{
    QString tableName = SQLSerialiserSystem::tableNameForCorpusObjectType(type);
    if (tableName.isEmpty()) return false;
    bool result = renameColumn(tableName, attributeID, newAttributeID, db);
    if (!result) return false;
    QSqlQuery q(db);
    q.prepare("UPDATE praalineMetadataAttributes SET attributeID=:newAttributeID WHERE objectType=:objectType AND attributeID=:attributeID ");
    q.bindValue(":objectType", corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", attributeID);
    q.bindValue(":newAttributeID", newAttributeID);
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID,
                                                             const DataType &oldDataType, const DataType &newDataType, QSqlDatabase &db)
{
    QString tableName = SQLSerialiserSystem::tableNameForCorpusObjectType(type);
    if (tableName.isEmpty()) return false;
    bool result = retypeColumn(tableName, attributeID, oldDataType, newDataType, db);
    if (!result) return false;
    QSqlQuery q(db);

}

// static
bool SQLSerialiserMetadataStructure::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID, QSqlDatabase &db)
{
    QString tableName = SQLSerialiserSystem::tableNameForCorpusObjectType(type);
    if (tableName.isEmpty()) return false;
    return deleteColumn(tableName, attributeID, db);
}

} // namespace Core
} // namespace Praaline
