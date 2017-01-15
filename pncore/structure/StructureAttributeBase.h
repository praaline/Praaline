#ifndef STRUCTUREATTRIBUTEBASE_H
#define STRUCTUREATTRIBUTEBASE_H

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
#include "base/DataType.h"
#include <QObject>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT StructureAttributeBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString ID READ ID WRITE setID)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString description READ description WRITE setDescription)
    Q_PROPERTY(QString datatypeString READ datatypeString)
    Q_PROPERTY(quint64 datatypePrecision READ datatypePrecision)
    Q_PROPERTY(int itemOrder READ itemOrder WRITE setItemOrder)
    Q_PROPERTY(bool indexed READ indexed WRITE setIndexed)
    Q_PROPERTY(QString nameValueList READ nameValueList WRITE setNameValueList)

public:
    explicit StructureAttributeBase(QObject *parent = 0);
    StructureAttributeBase(const QString &ID, QObject *parent = 0);
    StructureAttributeBase(const QString &ID, const QString &name, const QString &description, const DataType &datatype,
                  int order = 0, bool indexed = false, const QString &nameValueList = QString(), QObject *parent = 0);
    StructureAttributeBase(const StructureAttributeBase *other, QObject *parent = 0);
    virtual ~StructureAttributeBase() {}

    // Data
    virtual QString ID() const { return m_ID; }
    virtual void setID(const QString &ID) { m_ID = ID; }
    virtual QString name() const { return m_name; }
    virtual void setName(const QString &name) { m_name = name; }
    virtual QString description() const { return m_description; }
    virtual void setDescription(const QString &description) { m_description = description; }
    virtual DataType datatype() const { return m_datatype; }
    virtual void setDatatype(const DataType &datatype) { m_datatype = datatype; }
    virtual QString datatypeString() const { return m_datatype.string(); }
    virtual quint64 datatypePrecision() const { return m_datatype.precision(); }
    virtual int itemOrder() const { return m_itemOrder; }
    virtual void setItemOrder(int itemOrder) { m_itemOrder = itemOrder; }
    virtual bool indexed() const { return m_indexed; }
    virtual void setIndexed(bool indexed) { m_indexed = indexed; }
    virtual QString nameValueList() const { return m_nameValueList; }
    virtual void setNameValueList(const QString &nameValueList) { m_nameValueList = nameValueList; }

signals:

public slots:

protected:
    QString m_ID;               // ID (e.g. f0min)
    QString m_name;             // User-friendly name (e.g. F0 Minimum)
    QString m_description;      // Description (e.g. Fundamental frequency (f0) minimum, in semitones)
    DataType m_datatype;        // Data type (e.g. double) and precision
    int m_itemOrder;            // Used to order attributes in a container (e.g. inside a metadata section)
                                // for presentation purposes
    bool m_indexed;             // Should create database index?
    QString m_nameValueList;    // Use a name-value list (controlled vocabulary)
};

} // namespace Core
} // namespace Praaline

#endif // STRUCTUREATTRIBUTEBASE_H
