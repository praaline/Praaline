#ifndef STATISTICALMEASUREDEFINITION_H
#define STATISTICALMEASUREDEFINITION_H

#include <QString>
#include <QVariant>

namespace Praaline {
namespace Core {

class StatisticalMeasureDefinition
{
public:
    StatisticalMeasureDefinition();
    StatisticalMeasureDefinition(const QString &ID, const QString &displayName, const QString &units,
                                 const QString &description = QString(), QVariant::Type type = QVariant::Double) :
        m_ID(ID), m_displayName(displayName), m_units(units), m_description(description), m_type(type)
    {}

    QString ID() const          { return m_ID; }
    QString displayName() const { return m_displayName; }
    QString units() const       { return m_units; }
    QString description() const { return m_description; }
    QVariant::Type type() const { return m_type; }

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
};

} // namespace Core
} // namespace Praaline


#endif // STATISTICALMEASUREDEFINITION_H