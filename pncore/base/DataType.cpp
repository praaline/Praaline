#include "DataType.h"

namespace Praaline {
namespace Core {

DataType::DataType(DataType::Base base, quint64 precision, int scale) :
    m_base(base),
    m_precision(precision),
    m_scale(scale)
{
}

DataType::DataType(const QString &string) :
    m_base(Invalid),
    m_precision(0),
    m_scale(0)
{
         if (string == "invalid")   m_base = DataType::Invalid;
    else if (string == "char")      m_base = DataType::Char;
    else if (string == "varchar")   m_base = DataType::VarChar;
    else if (string == "binary")    m_base = DataType::Binary;
    else if (string == "varbinary") m_base = DataType::VarBinary;
    else if (string == "bool")      m_base = DataType::Boolean;
    else if (string == "smallint")  m_base = DataType::SmallInt;
    else if (string == "integer")   m_base = DataType::Integer;
    else if (string == "bigint")    m_base = DataType::BigInt;
    else if (string == "decimal")   m_base = DataType::Decimal;
    else if (string == "float")     m_base = DataType::Float;
    else if (string == "double")    m_base = DataType::Double;
    else if (string == "date")      m_base = DataType::Date;
    else if (string == "time")      m_base = DataType::Time;
    else if (string == "datetime")  m_base = DataType::DateTime;
    else if (string == "interval")  m_base = DataType::Interval;
    else if (string == "array")     m_base = DataType::Array;
    else if (string == "multiset")  m_base = DataType::Multiset;
    else if (string == "xml")       m_base = DataType::Xml;
    else if (string == "geometry")  m_base = DataType::Geometry;
}

bool DataType::isValid() const
{
    return m_base != Invalid;
}

const QString DataType::string() const
{
    QString s;
    switch (m_base) {
    case DataType::Invalid:     s = "invalid";      break;
    case DataType::Char:        s = "char";         break;
    case DataType::VarChar:     s = "varchar";      break;
    case DataType::Binary:      s = "binary";       break;
    case DataType::VarBinary:   s = "varbinary";    break;
    case DataType::Boolean:     s = "bool";         break;
    case DataType::SmallInt:    s = "smallint";     break;
    case DataType::Integer:     s = "integer";      break;
    case DataType::BigInt:      s = "bigint";       break;
    case DataType::Decimal:     s = "decimal";      break;
    case DataType::Float:       s = "float";        break;
    case DataType::Double:      s = "double";       break;
    case DataType::Date:        s = "date";         break;
    case DataType::Time:        s = "time";         break;
    case DataType::DateTime:    s = "datetime";     break;
    case DataType::Interval:    s = "interval";     break;
    case DataType::Array:       s = "array";        break;
    case DataType::Multiset:    s = "multiset";     break;
    case DataType::Xml:         s = "xml";          break;
    case DataType::Geometry:    s = "geometry";     break;
    }
    return s;
}

DataType::Base DataType::base() const {
    return m_base;
}

quint64 DataType::precision(quint64 defaultValue) const {
    if( 0 == m_precision )
        return defaultValue;
    return m_precision;
}

int DataType::scale() const {
    return m_scale;
}

const DataType &DataType::invalid()
{
    static DataType invalid_sql_type(DataType::Invalid);
    return invalid_sql_type;
}

} // namespace Core
} // namespace Praaline

