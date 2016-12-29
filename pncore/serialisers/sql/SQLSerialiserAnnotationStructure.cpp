#include <QDebug>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "QSqlMigrator/QSqlMigrator.h"
using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

#include "SQLSerialiserBase.h"
#include "SQLSerialiserAnnotationStructure.h"

namespace Praaline {
namespace Core {

// static
bool SQLSerialiserAnnotationStructure::initialiseAnnotationStructureTables(QSqlDatabase &db)
{
    Migrations::Migration initializeAnnotationStructure;
    Table::Builder tableAnnotationLevels("praalineAnnotationLevels");
    tableAnnotationLevels
            << Column("levelID", SqlType(SqlType::VarChar, 32), "", Column::Primary)
            << Column("levelType", SqlType::SmallInt)
            << Column("parentLevelID", SqlType(SqlType::VarChar, 32))
            << Column("name", SqlType(SqlType::VarChar, 128))
            << Column("description", SqlType(SqlType::VarChar, 1024))
            << Column("datatype", SqlType(SqlType::VarChar, 32))
            << Column("length", SqlType::Integer)
            << Column("isIndexed", SqlType::Boolean)
            << Column("nameValueList", SqlType(SqlType::VarChar, 32));
    initializeAnnotationStructure.add(new CreateTable(tableAnnotationLevels));
    Table::Builder tableAnnotationAttributes("praalineAnnotationAttributes");
    tableAnnotationAttributes
            << Column("attributeID", SqlType(SqlType::VarChar, 32), "", Column::Primary)
            << Column("levelID", SqlType(SqlType::VarChar, 32), "", Column::Primary)
            << Column("name", SqlType(SqlType::VarChar, 128))
            << Column("description", SqlType(SqlType::VarChar, 1024))
            << Column("datatype", SqlType(SqlType::VarChar, 32))
            << Column("length", SqlType::Integer)
            << Column("isIndexed", SqlType::Boolean)
            << Column("nameValueList", SqlType(SqlType::VarChar, 32));
    initializeAnnotationStructure.add(new CreateTable(tableAnnotationAttributes));

    return SQLSerialiserBase::applyMigration(
                QString("%1_initializeAnnotationStructure").arg(QDateTime::currentDateTimeUtc().toString()),
                &initializeAnnotationStructure, db);
}

// static
bool SQLSerialiserAnnotationStructure::loadAnnotationStructure(QPointer<AnnotationStructure> structure, QSqlDatabase &db)
{
    if (!structure) return false;
    QSqlQuery q1(db), q2(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM praalineAnnotationLevels");
    q2.setForwardOnly(true);
    q2.prepare("SELECT * FROM praalineAnnotationAttributes WHERE levelID = :levelID");
    //
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
    while (q1.next()) {
        AnnotationStructureLevel *level = new AnnotationStructureLevel();
        level->setID(q1.value("levelID").toString());
        switch (q1.value("levelType").toInt()) {
        case 0:     level->setLevelType(AnnotationStructureLevel::IndependentLevel);    break;
        case 1:     level->setLevelType(AnnotationStructureLevel::GroupingLevel);       break;
        case 2:     level->setLevelType(AnnotationStructureLevel::SequencesLevel);      break;
        case 3:     level->setLevelType(AnnotationStructureLevel::TreeLevel);           break;
        default:    level->setLevelType(AnnotationStructureLevel::IndependentLevel);    break;
        }
        level->setParentLevelID(q1.value("parentLevelID").toString());
        level->setName(q1.value("name").toString());
        level->setDescription(q1.value("description").toString());
        level->setDatatype(DataType(q1.value("datatype").toString()));
        level->setDatatype(DataType(level->datatype().base(), q1.value("length").toInt()));
        if (q1.value("isIndexed").toInt() > 0) level->setIndexed(true); else level->setIndexed(false);
        level->setNameValueList(q1.value("nameValueList").toString());
        q2.bindValue(":levelID", level->ID());
        q2.exec();
        while (q2.next()) {
            AnnotationStructureAttribute *attribute = new AnnotationStructureAttribute();
            attribute->setID(q2.value("attributeID").toString());
            attribute->setName(q2.value("name").toString());
            attribute->setDescription(q2.value("description").toString());
            attribute->setDatatype(DataType(q2.value("datatype").toString()));
            attribute->setDatatype(DataType(attribute->datatype().base(), q2.value("length").toInt()));
            if (q2.value("isIndexed").toInt() > 0) attribute->setIndexed(true); else attribute->setIndexed(false);
            attribute->setNameValueList(q2.value("nameValueList").toString());
            attribute->setParent(level);
            level->addAttribute(attribute);
        }
        level->setParent(structure);
        structure->addLevel(level);
    }
    return true;
}

// static
bool SQLSerialiserAnnotationStructure::createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel, QSqlDatabase &db)
{
    Migrations::Migration createLevel;
    QString tableName = newLevel->ID();
    ColumnList columns;
    columns << Column("annotationID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
               Column("speakerID", SqlType(SqlType::VarChar, 64), "", Column::Primary);
    if ((newLevel->levelType() == AnnotationStructureLevel::IndependentLevel) ||
        (newLevel->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        columns << Column("intervalNo", SqlType(SqlType::Integer, 11), "", Column::Primary) <<
                   Column("tMin", SqlType(SqlType::BigInt, 20)) <<
                   Column("tMax", SqlType(SqlType::BigInt, 20)) <<
                   Column("xText", SqlType(SqlType::VarChar, 1024));
    }
    else if (newLevel->levelType() == AnnotationStructureLevel::SequencesLevel) {
        columns << Column("tMin", SqlType(SqlType::BigInt, 20), "", Column::Primary) <<
                   Column("tMax", SqlType(SqlType::BigInt, 20), "", Column::Primary) <<
                   Column("xText", SqlType(SqlType::VarChar, 1024));
    }
    else if (newLevel->levelType() == AnnotationStructureLevel::TreeLevel) {
        columns << Column("intervalNoLeft", SqlType(SqlType::Integer, 11), "", Column::Primary) <<
                   Column("intervalNoRight", SqlType(SqlType::Integer, 11), "", Column::Primary) <<
                   Column("xText", SqlType(SqlType::VarChar, 1024));
    }
    else {
        return false;
    }
    foreach(AnnotationStructureAttribute *attribute, newLevel->attributes()) {
        columns << Column(attribute->ID(), SqlType(attribute->datatype()));
    }
    Table::Builder table(tableName, columns);
    createLevel.add(new CreateTable(table));
    bool result = SQLSerialiserBase::applyMigration(
                QString("%1_createlevel_%2").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName),
                &createLevel, db);
    if (result) {
        QSqlQuery q1(db), q2(db), qdel(db);
        q1.prepare("INSERT INTO praalineAnnotationLevels (levelID, levelType, parentLevelID, name, description, datatype, length, isIndexed, nameValueList) "
                   "VALUES (:levelID, :levelType, :parentLevelID, :name, :description, :datatype, :length, :isIndexed, :nameValueList)");
        q2.prepare("INSERT INTO praalineAnnotationAttributes (attributeID, levelID, name, description, datatype, length, isIndexed, nameValueList) "
                   "VALUES (:attributeID, :levelID, :name, :description, :datatype, :length, :isIndexed, :nameValueList)");
        qdel.prepare("DELETE FROM praalineAnnotationLevels WHERE levelID = :levelID");
        qdel.bindValue(":levelID", newLevel->ID());
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        qdel.prepare("DELETE FROM praalineAnnotationAttributes WHERE levelID = :levelID");
        qdel.bindValue(":levelID", newLevel->ID());
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        q1.bindValue(":levelID", newLevel->ID());
        q1.bindValue(":levelType", newLevel->levelType());
        q1.bindValue(":parentLevelID", newLevel->parentLevelID());
        q1.bindValue(":name", newLevel->name());
        q1.bindValue(":description", newLevel->description());
        q1.bindValue(":datatype", newLevel->datatypeString());
        q1.bindValue(":length", newLevel->datatypePrecision());
        q1.bindValue(":isIndexed", (newLevel->indexed()) ? 1 : 0);
        q1.bindValue(":nameValueList", newLevel->nameValueList());
        q1.exec();
        if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
        foreach(AnnotationStructureAttribute *attribute, newLevel->attributes()) {
            q2.bindValue(":attributeID", attribute->ID());
            q2.bindValue(":levelID", newLevel->ID());
            q2.bindValue(":name", attribute->name());
            q2.bindValue(":description", attribute->description());
            q2.bindValue(":datatype", attribute->datatypeString());
            q2.bindValue(":length", attribute->datatypePrecision());
            q2.bindValue(":isIndexed", (attribute->indexed()) ? 1 : 0);
            q2.bindValue(":nameValueList", attribute->nameValueList());
            q2.exec();
            if (q2.lastError().isValid()) { qDebug() << q2.lastError(); return false; }
        }
        qDebug() << "Sucessfully created annotation level " << newLevel->ID();
        return true;
    }
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::updateAnnotationLevel(QPointer<AnnotationStructureLevel> updatedLevel, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::renameAnnotationLevel(const QString &levelID, const QString &newLevelID, QSqlDatabase &db)
{
    bool result = renameTable(levelID, newLevelID, db);
    if (result) {
        QSqlQuery q(db);
        q.prepare("UPDATE praalineAnnotationLevels SET levelID = :newLevelID WHERE levelID = :levelID");
        q.bindValue(":levelID", levelID);
        q.bindValue(":newLevelID", newLevelID);
        q.exec();
        if (q.lastError().isValid()) { return false; }
        q.prepare("UPDATE praalineAnnotationAttributes SET levelID = :newLevelID WHERE levelID = :levelID");
        q.bindValue(":levelID", levelID);
        q.bindValue(":newLevelID", newLevelID);
        q.exec();
        if (q.lastError().isValid()) { return false; }
        return true;
    }
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::deleteAnnotationLevel(const QString &levelID, QSqlDatabase &db)
{
    bool result = deleteTable(levelID, db);
    if (result) {
        QSqlQuery qdel(db);
        qdel.prepare("DELETE FROM praalineAnnotationAttributes WHERE levelID = :levelID");
        qdel.bindValue(":levelID", levelID);
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        qdel.prepare("DELETE FROM praalineAnnotationLevels WHERE levelID = :levelID");
        qdel.bindValue(":levelID", levelID);
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        return true;
    }
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::createAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> newAttribute,
                                                                 QSqlDatabase &db)
{
    if (levelID.isEmpty()) return false;
    if (!newAttribute) return false;
    bool result = addColumnToTable(levelID, newAttribute->ID(), newAttribute->datatype(), db);
    if (result) {
        QSqlQuery q(db), qdel(db);
        q.prepare("INSERT INTO praalineAnnotationAttributes (attributeID, levelID, name, description, datatype, length, isIndexed, nameValueList) "
                  "VALUES "
                  "(:attributeID, :levelID, :name, :description, :datatype, :length, :isIndexed, :nameValueList)");
        qdel.prepare("DELETE FROM praalineAnnotationAttributes WHERE levelID = :levelID AND attributeID = :attributeID");
        qdel.bindValue(":levelID", levelID);
        qdel.bindValue(":attributeID", newAttribute->ID());
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        q.bindValue(":attributeID", newAttribute->ID());
        q.bindValue(":levelID", levelID);
        q.bindValue(":name", newAttribute->name());
        q.bindValue(":description", newAttribute->description());
        q.bindValue(":datatype", newAttribute->datatypeString());
        q.bindValue(":length", newAttribute->datatypePrecision());
        q.bindValue(":isIndexed", (newAttribute->indexed()) ? 1 : 0);
        q.bindValue(":nameValueList", newAttribute->nameValueList());
        q.exec();
        if (q.lastError().isValid()) { qDebug() << q.lastError(); return false; }
        return true;
    }
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::updateAnnotationAttribute(const QString &levelID, QPointer<AnnotationStructureAttribute> updatedAttribute,
                                                                 QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::renameAnnotationAttribute(const QString &levelID, const QString &attributeID, const QString &newAttributeID, QSqlDatabase &db)
{
    bool result = renameColumn(levelID, attributeID, newAttributeID, db);
    if (result) {
        QSqlQuery q(db);
        q.prepare("UPDATE praalineAnnotationAttributes SET attributeID = :newAttributeID WHERE levelID = :levelID AND attributeID = :attributeID");
        q.bindValue(":levelID", levelID);
        q.bindValue(":attributeID", attributeID);
        q.bindValue(":newAttributeID", newAttributeID);
        q.exec();
        if (q.lastError().isValid()) { return false; }
        return true;
    }
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::retypeAnnotationAttribute(const QString &levelID, const QString &attributeID, const DataType &newDataType, QSqlDatabase &db)
{
    return false;
}

// static
bool SQLSerialiserAnnotationStructure::deleteAnnotationAttribute(const QString &levelID, const QString &attributeID, QSqlDatabase &db)
{
    bool result = deleteColumn(levelID, attributeID, db);
    if (result) {
        QSqlQuery qdel(db);
        qdel.prepare("DELETE FROM praalineAnnotationAttributes WHERE levelID = :levelID AND attributeID = :attributeID");
        qdel.bindValue(":levelID", levelID);
        qdel.bindValue(":attributeID", attributeID);
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        return true;
    }
    return false;
}

} // namespace Core
} // namespace Praaline
