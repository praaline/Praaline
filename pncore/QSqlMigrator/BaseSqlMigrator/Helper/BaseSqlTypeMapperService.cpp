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
#include "BaseSqlTypeMapperService.h"

#include <QDebug>

namespace QSqlMigrator {
namespace Helper {

BaseSqlTypeMapperService::BaseSqlTypeMapperService()
{
    using namespace Structure;

    m_typeMap.insert(SqlType::Char,            "CHAR(%1)");
    m_typeMap.insert(SqlType::VarChar,         "VARCHAR(%1)");
    m_typeMap.insert(SqlType::Binary,          "BINARY(%1)");
    m_typeMap.insert(SqlType::VarBinary,       "VARBINARY(%1)");
    m_typeMap.insert(SqlType::Boolean,         "BOOLEAN");
    m_typeMap.insert(SqlType::SmallInt,        "SMALLINT");
    m_typeMap.insert(SqlType::Integer,         "INTEGER(%1)");
    m_typeMap.insert(SqlType::BigInt,          "BIGINT");
    m_typeMap.insert(SqlType::Decimal,         "DECIMAL(%1,%2)");
    m_typeMap.insert(SqlType::Float,           "FLOAT");
    m_typeMap.insert(SqlType::DoublePrecision, "DOUBLE PRECISION");
    m_typeMap.insert(SqlType::Date,            "DATE");
    m_typeMap.insert(SqlType::Time,            "TIME");
    m_typeMap.insert(SqlType::Timestamp,       "TIMESTAMP");
    m_typeMap.insert(SqlType::Interval,        "INTERVAL");
    //    typeMap.insert(Type::Array,           "ARRAY(%1)");
    //    typeMap.insert(Type::Multiset,        "MULTISET");
    //    typeMap.insert(Type::Xml,             "XML");
}

QString BaseSqlTypeMapperService::map(const Structure::SqlType &type) const
{
    if( type.isString() )
        return type.string();

    using namespace Structure;

    switch (type.base())
    {
    case SqlType::Char:
        return m_typeMap[SqlType::Char].arg(type.precision(1));

    case SqlType::Integer:
        return m_typeMap[SqlType::Integer].arg(type.precision(10));

    case SqlType::Decimal:
        return m_typeMap[SqlType::Decimal].arg(type.precision()).arg(type.scale());

    case SqlType::VarChar:
    case SqlType::Binary:
    case SqlType::VarBinary:
        return m_typeMap[type.base()].arg(type.precision());

    default:
        if (!m_typeMap.contains(type.base()))
            ::qWarning() << "unknown type";
        return m_typeMap[type.base()];
    }
}

} // namespace Helper
} // namespace QSqlMigrator

