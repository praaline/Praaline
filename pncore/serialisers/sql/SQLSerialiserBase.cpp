#include <QDebug>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "SQLSerialiserBase.h"

#include "QSqlMigrator/QSqlMigrator.h"
using namespace QSqlMigrator;
using namespace QSqlMigrator::Structure;
using namespace QSqlMigrator::Commands;

#include "QSqlMigrator/Migrations/MigrationRepository.h"
#include "QSqlMigrator/QSqlMigrator/QSqlMigratorService.h"

#include "QSqlMigrator/Databases/SqliteMigrator/SqliteMigrator.h"
#include "QSqlMigrator/Databases/MysqlMigrator/MysqlMigrator.h"
#include "QSqlMigrator/Databases/PostgresqlMigrator/PostgresqlMigrator.h"
#include "QSqlMigrator/Databases/FirebirdMigrator/FirebirdMigrator.h"

using namespace QSqlMigrator;
using namespace QSqlMigrator::Migrations;
using namespace QSqlMigrator::MigrationExecution;

namespace Praaline {
namespace Core {

struct SQLSerialiserBaseData {
    SQLSerialiserBase::DatabaseSystem system;
    MigrationRepository::NameMigrationMap migrations;
};

SQLSerialiserBase::SQLSerialiserBase() :
    d(new SQLSerialiserBaseData)
{
}

SQLSerialiserBase::~SQLSerialiserBase()
{
    delete d;
}

// static
bool SQLSerialiserBase::applyMigration(const QString &migrationName, const Migration *migration,
                                       QSqlDatabase &db)
{
    MigrationRepository::NameMigrationMap migrations;
    migrations.insert(migrationName, migration);
    auto contextBuilder = MigrationExecutionContext::Builder(migrations);
    contextBuilder.setDatabase(db);
    auto context = SqliteMigrator::buildContext(contextBuilder);
    QSqlMigrator::QSqlMigratorService manager;
    bool result = manager.applyAll(*context);
    return result;
}

// static
int SQLSerialiserBase::getPraalineSchemaVersion(QSqlDatabase &db)
{
    if (!db.tables().contains("praalineSystem")) return 0;
    QSqlQuery q(db);
    q.prepare("SELECT key, value FROM praalineSystem WHERE key=:key");
    q.bindValue(":key", "schema_version");
    q.exec();
    int version = 0;
    while (q.next()) {
        version = q.value("value").toInt();
    }
    return version;
}

// static
bool SQLSerialiserBase::setPraalineSchemaVersion(int version, QSqlDatabase &db)
{
    if (!db.tables().contains("praalineSystem")) {
        Migrations::Migration initialisePraalineSystem;
        Table::Builder tablePraalineSystem("praalineSystem");
        tablePraalineSystem
                << Column("key", SqlType(SqlType::VarChar, 64), "", Column::Primary)
                << Column("value", SqlType(SqlType::VarChar, 4096));
        initialisePraalineSystem.add(new CreateTable(tablePraalineSystem));
        bool result = applyMigration("initialisePraalineSystem", &initialisePraalineSystem, db);
        if (!result) return false;
    }
    QSqlQuery q(db);
    db.transaction();
    q.prepare("DELETE FROM praalineSystem WHERE key=:key");
    q.bindValue(":key", "schema_version");
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); db.rollback(); return false; }
    q.prepare("INSERT INTO praalineSystem(key, value) VALUES (:key, :value)");
    q.bindValue(":key", "schema_version");
    q.bindValue(":value", version);
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); db.rollback(); return false; }
    db.commit();
    return true;
}


} // namespace Core
} // namespace Praaline
