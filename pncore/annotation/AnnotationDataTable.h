#ifndef ANNOTATIONDATATABLE_H
#define ANNOTATIONDATATABLE_H

/*
    Praaline - Core module - Annotation
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

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QVariant>
#include <QHash>
#include "base/RealTime.h"
#include "base/RealValueList.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationDataTable : public QObject
{
    Q_OBJECT
public:
    explicit AnnotationDataTable(QObject *parent = 0);
    virtual ~AnnotationDataTable();

    QString ID() const { return m_ID; }
    void setID(const QString &ID) { m_ID = ID; }

    virtual int getRowCount() const;
    virtual int getColumnCount() const;

    virtual void clear();

    virtual QStringList getFieldNames() const;
    virtual QStringList getFieldTypes() const;
    virtual QString getFieldType(const QString &fieldname) const;
    virtual QStringList getFieldDescriptions() const;
    virtual QString getFieldDescription(const QString &fieldname) const;

    virtual QVariant getData(int row, int column);
    virtual QVariant getData(int row, const QString &fieldname);

    virtual RealTime getSeconds(int row, const QString &fieldname);

    virtual bool readFromFile(const QString &filename);
    virtual bool saveToFile(const QString &filename);

    virtual RealValueList getValueList(const QString &fieldname);
signals:

public slots:

protected:
    QString m_ID;
    QHash<QString, int> m_fieldNames;               // field name, column
    QHash<QString, QString> m_fieldTypes;           // field name, field type
    QHash<QString, QString> m_fieldDescriptions;    // field name, field description
    QList< QList<QVariant> > m_data;                // rows < columns >
};

} // namespace Core
} // namespace Praaline

#endif
