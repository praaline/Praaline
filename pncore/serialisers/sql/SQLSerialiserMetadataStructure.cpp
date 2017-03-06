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

// static
bool SQLSerialiserMetadataStructure::initialiseMetadataStructureSchema(QSqlDatabase &db)
{
    Migrations::Migration initializeMetadataStructure;
    Table::Builder tableMetadataSections("praalineMetadataSections");
    tableMetadataSections
            << Column("objectType", SqlType(SqlType::Char, 1), "", Column::Primary)
            << Column("sectionID", SqlType(SqlType::VarChar, 32), "", Column::Primary)
            << Column("name", SqlType(SqlType::VarChar, 255))
            << Column("description", SqlType(SqlType::VarChar, 1024))
            << Column("itemOrder", SqlType::Integer);
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
            << Column("itemOrder", SqlType::Integer);
    initializeMetadataStructure.add(new CreateTable(tableMetadataAttributes));
    // Try creating tables
    bool result = SQLSerialiserBase::applyMigration("initializeMetadataStructure", &initializeMetadataStructure, db);
    if (result) {
        setPraalineSchemaVersion(3, db);
    }
    // Create default sections
    QScopedPointer<MetadataStructureSection> section_corpus(new MetadataStructureSection("corpus", "Corpus", "Corpus metadata", 0));
    QScopedPointer<MetadataStructureSection> section_communication(new MetadataStructureSection("communication", "Communication", "Communication metadata", 0));
    QScopedPointer<MetadataStructureSection> section_speaker(new MetadataStructureSection("speaker", "Speaker", "Speaker metadata", 0));
    QScopedPointer<MetadataStructureSection> section_recording(new MetadataStructureSection("recording", "Recording", "Recording metadata", 0));
    QScopedPointer<MetadataStructureSection> section_annotation(new MetadataStructureSection("annotation", "Annotation", "Annotation metadata", 0));
    QScopedPointer<MetadataStructureSection> section_participation(new MetadataStructureSection("participation", "Participation ", "Participation metadata", 0));
    result = result && createMetadataSection(CorpusObject::Type_Corpus, section_corpus.data(), db);
    result = result && createMetadataSection(CorpusObject::Type_Communication, section_communication.data(), db);
    result = result && createMetadataSection(CorpusObject::Type_Speaker, section_speaker.data(), db);
    result = result && createMetadataSection(CorpusObject::Type_Recording, section_recording.data(), db);
    result = result && createMetadataSection(CorpusObject::Type_Annotation, section_annotation.data(), db);
    result = result && createMetadataSection(CorpusObject::Type_Participation, section_participation.data(), db);
    return result;
}

// static
bool SQLSerialiserMetadataStructure::upgradeMetadataStructureSchema(QSqlDatabase &db)
{
    int schemaVersion = getPraalineSchemaVersion(db);
    if (schemaVersion == 0) {
        if (db.tables().contains("praalineMetadataSections") && db.tables().contains("praalineMetadataAttributes")) {
            bool result = true;
            result = result && addColumnToTable("praalineMetadataSections", "itemOrder", DataType::Integer, db);
            result = result && addColumnToTable("praalineMetadataAttributes", "mandatory", DataType::Boolean, db);
            result = result && addColumnToTable("praalineMetadataAttributes", "itemOrder", DataType::Integer, db);
            if (result) setPraalineSchemaVersion(3, db);
        }
        else {
            return initialiseMetadataStructureSchema(db);
        }
    }
    return true;
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
        columns << Column("corpusID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
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
bool SQLSerialiserMetadataStructure::initialiseMetadataSchema(QPointer<MetadataStructure> structure, QSqlDatabase &db)
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
bool SQLSerialiserMetadataStructure::upgradeMetadataSchema(QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    bool result = true;
    if (!db.tables().contains("corpus"))        result = result && createNewSchema(structure, CorpusObject::Type_Corpus, db);
    if (!db.tables().contains("communication")) result = result && createNewSchema(structure, CorpusObject::Type_Communication, db);
    if (!db.tables().contains("speaker"))       result = result && createNewSchema(structure, CorpusObject::Type_Speaker, db);
    if (!db.tables().contains("recording"))     result = result && createNewSchema(structure, CorpusObject::Type_Recording, db);
    if (!db.tables().contains("annotation"))    result = result && createNewSchema(structure, CorpusObject::Type_Annotation, db);
    if (!db.tables().contains("participation")) result = result && createNewSchema(structure, CorpusObject::Type_Participation, db);
    return result;
}

void readAttribute(QSqlQuery &q, MetadataStructureAttribute *attribute)
{
    attribute->setID(q.value("attributeID").toString());
    attribute->setName(q.value("name").toString());
    attribute->setDescription(q.value("description").toString());
    attribute->setDatatype(DataType(q.value("datatype").toString()));
    attribute->setDatatype(DataType(attribute->datatype().base(), q.value("length").toInt()));
    if (q.value("isIndexed").toInt() > 0) attribute->setIndexed(true); else attribute->setIndexed(false);
    attribute->setNameValueList(q.value("nameValueList").toString());
    attribute->setMandatory(q.value("mandatory").toBool());
    attribute->setItemOrder(q.value("itemOrder").toInt());
}

// static
bool SQLSerialiserMetadataStructure::loadMetadataStructure(QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    if (!structure) return false;
    QSqlQuery q1(db), q2(db), q3(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM praalineMetadataSections ORDER BY itemOrder");
    q2.setForwardOnly(true);
    q2.prepare("SELECT * FROM praalineMetadataAttributes "
               "WHERE objectType=:objectType AND sectionID = :sectionID ORDER BY itemOrder");
    q3.setForwardOnly(true);
    q3.prepare("SELECT * FROM praalineMetadataAttributes "
               "WHERE objectType=:objectType AND sectionID IS NULL ORDER BY itemOrder");
    //
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
    structure->clearAll();
    // Read sections
    QStringList sectionIDsInDatabase;
    while (q1.next()) {
        CorpusObject::Type objectType = SQLSerialiserSystem::corpusObjectTypeFromCode(q1.value("objectType").toString());
        MetadataStructureSection *section(0);
        bool sectionExists(false);
        QString sectionID = q1.value("sectionID").toString();
        if (sectionID.isEmpty()) sectionID = MetadataStructure::defaultSectionID(objectType);
        section = structure->section(objectType, sectionID);
        if (section)
            sectionExists = true;
        else {
            section = new MetadataStructureSection(q1.value("sectionID").toString(),
                                                   q1.value("name").toString(),
                                                   q1.value("description").toString(),
                                                   q1.value("itemOrder").toInt());
        }
        if (!sectionExists) {
            section->setParent(structure);
            structure->addSection(objectType, section);
        }
        sectionIDsInDatabase << sectionID;

    }
    // Make sure default sections exist in the database
    MetadataStructureSection *section(0);
    if (!sectionIDsInDatabase.contains("corpus")) {
        section = new MetadataStructureSection("corpus", "Corpus", "Corpus metadata", 0);
        createMetadataSection(CorpusObject::Type_Corpus, section, db); structure->addSection(CorpusObject::Type_Corpus, section);
    }
    if (!sectionIDsInDatabase.contains("communication")) {
        section = new MetadataStructureSection("communication", "Communication", "Communication metadata", 0);
        createMetadataSection(CorpusObject::Type_Communication, section, db); structure->addSection(CorpusObject::Type_Communication, section);
    }
    if (!sectionIDsInDatabase.contains("speaker")) {
        section = new MetadataStructureSection("speaker", "Speaker", "Speaker metadata", 0);
        createMetadataSection(CorpusObject::Type_Speaker, section, db); structure->addSection(CorpusObject::Type_Speaker, section);
    }
    if (!sectionIDsInDatabase.contains("recording")) {
        section = new MetadataStructureSection("recording", "Recording", "Recording metadata", 0);
        createMetadataSection(CorpusObject::Type_Recording, section, db); structure->addSection(CorpusObject::Type_Recording, section);
    }
    if (!sectionIDsInDatabase.contains("annotation")) {
        section = new MetadataStructureSection("annotation", "Annotation", "Annotation metadata", 0);
        createMetadataSection(CorpusObject::Type_Annotation, section, db); structure->addSection(CorpusObject::Type_Annotation, section);
    }
    if (!sectionIDsInDatabase.contains("participation")) {
        section = new MetadataStructureSection("participation", "Participation ", "Participation metadata", 0);
        createMetadataSection(CorpusObject::Type_Participation, section, db); structure->addSection(CorpusObject::Type_Participation, section);
    }
    // Read attributes to sections
    QList<CorpusObject::Type> objectTypes;
    objectTypes << CorpusObject::Type_Corpus << CorpusObject::Type_Communication << CorpusObject::Type_Speaker
                << CorpusObject::Type_Recording << CorpusObject::Type_Annotation << CorpusObject::Type_Participation;
    foreach (CorpusObject::Type objectType, objectTypes) {
        foreach (QPointer<MetadataStructureSection> section, structure->sections(objectType)) {
            q2.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(objectType));
            q2.bindValue(":sectionID", section->ID());
            q2.exec();
            while (q2.next()) {
                MetadataStructureAttribute *attribute = new MetadataStructureAttribute();
                readAttribute(q2, attribute);
                attribute->setParent(section);
                section->addAttribute(attribute);
            }
        }
        // For attributes without a section (move to default section)
        QString sectionID = MetadataStructure::defaultSectionID(objectType);
        MetadataStructureSection *section(0);
        section = structure->section(objectType, sectionID);
        if (!section) continue;
        q3.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(objectType));
        q3.exec();
        if (q3.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
        while (q3.next()) {
            MetadataStructureAttribute *attribute = new MetadataStructureAttribute();
            readAttribute(q3, attribute);
            attribute->setParent(section);
            section->addAttribute(attribute);
        }
    }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::saveMetadataStructure(QPointer<MetadataStructure> structure, QSqlDatabase &db)
{
    if (!structure) return false;
    QSqlQuery q1(db), q2(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT sectionID FROM praalineMetadataSections WHERE objectType = :objectType");
    q2.setForwardOnly(true);
    q2.prepare("SELECT attributeID FROM praalineMetadataAttributes WHERE objectType = :objectType");
    QList<CorpusObject::Type> objectTypes;
    objectTypes << CorpusObject::Type_Corpus << CorpusObject::Type_Communication << CorpusObject::Type_Speaker
                << CorpusObject::Type_Recording << CorpusObject::Type_Annotation << CorpusObject::Type_Participation;
    bool result = true;
    foreach (CorpusObject::Type objectType, objectTypes) {
        QStringList sectionIDsInDatabase, sectionIDsInStructure, attributeIDsInDatabase, attributeIDsInStructure;
        q1.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(objectType));
        q1.exec();
        while (q1.next()) sectionIDsInDatabase << q1.value("sectionID").toString();
        q2.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(objectType));
        q2.exec();
        while (q2.next()) attributeIDsInDatabase << q2.value("attributeID").toString();
        // Update (possilby insert)
        foreach (QPointer<MetadataStructureSection> section, structure->sections(objectType)) {
            result = result && updateMetadataSection(objectType, section, db);
            sectionIDsInStructure << section->ID();
            foreach (QPointer<MetadataStructureAttribute> attribute, section->attributes()) {
                result = result && updateMetadataAttribute(objectType, attribute, db);
                attributeIDsInStructure << attribute->ID();
            }
        }
        // Delete if necessary
        foreach (QString attributeID, attributeIDsInDatabase) {
            if (!attributeIDsInStructure.contains(attributeID))
                result = result && deleteMetadataAttribute(objectType, attributeID, db);
        }
        foreach (QString sectionID, sectionIDsInDatabase) {
            if (!sectionIDsInStructure.contains(sectionID))
                result = result && deleteMetadataSection(objectType, sectionID, db);
        }
    }
    return result;
}

// static
bool SQLSerialiserMetadataStructure::createMetadataSection(CorpusObject::Type type, QPointer<MetadataStructureSection> newSection, QSqlDatabase &db)
{
    if (!newSection) return false;
    QSqlQuery q(db);
    q.prepare("INSERT INTO praalineMetadataSections (objectType, sectionID, name, description, itemOrder) "
              "VALUES (:objectType, :sectionID, :name, :description, :itemOrder) ");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    QString sectionID = (newSection->ID().isEmpty()) ? MetadataStructure::defaultSectionID(type) : newSection->ID();
    q.bindValue(":sectionID", sectionID);
    q.bindValue(":name", newSection->name());
    q.bindValue(":description", newSection->description());
    q.bindValue(":itemOrder", newSection->itemOrder());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::updateMetadataSection(CorpusObject::Type type, QPointer<MetadataStructureSection> updatedSection, QSqlDatabase &db)
{
    if (!updatedSection) return false;
    QSqlQuery q_exists(db), q(db);
    // Check if section exists - if not, create it
    q_exists.prepare("SELECT sectionID FROM praalineMetadataSections WHERE sectionID=:sectionID ");
    q_exists.bindValue(":sectionID", updatedSection->ID());
    q_exists.exec();
    bool exists = false;
    while (q_exists.next()) exists = true;
    if (!exists) return createMetadataSection(type, updatedSection, db);
    // Otherwise, ok to update
    q.prepare("UPDATE praalineMetadataSections SET name=:name, description=:description, itemOrder=:itemOrder "
              "WHERE objectType=:objectType AND sectionID=:sectionID ");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    QString sectionID = (updatedSection->ID().isEmpty()) ? MetadataStructure::defaultSectionID(type) : updatedSection->ID();
    q.bindValue(":sectionID", sectionID);
    q.bindValue(":name", updatedSection->name());
    q.bindValue(":description", updatedSection->description());
    q.bindValue(":itemOrder", updatedSection->itemOrder());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::deleteMetadataSection(CorpusObject::Type type, const QString &sectionID, QSqlDatabase &db)
{
    if (sectionID.isEmpty()) return false;
    db.transaction();
    QSqlQuery qupd(db), qdel(db);
    qupd.prepare("UPDATE praalineMetadataAttributes SET sectionID = :defaultSectionID WHERE objectType=:objectType AND sectionID = :sectionID ");
    qupd.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    qupd.bindValue(":sectionID", sectionID);
    qupd.bindValue(":defaultSectionID", SQLSerialiserSystem::defaultSectionID(type));
    qupd.exec();
    if (qupd.lastError().isValid()) { qDebug() << qupd.lastError(); db.rollback(); return false; }
    qdel.prepare("DELETE FROM praalineMetadataSections WHERE objectType=:objectType AND sectionID = :sectionID ");
    qdel.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    qdel.bindValue(":sectionID", sectionID);
    qdel.exec();
    if (qdel.lastError().isValid()) { qDebug() << qdel.lastError(); db.rollback(); return false; }
    db.commit();
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
              "       (objectType, attributeID, sectionID, name, description, datatype, length, isIndexed, nameValueList, mandatory, itemOrder) "
              "VALUES (:objectType, :attributeID, :sectionID, :name, :description, :datatype, :length, :isIndexed, :nameValueList, :mandatory, :itemOrder) ");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", newAttribute->ID());
    q.bindValue(":sectionID", newAttribute->sectionID());
    q.bindValue(":name", newAttribute->name());
    q.bindValue(":description", newAttribute->description());
    q.bindValue(":datatype", newAttribute->datatypeString());
    q.bindValue(":length", newAttribute->datatypePrecision());
    q.bindValue(":isIndexed", (newAttribute->indexed()) ? 1 : 0);
    q.bindValue(":nameValueList", newAttribute->nameValueList());
    q.bindValue(":mandatory", (newAttribute->mandatory()) ? 1 : 0);
    q.bindValue(":itemOrder", newAttribute->itemOrder());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::updateMetadataAttribute(CorpusObject::Type type, QPointer<MetadataStructureAttribute> updatedAttribute,
                                                             QSqlDatabase &db)
{
    if (!updatedAttribute) return false;
    QSqlQuery q_exists(db), q(db);
    // Check if attribute exists - if not, create it
    q_exists.prepare("SELECT attributeID FROM praalineMetadataAttributes WHERE attributeID=:attributeID ");
    q_exists.bindValue(":attributeID", updatedAttribute->ID());
    q_exists.exec();
    bool exists = false;
    while (q_exists.next()) exists = true;
    if (!exists) return createMetadataAttribute(type, updatedAttribute, db);
    // Otherwise, ok to update
    q.prepare("UPDATE praalineMetadataAttributes SET sectionID=:sectionID, name=:name, description=:description, "
              "nameValueList=:nameValueList, mandatory=:mandatory, itemOrder=:itemOrder "
              "WHERE objectType=:objectType AND attributeID=:attributeID");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", updatedAttribute->ID());
    q.bindValue(":sectionID", updatedAttribute->sectionID());
    q.bindValue(":name", updatedAttribute->name());
    q.bindValue(":description", updatedAttribute->description());
    q.bindValue(":nameValueList", updatedAttribute->nameValueList());
    q.bindValue(":mandatory", (updatedAttribute->mandatory()) ? 1 : 0);
    q.bindValue(":itemOrder", updatedAttribute->itemOrder());
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
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", attributeID);
    q.bindValue(":newAttributeID", newAttributeID);
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::retypeMetadataAttribute(CorpusObject::Type type, const QString &attributeID,
                                                             const DataType &newDataType, QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.prepare("SELECT datatype, length FROM praalineMetadataAttributes "
              "WHERE objectType=:objectType AND attributeID=:attributeID ");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", attributeID);
    if (!q.exec()) { qDebug() << q.lastError(); return false; }
    DataType oldDataType("");
    while (q.next()) {
        oldDataType = DataType(q.value("datatype").toString());
        oldDataType = DataType(oldDataType.base(), q.value("length").toInt());
    }
    if (!oldDataType.isValid()) return false;

    QString tableName = SQLSerialiserSystem::tableNameForCorpusObjectType(type);
    if (tableName.isEmpty()) return false;
    bool result = retypeColumn(tableName, attributeID, oldDataType, newDataType, db);
    if (!result) return false;

    q.prepare("UPDATE praalineMetadataAttributes SET datatype=:datatype, length=:length "
              "WHERE objectType=:objectType AND attributeID=:attributeID ");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", attributeID);
    q.bindValue(":datatype", newDataType.string());
    q.bindValue(":length", newDataType.precision());
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

// static
bool SQLSerialiserMetadataStructure::deleteMetadataAttribute(CorpusObject::Type type, const QString &attributeID, QSqlDatabase &db)
{
    QString tableName = SQLSerialiserSystem::tableNameForCorpusObjectType(type);
    if (tableName.isEmpty()) return false;
    bool result = deleteColumn(tableName, attributeID, db);
    if (!result) return false;
    QSqlQuery q(db);
    q.prepare("DELETE FROM praalineMetadataAttributes WHERE objectType=:objectType AND attributeID=:attributeID ");
    q.bindValue(":objectType", SQLSerialiserSystem::corpusObjectCodeFromType(type));
    q.bindValue(":attributeID", attributeID);
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
    return true;
}

} // namespace Core
} // namespace Praaline
