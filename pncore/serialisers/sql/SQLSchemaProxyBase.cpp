#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>

#include "QSqlMigrator/QSqlMigrator.h"
using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

#include "base/DataType.h"
#include "SQLSerialiserBase.h"
#include "SQLSchemaProxyBase.h"

namespace Praaline {
namespace Core {

bool SQLSchemaProxyBase::addColumnToTable(QString tableName, QString columnName,
                                          DataType datatype, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new AddColumn(Column(columnName, SqlType(datatype)), tableName));
    return SQLSerialiserBase::applyMigration(
                QString("%1_addcolumn_%2_%3").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName).arg(columnName),
                &migration, db);
}

bool SQLSchemaProxyBase::renameColumn(QString tableName, QString oldName, QString newName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new RenameColumn(oldName, newName, tableName));
    return SQLSerialiserBase::applyMigration(
                QString("%1_renamecolumn_%2_%3_%4").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName).arg(oldName).arg(newName),
                &migration, db);
}

bool SQLSchemaProxyBase::retypeColumn(QString tableName, QString columnName,
                                      DataType oldDataType, DataType newDataType, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new AlterColumnType(columnName, tableName, SqlType(newDataType), SqlType(oldDataType)));
    return SQLSerialiserBase::applyMigration(
                QString("%1_retypecolumn_%2_%3").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName).arg(columnName),
                &migration, db);
}

bool SQLSchemaProxyBase::renameTable(QString oldTableName, QString newTableName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new RenameTable(oldTableName, newTableName));
    return SQLSerialiserBase::applyMigration(
                QString("%1_renametable_%2_%3").arg(QDateTime::currentDateTimeUtc().toString()).arg(oldTableName).arg(newTableName),
                &migration, db);
}

bool SQLSchemaProxyBase::deleteColumn(QString tableName, QString columnName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new DropColumn(columnName, tableName));
    return SQLSerialiserBase::applyMigration(
                QString("%1_dropcolumn_%2_%3").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName).arg(columnName),
                &migration, db);
}

bool SQLSchemaProxyBase::deleteTable(QString tableName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new DropTable(tableName));
    return SQLSerialiserBase::applyMigration(
                QString("%1_droptable_%2").arg(QDateTime::currentDateTimeUtc().toString()).arg(tableName),
                &migration, db);
}

} // namespace Core
} // namespace Praaline
