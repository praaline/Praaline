#include <QDebug>
#include <QDateTime>
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
    MigrationExecution::MigrationExecutionContextPtr context(0);
    if (db.driverName() == "QSQLITE")
        context = SqliteMigrator::buildContext(contextBuilder);
    else if (db.driverName() == "QMYSQL")
        context = MysqlMigrator::buildContext(contextBuilder);
    else if (db.driverName() == "QPSQL")
        context = PostgresqlMigrator::buildContext(contextBuilder);
    if (!context) return false;
    QSqlMigrator::QSqlMigratorService manager;
    bool result = manager.applyAll(*context);
    return result;
}

// static
int SQLSerialiserBase::getPraalineSchemaVersion(QSqlDatabase &db)
{
    if (!db.tables().contains("praalineSystem")) return 0;
    QSqlQuery q(db);  
    q.prepare("SELECT id, value FROM praalineSystem WHERE id=:id");
    q.bindValue(":id", "schema_version");
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
                << Column("id", SqlType(SqlType::VarChar, 64), "", Column::Primary)
                << Column("value", SqlType(SqlType::VarChar, 4096));
        initialisePraalineSystem.add(new CreateTable(tablePraalineSystem));
        bool result = applyMigration("initialisePraalineSystem", &initialisePraalineSystem, db);
        if (!result) return false;
    }
    QSqlQuery q(db);
    db.transaction();
    q.prepare("DELETE FROM praalineSystem WHERE id=:id");
    q.bindValue(":id", "schema_version");
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); db.rollback(); return false; }
    q.prepare("INSERT INTO praalineSystem(id, value) VALUES (:id, :value)");
    q.bindValue(":id", "schema_version");
    q.bindValue(":value", version);
    q.exec();
    if (q.lastError().isValid()) { qDebug() << q.lastError(); db.rollback(); return false; }
    db.commit();
    return true;
}

// static
bool SQLSerialiserBase::upgradeSchema(QSqlDatabase &db)
{
    bool upgrade(false);
    QSqlQuery q(db);
    q.prepare("SELECT key, value from praalineSystem");
    q.exec();
    while (q.next()) upgrade = true;
    if (upgrade) {
        // Upgrade: previous schemata had a column "key" instead of "id"
        Migrations::Migration upgradePraalineSystem;
        upgradePraalineSystem.add(new RenameColumn("key", "id", "praalineSystem"));
        return SQLSerialiserBase::applyMigration(
                    QString("%1_upgradePraalineSystem").arg(QDateTime::currentDateTimeUtc().toString()),
                    &upgradePraalineSystem, db);
    }
    return true;
}


} // namespace Core
} // namespace Praaline
