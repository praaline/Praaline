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
#ifndef SQLITEMIGRATOR_H
#define SQLITEMIGRATOR_H

#include "QSqlMigrator/MigrationExecution/MigrationExecutionContext.h"

#include <Qt>

namespace QSqlMigrator {
namespace SqliteMigrator {

/*!
 * \brief Use this function to setup your execution context for sqlite migrations.
 *
 * Example:
 *
 * MigrationExecutionContext::Builder builder(...);
 *
 * QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
 * db.setDatabaseName("sample_db.sqlite3");
 * builder.setDatabase(db);
 *
 * MigrationExecutionContextPtr context( SqliteMigrator::buildContext(builder) );
 * if (context) {
 *      // do something
 * }
 *
 */
MigrationExecution::MigrationExecutionContextPtr buildContext(MigrationExecution::MigrationExecutionContext::Builder &contextBuilder);

} // namespace SqliteMigrator
} // namespace QSqlMigrator

#endif // SQLITEMIGRATOR_H
