#include "LocalSchemeMigrator.h"

#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeAddColumnService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeAlterColumnTypeService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeCreateIndexService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeCreateTableService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropColumnService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropIndexService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeDropTableService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeRenameColumnService.h"
#include "QSqlMigrator/LocalSchemeMigrator/CommandExecution/LocalSchemeRenameTableService.h"

#include "QSqlMigrator/CommandExecution/LocalSchemeCommandExecutionServiceRepository.h"

namespace QSqlMigrator {
namespace LocalSchemeMigrator {

QSharedPointer<CommandExecution::LocalSchemeCommandExecutionServiceRepository> createCommandServiceRepository()
{
    using namespace CommandExecution;

    QSharedPointer<LocalSchemeCommandExecutionServiceRepository> commandRepository(new LocalSchemeCommandExecutionServiceRepository);
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeAddColumnService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeAlterColumnTypeService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeCreateIndexService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeCreateTableService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeDropColumnService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeDropIndexService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeDropTableService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeRenameColumnService));
    commandRepository->add(LocalSchemeBaseCommandServicePtr(new LocalSchemeRenameTableService));

    return commandRepository;
}

} // namespace LocalSchemeMigrator
} // namespace QSqlMigrator
