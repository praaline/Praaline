#ifndef NAMEVALUELIST_H
#define NAMEVALUELIST_H

/*
    Praaline - Core module - Corpus Structure Definition
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
#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QVariant>
#include "base/DataType.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT NameValueList  : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ID READ ID)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString datatypeString READ datatypeString)
    Q_PROPERTY(quint64 datatypePrecision READ datatypePrecision)

public:
    explicit NameValueList(QObject *parent = 0);
    ~NameValueList();

    QString ID() const { return m_ID; }
    void setID(const QString &id) { m_ID = id; }
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }
    DataType datatype() const { return m_datatype; }
    void setDatatype(const DataType &datatype) { m_datatype = datatype; }
    QString datatypeString() const { return m_datatype.string(); }
    quint64 datatypePrecision() const { return m_datatype.precision(); }

    QList<QString> displayStrings() const;
    QList<QVariant> values() const;
    int count() const;

    QVariant value(int index) const;
    bool setValue(int index, const QVariant &value);
    QString displayString(int index) const;
    bool setDisplayString(int index, const QString &displayString);
    int indexOfValue(QVariant value) const;

    QVariant value(const QString &displayString) const;
    QString displayString(const QVariant &value) const;

    bool append(const QString &displayString, QVariant value);
    bool insert(int index, const QString &displayString, QVariant value);
    bool removeAt(int index);
    bool remove(const QString &displayString);
    void move(int indexFrom, int indexTo);
    void clear();

signals:
    void listChanged();

private:
    QString m_ID;               // Section ID (e.g. tags)
    QString m_name;             // User-friendly name (e.g. POS Tags)
    QString m_description;      // Description (e.g. Part-of-speech tags)
    DataType m_datatype;        // The data type of the values stored in the list
    // Data
    QList<QString> m_displayStrings;
    QHash<QString, QVariant> m_displayToValue;
    QHash<QString, QString> m_valueToDisplay;
};

} // namespace Core
} // namespace Praaline

#endif // NAMEVALUELIST_H
