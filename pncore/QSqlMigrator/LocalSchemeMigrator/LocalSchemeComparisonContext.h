#ifndef LOCALSCHEMEMIGRATOR_LOCALSCHEMECOMPARISONCONTEXT_H
#define LOCALSCHEMEMIGRATOR_LOCALSCHEMECOMPARISONCONTEXT_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

#include "QSqlMigrator/Structure/LocalScheme.h"
#include "QSqlMigrator/Helper/HelperRepository.h"

#include <QSqlDatabase>

namespace QSqlMigrator {
namespace LocalSchemeMigrator {

class QSQLMIGRATOR_DLL_EXPORT LocalSchemeComparisonContext
{
public:
    LocalSchemeComparisonContext(const LocalSchemePtr &localScheme,
                                 const Helper::HelperRepository &helperRepository,
                                 const QSqlDatabase &database);

    const LocalSchemePtr &localScheme() const;
    const Helper::HelperRepository &helperRepository() const;
    const QSqlDatabase &database() const;

private:
    const LocalSchemePtr m_localScheme;
    const Helper::HelperRepository m_helperRepository;
    const QSqlDatabase m_database;
};

} // namespace LocalSchemeMigrator
} // namespace QSqlMigrator

#endif // LOCALSCHEMEMIGRATOR_LOCALSCHEMECOMPARISONCONTEXT_H
