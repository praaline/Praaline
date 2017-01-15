#ifndef ANNOTATIONSTRUCTURELEVEL_H
#define ANNOTATIONSTRUCTURELEVEL_H

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
#include <QPointer>
#include <QString>
#include <QStringList>
#include "StructureAttributeBase.h"
#include "AnnotationStructureAttribute.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationStructureLevel : public StructureAttributeBase
{
    Q_OBJECT
    Q_PROPERTY(LevelType levelType READ levelType WRITE setLevelType)
    Q_PROPERTY(QString parentLevelID READ parentLevelID WRITE setParentLevelID)
    Q_ENUMS(LevelType)

public:
    enum LevelType {
        IndependentPointsLevel    = 10,
        IndependentIntervalsLevel = 20,
        GroupingLevel             = 30,
        SequencesLevel            = 40,
        TreeLevel                 = 50,
        RelationsLevel            = 60
    };

    explicit AnnotationStructureLevel(QObject *parent = 0);
    AnnotationStructureLevel(const QString &ID, LevelType type = IndependentIntervalsLevel,
                             const QString &name = QString(), const QString &description = QString(),
                             const QString &parentLevelID = QString(),
                             const DataType &datatype = DataType(DataType::VarChar, 1024),
                             int order = 0, bool indexed = false, const QString &nameValueList = QString(),
                             QObject *parent = 0);

    // Data
    LevelType levelType() const { return m_levelType; }
    bool isLevelTypePrimary() const;
    void setLevelType(LevelType type) { m_levelType = type; }
    QString parentLevelID() const { return m_parentLevelID; }
    void setParentLevelID(const QString &parentLevelID) { m_parentLevelID = parentLevelID; }

    // ATTRIBUTES
    // Accessors
    QPointer<AnnotationStructureAttribute> attribute(int index) const;
    QPointer<AnnotationStructureAttribute> attribute(const QString &ID) const;
    int attributeIndexByID(const QString &ID) const;
    int attributesCount() const;
    bool hasAttributes() const;
    bool hasAttribute(const QString &ID);
    QStringList attributeIDs() const;
    QList<QPointer<AnnotationStructureAttribute> > attributes() const;
    void insertAttribute(int index, AnnotationStructureAttribute *attribute);
    void addAttribute(AnnotationStructureAttribute *attribute);
    void swapAttribute(int oldIndex, int newIndex);
    void removeAttributeAt(int i);
    void removeAttributeByID(const QString &ID);

signals:
    
public slots:

protected:
    LevelType m_levelType;      // Level type (e.g. independent or grouping)
    QString m_parentLevelID;    // Parent level ID for grouping levels (e.g. phone for syll)
    QList<QPointer<AnnotationStructureAttribute> > m_attributes;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSANNOTATIONLEVEL_H

