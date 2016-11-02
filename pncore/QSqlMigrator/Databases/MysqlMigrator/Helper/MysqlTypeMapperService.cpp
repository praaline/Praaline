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
#include "QSqlMigrator/Databases/MysqlMigrator/Helper/MysqlTypeMapperService.h"

#include "QSqlMigrator/MigrationTracker/MigrationTrackerService.h"

#include <QDebug>

namespace QSqlMigrator {
namespace Helper {

MysqlTypeMapperService::MysqlTypeMapperService()
{
    using namespace Structure;

    m_typeMap.insert(SqlType::Boolean, "TINYINT(1)");
    m_typeMap.insert(SqlType::Integer, "INT(%1)");
    m_typeMap.insert(SqlType::BigInt,  "BIGINT(%1)");
    m_typeMap.insert(SqlType::DoublePrecision,  "DOUBLE");
}

QString MysqlTypeMapperService::map(const Structure::SqlType &type) const
{
    using namespace Structure;

    switch (type.base())
    {
    case SqlType::BigInt:
        return m_typeMap[SqlType::BigInt].arg(type.precision(20));

    case SqlType::VarBinary:
    {
        const quint64 precision = type.precision();
        if (precision <= 255 && precision > 0)
            return "TINYBLOB";
        if (precision <= 65535)
            return "BLOB";
        if (precision <= 16777215)
            return "MEDIUMBLOB";
        if (precision <= 4294967295)
            return "LONGBLOB";
        ::qWarning() << "blob is too large";
        return "LONGBLOB";
    }
    case SqlType::VarChar:
    {
        const quint64 precision = type.precision();
        if (precision > 65535)
            return "TEXT";
        return BaseSqlTypeMapperService::map(type);
    }
    default:
        return BaseSqlTypeMapperService::map(type);
    }
}

} // namespace Helper
} // namespace QSqlMigrator
