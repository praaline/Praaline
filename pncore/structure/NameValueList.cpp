#include "NameValueList.h"

namespace Praaline {
namespace Core {

NameValueList::NameValueList(QObject *parent) :
    QObject(parent), m_datatype(DataType::Invalid)
{
}

NameValueList::~NameValueList()
{
}

QList<QString> NameValueList::displayStrings() const
{
    return m_displayStrings;
}

QList<QVariant> NameValueList::values() const
{
    QList<QVariant> ret;
    foreach (QString str, m_displayStrings) {
        ret << value(str);
    }
    return ret;
}

int NameValueList::count() const
{
    return m_displayStrings.count();
}

QVariant NameValueList::value(int index) const
{
    if ((index < 0) || (index >= m_displayStrings.count())) return QVariant();
    return m_displayToValue.value(m_displayStrings.at(index), QVariant());
}

bool NameValueList::setValue(int index, const QVariant &value)
{
    if ((index < 0) || (index >= m_displayStrings.count())) return false;
    if (m_valueToDisplay.contains(value.toString())) return false;
    QString displayStringToModify = m_displayStrings.at(index);
    QString previousValueKey = m_displayToValue.value(displayStringToModify).toString();
    m_displayToValue.insert(displayStringToModify, value);
    m_valueToDisplay.remove(previousValueKey);
    m_valueToDisplay.insert(value.toString(), displayStringToModify);
    return true;
}

QString NameValueList::displayString(int index) const
{
    if ((index < 0) || (index >= m_displayStrings.count())) return QString();
    return m_displayStrings.at(index);
}

bool NameValueList::setDisplayString(int index, const QString &displayString)
{
    if ((index < 0) || (index >= m_displayStrings.count())) return false;
    if (m_displayStrings.contains(displayString)) return false;
    QString previousDisplayString = m_displayStrings.at(index);
    QVariant valueToModify = m_displayToValue.value(previousDisplayString);
    m_displayStrings.replace(index, displayString);
    m_displayToValue.remove(previousDisplayString);
    m_displayToValue.insert(displayString, valueToModify);
    m_valueToDisplay.insert(valueToModify.toString(), displayString);
    return true;
}

int NameValueList::indexOfValue(QVariant value) const
{
    return m_displayStrings.indexOf(displayString(value));
}

QVariant NameValueList::value(const QString &displayString) const
{
    return m_displayToValue.value(displayString, QVariant());
}


QString NameValueList::displayString(const QVariant &value) const
{
    return m_valueToDisplay.value(value.toString(), QString());
}


bool NameValueList::append(const QString &displayString, QVariant value)
{
    if (m_displayStrings.contains(displayString)) return false;
    if (m_valueToDisplay.contains(value.toString())) return false;
    m_displayStrings.append(displayString);
    m_displayToValue.insert(displayString, value);
    m_valueToDisplay.insert(value.toString(), displayString);
    emit listChanged();
    return true;
}

bool NameValueList::insert(int index, const QString &displayString, QVariant value)
{
    if (m_displayStrings.contains(displayString)) return false;
    if (m_valueToDisplay.contains(value.toString())) return false;
    m_displayStrings.insert(index, displayString);
    m_displayToValue.insert(displayString, value);
    m_valueToDisplay.insert(value.toString(), displayString);
    emit listChanged();
    return true;
}

bool NameValueList::removeAt(int index)
{
    if ((index < 0) || (index >= m_displayStrings.count())) return false;
    return remove(m_displayStrings.at(index));
}

bool NameValueList::remove(const QString &displayString)
{
    if (!m_displayStrings.contains(displayString)) return false;
    m_valueToDisplay.remove(value(displayString).toString());
    m_displayToValue.remove(displayString);
    m_displayStrings.removeOne(displayString);
    emit listChanged();
    return true;
}

void NameValueList::move(int indexFrom, int indexTo)
{
    m_displayStrings.move(indexFrom, indexTo);
}

void NameValueList::clear()
{
    m_displayStrings.clear();
    m_displayToValue.clear();
    m_valueToDisplay.clear();
}

} // namespace Core
} // namespace Praaline
