#ifndef LOCALSCHEMEMIGRATOR_LOCALSCHEMECOMPARISONSERVICE_H
#define LOCALSCHEMEMIGRATOR_LOCALSCHEMECOMPARISONSERVICE_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

namespace QSqlMigrator {
namespace LocalSchemeMigrator {

class LocalSchemeComparisonContext;

class QSQLMIGRATOR_DLL_EXPORT LocalSchemeComparisonService
{
public:
    LocalSchemeComparisonService();

    bool compareLocalSchemeWithDatabase(const LocalSchemeComparisonContext &context) const;
};

} // namespace LocalSchemeMigrator
} // namespace QSqlMigrator

#endif // LOCALSCHEMEMIGRATOR_LOCALSCHEMECOMPARISONSERVICE_H
