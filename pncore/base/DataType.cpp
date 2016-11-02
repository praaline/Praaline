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
    m_scale(0),
    m_string(string)
{
}

bool DataType::isBaseType() const
{
    return m_base != Invalid;
}

bool DataType::isString() const
{
    return m_base == Invalid && (!m_string.isEmpty());
}

bool DataType::isValid() const
{
    return m_base != Invalid || (!m_string.isEmpty());
}

const QString &DataType::string() const
{
    return m_string;
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

