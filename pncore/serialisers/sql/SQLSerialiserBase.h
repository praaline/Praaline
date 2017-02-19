#ifndef SQLSERIALISERBASE_H
#define SQLSERIALISERBASE_H

#include <QString>
#include <QSqlDatabase>

namespace QSqlMigrator {
namespace Migrations {
class Migration;
}
}

namespace Praaline {
namespace Core {

struct SQLSerialiserBaseData;

class SQLSerialiserBase
{
public:
    enum DatabaseSystem {
        Sqlite,
        MySQL,
        Postgres
    };

    SQLSerialiserBase();
    ~SQLSerialiserBase();

    static bool applyMigration(const QString &migrationName, const QSqlMigrator::Migrations::Migration *migration,
                               QSqlDatabase &db);

    static int getPraalineSchemaVersion(QSqlDatabase &db);
    static bool setPraalineSchemaVersion(int version, QSqlDatabase &db);
    static bool upgradeSchema(QSqlDatabase &db);

private:
    SQLSerialiserBaseData *d;
};

} // namespace Core
} // namespace Praaline

#endif // SQLSERIALISERBASE_H
