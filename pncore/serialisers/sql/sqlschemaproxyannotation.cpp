#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "structure/annotationstructure.h"
#include "structure/annotationstructurelevel.h"
#include "structure/annotationstructureattribute.h"
#include "sqlschemaproxyannotation.h"

SQLSchemaProxyAnnotation::SQLSchemaProxyAnnotation()
{
}

bool createNewSchema(AnnotationStructureLevel *level, QSqlDatabase &db)
{
    QString sql;
    QSqlQuery q(db), qdel(db);
    qdel.prepare(QString("DROP TABLE IF EXISTS %1").arg(level->ID()));
    qdel.exec();
    if (qdel.lastError().isValid()) return false;
    sql = QString("CREATE TABLE %1 (").arg(level->ID());
    if ((level->levelType() == AnnotationStructureLevel::IndependentLevel) ||
        (level->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        sql.append("annotationID varchar(64) NOT NULL DEFAULT '', "
                   "speakerID varchar(64) NOT NULL DEFAULT '', "
                   "intervalNo int(11), "
                   "tMin bigint(20), "
                   "tMax bigint(20), "
                   "xText varchar(1024) ");
    }
    else if (level->levelType() == AnnotationStructureLevel::SequencesLevel) {
        sql.append("annotationID varchar(64) NOT NULL DEFAULT '', "
                   "speakerID varchar(64) NOT NULL DEFAULT '', "
                   "tMin bigint(20), "
                   "tMax bigint(20), "
                   "xText varchar(1024) ");
    }
    else if (level->levelType() == AnnotationStructureLevel::TreeLevel) {
        sql.append("annotationID varchar(64) NOT NULL DEFAULT '', "
                   "speakerID varchar(64) NOT NULL DEFAULT '', "
                   "intervalNoLeft int(11), "
                   "intervalNoRight int(11), "
                   "xText varchar(1024) ");
    }
    foreach(AnnotationStructureAttribute *attribute, level->attributes()) {
        sql.append(", ").append(attribute->ID()).append(" ").append(attribute->datatype());
        if (attribute->datalength() > 0)
            sql.append("(").append(QString::number(attribute->datalength())).append(")");
        sql.append(" ");
    }
    if ((level->levelType() == AnnotationStructureLevel::IndependentLevel) ||
        (level->levelType() == AnnotationStructureLevel::GroupingLevel)) {
        sql.append(", PRIMARY KEY (annotationID, speakerID, intervalNo) )");
    }
    else if (level->levelType() == AnnotationStructureLevel::SequencesLevel) {
        sql.append(", PRIMARY KEY (annotationID, speakerID, tMin, tMax) )");
    }
    else if (level->levelType() == AnnotationStructureLevel::TreeLevel) {
        sql.append(", PRIMARY KEY (annotationID, speakerID, intervalNoLeft, intervalNoRight) )");
    }
    q.prepare(sql);
    q.exec();
    if (q.lastError().isValid()) return false;
    return true;
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
    QString datatype = newAttribute->datatype();
    if (newAttribute->datatype() == "varchar") datatype.append(QString("(%1)").arg(newAttribute->datalength()));
    return addColumnToTable(levelID, newAttribute->ID(), datatype, db);
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


