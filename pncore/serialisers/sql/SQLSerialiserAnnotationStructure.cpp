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

#include "SQLSerialiserBase.h"
#include "SQLSerialiserAnnotationStructure.h"

namespace Praaline {
namespace Core {

SQLSerialiserAnnotationStructure::SQLSerialiserAnnotationStructure()
{
}

// static
void SQLSerialiserAnnotationStructure::initialiseAnnotationStructureTables(QSqlDatabase &db)
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

    SQLSerialiserBase::applyMigration("initializeAnnotationStructure", &initializeAnnotationStructure, db);
}

bool updateDatabase(AnnotationStructure *structure, QSqlDatabase &db)
{
    QSqlQuery q1(db), q2(db), qdel(db);
    q1.prepare("INSERT INTO praalineAnnotationLevels (levelID, levelType, parentLevelID, name, description, datatype, length, isIndexed, nameValueList) VALUES "
               "(:levelID, :levelType, :parentLevelID, :name, :description, :datatype, :length, :isIndexed, :nameValueList)");
    q2.prepare("INSERT INTO praalineAnnotationAttributes (attributeID, levelID, name, description, datatype, length, isIndexed, nameValueList) VALUES "
               "(:attributeID, :levelID, :name, :description, :datatype, :length, :isIndexed, :nameValueList)");
    //
    qdel.prepare("DELETE FROM praalineAnnotationLevels");
    qdel.exec();
    if (qdel.lastError().isValid()) { return false; }
    qdel.prepare("DELETE FROM praalineAnnotationAttributes");
    qdel.exec();
    if (qdel.lastError().isValid()) { return false; }
    foreach (AnnotationStructureLevel *level, structure->levels()) {
        q1.bindValue(":levelID", level->ID());
        q1.bindValue(":levelType", level->levelType());
        q1.bindValue(":parentLevelID", level->parentLevelID());
        q1.bindValue(":name", level->name());
        q1.bindValue(":description", level->description());
        q1.bindValue(":datatype", level->datatypeString());
        q1.bindValue(":length", level->datatypePrecision());
        q1.bindValue(":isIndexed", (level->indexed()) ? 1 : 0);
        q1.bindValue(":nameValueList", level->nameValueList());
        q1.exec();
        if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
        foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
            q2.bindValue(":attributeID", attribute->ID());
            q2.bindValue(":levelID", level->ID());
            q2.bindValue(":name", attribute->name());
            q2.bindValue(":description", attribute->description());
            q2.bindValue(":datatype", attribute->datatypeString());
            q2.bindValue(":length", attribute->datatypePrecision());
            q2.bindValue(":isIndexed", (attribute->indexed()) ? 1 : 0);
            q2.bindValue(":nameValueList", attribute->nameValueList());
            q2.exec();
            if (q2.lastError().isValid()) { qDebug() << q2.lastError(); return false; }
        }
    }
    return true;
}

// static
bool SQLSerialiserAnnotationStructure::write(AnnotationStructure *structure, QSqlDatabase &db)
{
    db.transaction();
    if (!updateDatabase(structure, db)) { db.rollback(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSerialiserAnnotationStructure::read(AnnotationStructure *structure, QSqlDatabase &db)
{
    if (!structure) return false;
    structure->clear();

    QSqlQuery q1(db), q2(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM praalineAnnotationLevels");
    q2.setForwardOnly(true);
    q2.prepare("SELECT * FROM praalineAnnotationAttributes WHERE levelID = :levelID");
    //
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return false; }
    while (q1.next()) {
        // levels
        AnnotationStructureLevel *level = new AnnotationStructureLevel();
        level->setID(q1.value("levelID").toString());
        switch (q1.value("levelType").toInt()) {
        case 0: level->setLevelType(AnnotationStructureLevel::IndependentLevel); break;
        case 1: level->setLevelType(AnnotationStructureLevel::GroupingLevel); break;
        case 2: level->setLevelType(AnnotationStructureLevel::SequencesLevel); break;
        case 3: level->setLevelType(AnnotationStructureLevel::TreeLevel); break;
        default: level->setLevelType(AnnotationStructureLevel::IndependentLevel); break;
        }
        level->setName(q1.value("name").toString());
        level->setDescription(q1.value("description").toString());
        level->setParentLevelID(q1.value("parentLevelID").toString());
        level->setDatatype(DataType(q1.value("datatype").toString()));
        level->setDatatype(DataType(level->datatype().base(), q1.value("length").toInt()));
        if (q1.value("isIndexed").toInt() > 0) level->setIndexed(true); else level->setIndexed(false);
        level->setNameValueList(q1.value("nameValueList").toString());
        // attributes
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

} // namespace Core
} // namespace Praaline
