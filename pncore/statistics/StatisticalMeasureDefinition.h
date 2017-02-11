#ifndef STATISTICALMEASUREDEFINITION_H
#define STATISTICALMEASUREDEFINITION_H

/*
    Praaline - Core module - Statistics
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

#include <QString>
#include <QVariant>

namespace Praaline {
namespace Core {

class StatisticalMeasureDefinition
{
public:
    StatisticalMeasureDefinition();
    StatisticalMeasureDefinition(const QString &ID, const QString &displayName, const QString &units,
                                 const QString &description = QString(), QVariant::Type type = QVariant::Double, bool isVector = false) :
        m_ID(ID), m_displayName(displayName), m_units(units), m_description(description), m_type(type), m_isVector(isVector)
    {}

    QString ID() const          { return m_ID; }
    QString displayName() const { return m_displayName; }
    QString units() const       { return m_units; }
    QString description() const { return m_description; }
    QVariant::Type type() const { return m_type; }
    bool isVector() const       { return m_isVector; }

    QString displayNameUnit() const {
        if (m_units.isEmpty()) return m_displayName;
        return QString("%1 (%2)").arg(m_displayName).arg(m_units);
    }

private:
    QString m_ID;
    QString m_displayName;
    QString m_units;
    QString m_description;
    QVariant::Type m_type;
    bool m_isVector;
};

} // namespace Core
} // namespace Praaline


#endif // STATISTICALMEASUREDEFINITION_H
