#ifndef ANNOTATIONSTRUCTUREATTRIBUTE_H
#define ANNOTATIONSTRUCTUREATTRIBUTE_H

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
#include "StructureAttributeBase.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationStructureAttribute : public StructureAttributeBase
{
    Q_OBJECT
    Q_PROPERTY(QString statLevelOfMeasurement READ statLevelOfMeasurement  WRITE setStatLevelOfMeasurement)

public:
    explicit AnnotationStructureAttribute(QObject *parent = 0);
    AnnotationStructureAttribute(const AnnotationStructureAttribute *other, QObject *parent = 0);
    AnnotationStructureAttribute(const QString &ID, const QString &name = QString(), const QString &description = QString(),
                                 const DataType &datatype = DataType(DataType::VarChar, 255), int order = 0,
                                 bool indexed = false, const QString &nameValueList = QString(),
                                 QObject *parent = 0);

    // Data
    QString statLevelOfMeasurement() const { return m_statLevelOfMeasurement; }
    void setStatLevelOfMeasurement(const QString &statLevelOfMeasurement) { m_statLevelOfMeasurement = statLevelOfMeasurement; }

signals:
    
public slots:

protected:
    QString m_statLevelOfMeasurement;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSANNOTATIONATTRIBUTE_H
