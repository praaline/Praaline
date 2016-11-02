#include "SqlType.h"

namespace QSqlMigrator {
namespace Structure {

SqlType::SqlType(SqlType::Base base, quint64 precision, int scale) :
    m_base(base),
    m_precision(precision),
    m_scale(scale)
{
}

SqlType::SqlType(const QString &string) :
    m_base(Invalid),
    m_precision(0),
    m_scale(0),
    m_string(string)
{
}

bool SqlType::isBaseType() const
{
    return m_base != Invalid;
}

bool SqlType::isString() const
{
    return m_base == Invalid && (!m_string.isEmpty());
}

bool SqlType::isValid() const
{
    return m_base != Invalid || (!m_string.isEmpty());
}

const QString &SqlType::string() const
{
    return m_string;
}

SqlType::Base SqlType::base() const {
    return m_base;
}

quint64 SqlType::precision(quint64 defaultValue) const {
    if( 0 == m_precision )
        return defaultValue;
    return m_precision;
}

int SqlType::scale() const {
    return m_scale;
}

const SqlType &SqlType::invalid()
{
    static SqlType invalid_sql_type(SqlType::Invalid);
    return invalid_sql_type;
}

} // namespace Structure
} // namespace QSqlMigrator
