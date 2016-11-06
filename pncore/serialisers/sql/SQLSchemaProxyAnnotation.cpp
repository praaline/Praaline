#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "structure/AnnotationStructure.h"
#include "structure/AnnotationStructureLevel.h"
#include "structure/AnnotationStructureAttribute.h"

#include "QSqlMigrator/QSqlMigrator.h"
using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

#include "SQLSerialiserBase.h"
#include "SQLSchemaProxyAnnotation.h"

namespace Praaline {
namespace Core {

SQLSchemaProxyAnnotation::SQLSchemaProxyAnnotation()
{
}

bool createNewSchema(AnnotationStructureLevel *level, QSqlDatabase &db)
{
    Migrations::Migration createLevel;
    QString tableName = level->ID();
    ColumnList columns;
    columns << Column("annotationID", SqlType(SqlType::VarChar, 64), "", Column::Primary) <<
               Column("speakerID", SqlType(SqlType::VarChar, 64), "", Column::Primary);
    if ((level->levelType() == AnnotationStructureLevel::IndependentLevel) ||
        (level->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        columns << Column("intervalNo", SqlType(SqlType::Integer, 11), "", Column::Primary) <<
                   Column("tMin", SqlType(SqlType::BigInt, 20)) <<
                   Column("tMax", SqlType(SqlType::BigInt, 20)) <<
                   Column("xText", SqlType(SqlType::VarChar, 1024));
    }
    else if (level->levelType() == AnnotationStructureLevel::SequencesLevel) {
        columns << Column("tMin", SqlType(SqlType::BigInt, 20), "", Column::Primary) <<
                   Column("tMax", SqlType(SqlType::BigInt, 20), "", Column::Primary) <<
                   Column("xText", SqlType(SqlType::VarChar, 1024));
    }
    else if (level->levelType() == AnnotationStructureLevel::TreeLevel) {
        columns << Column("intervalNoLeft", SqlType(SqlType::Integer, 11), "", Column::Primary) <<
                   Column("intervalNoRight", SqlType(SqlType::Integer, 11), "", Column::Primary) <<
                   Column("xText", SqlType(SqlType::VarChar, 1024));
    }
    foreach(AnnotationStructureAttribute *attribute, level->attributes()) {
        columns << Column(attribute->ID(), SqlType(attribute->datatype()));
    }

    Table::Builder table(tableName, columns);
    createLevel.add(new CreateTable(table));
    return SQLSerialiserBase::applyMigration(QString("createlevel_%1").arg(tableName),
                                             &createLevel, db);
}

// static
bool SQLSchemaProxyAnnotation::createSchema(AnnotationStructure *structure, QSqlDatabase &db)
{
    db.transaction();
    foreach (AnnotationStructureLevel *level, structure->levels()) {
        if (!createNewSchema(level, db)) { db.rollback(); return false; }
    }
    db.commit();
    return true;
}

// static
bool SQLSchemaProxyAnnotation::createAnnotationLevel(QPointer<AnnotationStructureLevel> newLevel, QSqlDatabase &db)
{
    db.transaction();
    if (!createNewSchema(newLevel, db)) { db.rollback(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSchemaProxyAnnotation::renameAnnotationLevel(QString levelID, QString newLevelID, QSqlDatabase &db)
{
    return renameTable(levelID, newLevelID, db);
}

// static
bool SQLSchemaProxyAnnotation::deleteAnnotationLevel(QString levelID, QSqlDatabase &db)
{
    return deleteTable(levelID, db);
}

// static
bool SQLSchemaProxyAnnotation::createAnnotationAttribute(QString levelID, QPointer<AnnotationStructureAttribute> newAttribute, QSqlDatabase &db)
{
    if (!newAttribute) return false;
    return addColumnToTable(levelID, newAttribute->ID(), newAttribute->datatype(), db);
    return false;
}

// static
bool SQLSchemaProxyAnnotation::renameAnnotationAttribute(QString levelID, QString attributeID, QString newAttributeID, QSqlDatabase &db)
{
    return renameColumn(levelID, attributeID, newAttributeID, db);
}

// static
bool SQLSchemaProxyAnnotation::deleteAnnotationAttribute(QString levelID, QString attributeID, QSqlDatabase &db)
{
    return deleteColumn(levelID, attributeID, db);
}

// static
bool SQLSchemaProxyAnnotation::retypeAnnotationAttribute(QString levelID, QString attributeID,
                                                         QString newDatatype, int newDatalength, QSqlDatabase &db)
{
    return true;
}

} // namespace Core
} // namespace Praaline
