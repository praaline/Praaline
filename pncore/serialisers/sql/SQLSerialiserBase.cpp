#include "SQLSerialiserBase.h"

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


} // namespace Core
} // namespace Praaline
