/****************************************************************************
**
** Copyright (C) 2013, HicknHack Software
** All rights reserved.
** Contact: http://www.hicknhack-software.com/contact
**
** This file is part of the QSqlMigrator
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL3 included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/
#include "QSqlMigrator/Databases/MysqlMigrator/MysqlMigrator.h"

#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlAddColumnService.h"
#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlCreateIndexService.h"
#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlCreateTableService.h"
#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropColumnService.h"
#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropIndexService.h"
#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlDropTableService.h"
#include "QSqlMigrator/BaseSqlMigrator/CommandExecution/BaseSqlRenameTableService.h"

#include "QSqlMigrator/Databases/MysqlMigrator/CommandExecution/MysqlAlterColumnTypeService.h"
#include "QSqlMigrator/Databases/MysqlMigrator/CommandExecution/MysqlRenameColumnService.h"

#include "QSqlMigrator/Helper/HelperRepository.h"

#include "QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlColumnService.h"
#include "QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlStructureService.h"
#include "QSqlMigrator/BaseSqlMigrator/Helper/BaseSqlQuoteService.h"
#include "QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlTypeMapperService.h"

#include "QSqlMigrator/CommandExecution/CustomCommandService.h"
#include "QSqlMigrator/BaseSqlMigrator/MigrationTracker/BaseMigrationTableService.h"
#include "QSqlMigrator/MigrationExecution/MigrationExecutionContext.h"

#include <QSqlDatabase>

#include <QDebug>

namespace QSqlMigrator {
namespace MysqlMigrator {

QSharedPointer<CommandExecution::CommandExecutionServiceRepository> createCommandRepository()
{
    using namespace CommandExecution;

    QSharedPointer<CommandExecutionServiceRepository> commandRepository(new CommandExecutionServiceRepository);
    commandRepository->add(BaseCommandServicePtr(new BaseSqlAddColumnService));
    commandRepository->add(BaseCommandServicePtr(new MysqlAlterColumnTypeService));
    commandRepository->add(BaseCommandServicePtr(new BaseSqlCreateIndexService));
    commandRepository->add(BaseCommandServicePtr(new BaseSqlCreateTableService));
    commandRepository->add(BaseCommandServicePtr(new BaseSqlDropColumnService));
    commandRepository->add(BaseCommandServicePtr(new BaseSqlDropIndexService));
    commandRepository->add(BaseCommandServicePtr(new BaseSqlDropTableService));
    commandRepository->add(BaseCommandServicePtr(new MysqlRenameColumnService));
    commandRepository->add(BaseCommandServicePtr(new BaseSqlRenameTableService));
    commandRepository->add(BaseCommandServicePtr(new CustomCommandService));

    return commandRepository;
}

const Helper::HelperRepository &createHelperRepository()
{
    ::qDebug() << "creating MySQL helper aggregate";

    using namespace Helper;

    static BaseSqlQuoteService quoteService;
    static MysqlTypeMapperService typeMapperService;
    static MysqlColumnService columnService(typeMapperService);
    static MysqlStructureService dbReaderService;
    static HelperRepository helperRepository(quoteService, typeMapperService, columnService, dbReaderService);

    return helperRepository;
}

MigrationExecution::MigrationExecutionContextPtr buildContext(MigrationExecution::MigrationExecutionContext::Builder &contextBuilder)
{
    using namespace MigrationExecution;

    MigrationTableServicePtr migrationTableService(new MigrationTracker::BaseMigrationTableService());

    MigrationExecution::MigrationExecutionContextPtr context(contextBuilder.build(createCommandRepository(), createHelperRepository(), migrationTableService));

    QSqlDatabase database(context->database());
    bool success = false;
    if (database.open()) {
        success = migrationTableService->prepare(*context);
    }
    if (!success )
        context.clear();

    return context;
}

} // namespace MysqlMigrator
} // namespace QSqlMigrator
