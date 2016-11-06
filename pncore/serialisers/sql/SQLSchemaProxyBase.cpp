#include <QSqlDatabase>
#include <QSqlQuery>

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
                QString("addcolumn_%1_%2").arg(tableName).arg(columnName),
                &migration, db);
}

bool SQLSchemaProxyBase::renameColumn(QString tableName, QString oldName, QString newName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new RenameColumn(oldName, newName, tableName));
    return SQLSerialiserBase::applyMigration(
                QString("renamecolumn_%1_%2_%3").arg(tableName).arg(oldName).arg(newName),
                &migration, db);
}

bool SQLSchemaProxyBase::renameTable(QString oldTableName, QString newTableName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new RenameTable(oldTableName, newTableName));
    return SQLSerialiserBase::applyMigration(
                QString("renametable_%1_%2").arg(oldTableName).arg(newTableName),
                &migration, db);
}

bool SQLSchemaProxyBase::deleteColumn(QString tableName, QString columnName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new DropColumn(columnName, tableName));
    return SQLSerialiserBase::applyMigration(
                QString("dropcolumn_%1_%2").arg(tableName).arg(columnName),
                &migration, db);
}

bool SQLSchemaProxyBase::deleteTable(QString tableName, QSqlDatabase &db)
{
    Migrations::Migration migration;
    migration.add(new DropTable(tableName));
    return SQLSerialiserBase::applyMigration(
                QString("droptable_%1").arg(tableName),
                &migration, db);
}

} // namespace Core
} // namespace Praaline
