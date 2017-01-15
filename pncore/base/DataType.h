#ifndef DATATYPE_H
#define DATATYPE_H

/*
    Praaline - Core module - Base
    Copyright (c) 2011-2017 George Christodoulides

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QString>

namespace Praaline {
namespace Core {

/*!
 * \brief value object that represent any sql type
 */
class PRAALINE_CORE_SHARED_EXPORT DataType
{
public:
    /*!
     * \brief Base Type enum
     */
    enum Base {
        Invalid,
        Char,               //!< precision is length (defaults to 1)
        VarChar,            //!< precision is maximum length (defaults to 255)
        Binary,             //!< precision represents length in bytes (required)
        VarBinary,          //!< precision represents maximum bytes
        Boolean,
        SmallInt,           //!< precision is 5
        Integer,            //!< precision defaults to 10
        BigInt,             //!< precision is 19
        Decimal,            //!< precision and scale required (total digits, digits after comma)
        Float,
        Double,
        Date,
        Time,
        DateTime,
        Interval,
        Array,
        Multiset,
        Xml,
        Geometry
    };

    /*!
     * \brief Construct the type from
     * \param string the string representation of an sql type
     */
    DataType(const QString &string);

    /*!
     * \brief Constructs the type
     * \param base the required base type
     * \param precision if supported or required the precision or length
     * \param scale if supported or required by the base type
     */
    DataType(Base base, quint64 precision = 0, int scale = 0);

    //! \return the string representation of the data type
    const QString string() const;

    //! \return the base type (if base form is used)
    Base base() const;

    //! \return the precision (only used for supported types - numeric and string)
    quint64 precision(quint64 defaultValue = 0) const;

    //! \return the scale of the type (only used for numeric values)
    int scale() const;

    //! \return the invalid Type
    static const DataType& invalid();

    //! \return true if this type is not invalid
    bool isValid() const;

private:
    Base m_base;
    quint64 m_precision;
    int m_scale;
};

} // namespace Core
} // namespace Praaline

#endif // DATATYPE_H
