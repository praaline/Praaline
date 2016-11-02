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
#ifndef QSQLMIGRATOR_API_H
#define QSQLMIGRATOR_API_H

#include "QSqlMigrator/QSqlMigratorConfig.h"

#include "QSqlMigrator/Commands/AddColumn.h"
#include "QSqlMigrator/Commands/AlterColumnType.h"
#include "QSqlMigrator/Commands/BaseCommand.h"
#include "QSqlMigrator/Commands/CreateIndex.h"
#include "QSqlMigrator/Commands/CreateTable.h"
#include "QSqlMigrator/Commands/DropColumn.h"
#include "QSqlMigrator/Commands/DropIndex.h"
#include "QSqlMigrator/Commands/DropTable.h"
#include "QSqlMigrator/Commands/RenameColumn.h"
#include "QSqlMigrator/Commands/RenameTable.h"

#include "QSqlMigrator/Migrations/RegisterMigration.h"

#include "QSqlMigrator/Structure/Column.h"
#include "QSqlMigrator/Structure/Index.h"
#include "QSqlMigrator/Structure/Table.h"

#endif // QSQLMIGRATOR_API_H
