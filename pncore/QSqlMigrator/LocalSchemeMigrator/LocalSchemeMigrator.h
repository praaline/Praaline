#ifndef LOCALSCHEMEMIGRATOR_LOCALSCHEMEMIGRATOR_H
#define LOCALSCHEMEMIGRATOR_LOCALSCHEMEMIGRATOR_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

#include "QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionServiceRepository.h"

#include <QSharedPointer>

namespace QSqlMigrator {
namespace LocalSchemeMigrator {

QSharedPointer<CommandExecution::LocalSchemeCommandExecutionServiceRepository> QSQLMIGRATOR_DLL_EXPORT createCommandServiceRepository();

} // namespace LocalSchemeMigrator
} // namespace QSqlMigrator

#endif // LOCALSCHEMEMIGRATOR_H
