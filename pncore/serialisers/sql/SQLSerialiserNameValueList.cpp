#include <QDebug>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <QPointer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "structure/NameValueList.h"
#include "SQLSerialiserNameValueList.h"

#include "QSqlMigrator/QSqlMigrator.h"
using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

namespace Praaline {
namespace Core {

// static
bool SQLSerialiserNameValueList::initialiseNameValueListSchema(QSqlDatabase &db)
{
    if (db.tables().contains("praalineNameValueLists")) return true;
    Migrations::Migration initialiseNameValueListSchema;
    Table::Builder tableNameValueLists("praalineNameValueLists");
    tableNameValueLists
            << Column("listID", SqlType(SqlType::VarChar, 64), "", Column::Primary)
            << Column("listType", SqlType::Integer, "", Column::Primary)
            << Column("name", SqlType(SqlType::VarChar, 255))
            << Column("description", SqlType(SqlType::VarChar, 1024))
            << Column("datatype", SqlType(SqlType::VarChar, 32))
            << Column("datatypePrecision", SqlType::Integer);
    initialiseNameValueListSchema.add(new CreateTable(tableNameValueLists));
    bool result = SQLSerialiserBase::applyMigration("initialiseNameValueListSchema", &initialiseNameValueListSchema, db);
    if (result) setPraalineSchemaVersion(3, db);
    return result;
}

// static
bool SQLSerialiserNameValueList::upgradeNameValueListSchema(QSqlDatabase &db)
{
    int schemaVersion = getPraalineSchemaVersion(db);
    if (schemaVersion < 2) {
        return initialiseNameValueListSchema(db);
    }
    return true;
}

// static
NameValueList *SQLSerialiserNameValueList::getNameValueList(const QString &listID, NameValueListType listType, QSqlDatabase &db)
{
    QString tableName = getTableName(listID, listType);
    if (!db.tables().contains(tableName)) return Q_NULLPTR;
    // Select information from internal table, then select data
    QSqlQuery q1(db), q2(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM praalineNameValueLists WHERE listID = :listID AND listType = :listType");
    q1.bindValue(":listID", listID);
    q1.bindValue(":listType", listType);
    q2.setForwardOnly(true);
    q2.prepare(QString("SELECT * FROM %1 ORDER BY itemOrder").arg(tableName));
    // Run queries
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return Q_NULLPTR; }
    while (q1.next()) {
        NameValueList *list = new NameValueList();
        list->setID(q1.value("listID").toString());
        list->setName(q1.value("name").toString());
        list->setDescription(q1.value("description").toString());
        list->setDatatype(DataType(q1.value("datatype").toString()));
        list->setDatatype(DataType(list->datatype().base(), q1.value("datatypePrecision").toInt()));
        q2.exec();
        if (q2.lastError().isValid()) { qDebug() << q2.lastError(); delete list; return Q_NULLPTR; }
        while (q2.next()) {
            list->append(q2.value("displayText").toString(), q2.value("value"));
        }
        return list;
    }
    return 0;
}

// static
QStringList SQLSerialiserNameValueList::getAllNameValueListIDs(NameValueListType listType, QSqlDatabase &db)
{
    QStringList listIDs;
    QSqlQuery q1(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT listID FROM praalineNameValueLists WHERE listType = :listType");
    q1.bindValue(":listType", listType);
    // Run queries
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return listIDs; }
    while (q1.next()) {
        listIDs << q1.value("listID").toString();
    }
    return listIDs;
}

// static
QMap<QString, QPointer<NameValueList> > SQLSerialiserNameValueList::getAllNameValueLists(NameValueListType listType, QSqlDatabase &db)
{
    QMap<QString, QPointer<NameValueList> > lists;
    // Select information from internal table, then select data
    QSqlQuery q1(db);
    q1.setForwardOnly(true);
    q1.prepare("SELECT * FROM praalineNameValueLists WHERE listType = :listType");
    q1.bindValue(":listType", listType);
    // Run queries
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); return lists; }
    while (q1.next()) {
        NameValueList *list = new NameValueList();
        list->setID(q1.value("listD").toString());
        list->setName(q1.value("name").toString());
        list->setDescription(q1.value("description").toString());
        list->setDatatype(DataType(q1.value("datatype").toString()));
        list->setDatatype(DataType(list->datatype().base(), q1.value("datatypePrecision").toInt()));
        QSqlQuery q2(db);
        q2.setForwardOnly(true);
        q2.prepare(QString("SELECT * FROM %1 ORDER BY itemOrder").arg(getTableName(list->ID(), listType)));
        q2.exec();
        if (q2.lastError().isValid()) {
            qDebug() << q2.lastError(); delete list;
        } else {
            while (q2.next()) {
                list->append(q2.value("displayText").toString(), q2.value("ID"));
            }
            lists.insert(list->ID(), list);
        }
    }
    return lists;
}

// static
bool SQLSerialiserNameValueList::createNameValueList(NameValueList *newList, NameValueListType listType, QSqlDatabase &db)
{
    QString tableName = getTableName(newList->ID(), listType);
    Migrations::Migration createList;
    ColumnList columns;
    // Name value lists have an ID (primary key), a unique description and an item order
    columns << Column("value", SqlType(newList->datatype()), "", Column::Primary)
            << Column("displayText", SqlType(SqlType::VarChar, 256), "", Column::Unique)
            << Column("itemOrder", SqlType::Integer);
    // Build a database table corresponding to the list
    Table::Builder table(tableName, columns);
    createList.add(new CreateTable(table));
    // Execute SQL commands to build the list table
    bool result = SQLSerialiserBase::applyMigration(
                QString("%1_createlist_%2").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName),
                &createList, db);
    if (!result) { return false; }
    db.transaction();
    QSqlQuery q1(db), q2(db), qdel(db);
    q1.prepare("INSERT INTO praalineNameValueLists (listID, listType, name, description, datatype, datatypePrecision) "
               "VALUES (:listID, :listType, :name, :description, :datatype, :datatypePrecision)");
    q2.prepare(QString("INSERT INTO %1 (value, displayText, itemOrder) VALUES (:value, :displayText, :itemOrder)").arg(tableName));
    qdel.prepare("DELETE FROM praalineNameValueLists WHERE listID = :listID");
    qdel.bindValue(":listID", newList->ID());
    qdel.exec();
    if (qdel.lastError().isValid()) { qDebug() << q1.lastError(); db.rollback(); return false; }
    q1.bindValue(":listID", newList->ID());
    q1.bindValue(":listType", listType);
    q1.bindValue(":name", newList->name());
    q1.bindValue(":description", newList->description());
    q1.bindValue(":datatype", newList->datatypeString());
    q1.bindValue(":datatypePrecision", newList->datatypePrecision());
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); db.rollback(); return false; }
    for (int i = 0; i < newList->count(); ++i) {
        q2.bindValue(":value", newList->value(i));
        q2.bindValue(":displayText", newList->displayString(i));
        q2.bindValue(":itemOrder", i + 1);
        q2.exec();
        if (q2.lastError().isValid()) { qDebug() << q2.lastError(); db.rollback(); return false; }
    }
    db.commit();
    qDebug() << "Sucessfully created name-value list " << newList->ID();
    return true;
}

// static
bool SQLSerialiserNameValueList::updateNameValueList(NameValueList *list, NameValueListType listType, QSqlDatabase &db)
{
    QString tableName = getTableName(list->ID(), listType);
    if (!checkNameValueListExists(list->ID(), listType, db))
        return createNameValueList(list, listType, db);
    // Update internal tables, then data
    db.transaction();
    QSqlQuery q1(db), q2(db), qdel(db);
    q1.prepare("UPDATE praalineNameValueLists SET name = :name, description = :description "
               "WHERE listID = :listID AND listType = :listType");
    q1.bindValue(":listID", list->ID());
    q1.bindValue(":listType", listType);
    q1.bindValue(":name", list->name());
    q1.bindValue(":description", list->description());
    q1.exec();
    if (q1.lastError().isValid()) { qDebug() << q1.lastError(); db.rollback(); return false; }
    qdel.prepare(QString("DELETE FROM %1").arg(tableName));
    qdel.exec();
    if (qdel.lastError().isValid()) { qDebug() << q1.lastError(); db.rollback(); return false; }
    q2.prepare(QString("INSERT INTO %1 (value, displayText, itemOrder) VALUES (:value, :displayText, :itemOrder)").arg(tableName));
    for (int i = 0; i < list->count(); ++i) {
        q2.bindValue(":value", list->value(i));
        q2.bindValue(":displayText", list->displayString(i));
        q2.bindValue(":itemOrder", i + 1);
        q2.exec();
        if (q2.lastError().isValid()) { qDebug() << q2.lastError(); db.rollback(); return false; }
    }
    db.commit();
    qDebug() << "Sucessfully created name-value list " << list->ID();
    return true;
}

// static
bool SQLSerialiserNameValueList::deleteNameValueList(const QString &listID, NameValueListType listType, QSqlDatabase &db)
{
    QString tableName = getTableName(listID, listType);
    if (!db.tables().contains(tableName)) return false;
    bool result = deleteTable(tableName, db);
    if (result) {
        QSqlQuery qdel(db);
        qdel.prepare("DELETE FROM praalineNameValueLists WHERE listID = :listID");
        qdel.bindValue(":listID", listID);
        qdel.exec();
        if (qdel.lastError().isValid()) { return false; }
        return true;
    }
    return false;
}

// static
bool SQLSerialiserNameValueList::checkNameValueListExists(const QString &listID, NameValueListType listType, QSqlDatabase &db)
{
    QString tableName = getTableName(listID, listType);
    if (db.tables().contains(tableName)) return true;
    // else
    return false;
}

// static private
QString SQLSerialiserNameValueList::getTableName(const QString &listID, NameValueListType listType)
{
    if (listType == SQLSerialiserNameValueList::Metadata)
        return QString("nvlm_%1").arg(listID);
    // else
    return QString("nvla_%1").arg(listID);
}

} // namespace Core
} // namespace Praaline
