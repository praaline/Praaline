#include "base/DataType.h"
using namespace Praaline::Core;

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

SqlType::SqlType(const Praaline::Core::DataType& datatype) :
    m_base(Invalid),
    m_precision(datatype.precision()),
    m_scale(datatype.scale())
{
    switch (datatype.base()) {
    case DataType::Invalid:     m_base = SqlType::Invalid;  break;
    case DataType::Char:        m_base = SqlType::Char;     break;
    case DataType::VarChar:     m_base = SqlType::VarChar;  break;
    case DataType::Binary:      m_base = SqlType::Binary;   break;
    case DataType::VarBinary:   m_base = SqlType::VarBinary;break;
    case DataType::Boolean:     m_base = SqlType::Boolean;  break;
    case DataType::SmallInt:    m_base = SqlType::SmallInt; break;
    case DataType::Integer:     m_base = SqlType::Integer;  break;
    case DataType::BigInt:      m_base = SqlType::BigInt;   break;
    case DataType::Decimal:     m_base = SqlType::Decimal;  break;
    case DataType::Float:       m_base = SqlType::Float;    break;
    case DataType::Double:      m_base = SqlType::Double;   break;
    case DataType::Date:        m_base = SqlType::Date;     break;
    case DataType::Time:        m_base = SqlType::Time;     break;
    case DataType::DateTime:    m_base = SqlType::Timestamp;break;
    case DataType::Interval:    m_base = SqlType::Interval; break;
    case DataType::Array:       m_base = SqlType::Array;    break;
    case DataType::Multiset:    m_base = SqlType::Multiset; break;
    case DataType::Xml:         m_base = SqlType::Xml;      break;
    case DataType::Geometry:    m_base = SqlType::Xml;      break;
    }
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
