#ifndef ANNOTATIONSTRUCTURE_H
#define ANNOTATIONSTRUCTURE_H

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
#include "AnnotationStructureLevel.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT AnnotationStructure : public QObject
{
    Q_OBJECT
public:
    explicit AnnotationStructure(QObject *parent = 0);
    ~AnnotationStructure();

    QString ID() const { return m_ID; }
    void setID(const QString &ID) { m_ID = ID; }

    // Management
    void clear();

    // ANNOTATION LEVELS
    AnnotationStructureLevel *level(int index) const;
    AnnotationStructureLevel *level(const QString &ID) const;
    int getLevelIndexByID(const QString &ID) const;
    int levelsCount() const;
    bool hasLevels() const;
    bool hasLevel(const QString &ID) const;
    QStringList levelIDs() const;
    QList<AnnotationStructureLevel *> levels() const;
    void insertLevel(int index, AnnotationStructureLevel *level);
    void addLevel(AnnotationStructureLevel *level);
    void swapLevels(int oldIndex, int newIndex);
    void removeLevelAt(int i);
    void removeLevelByID(const QString &ID);

signals:
    void AnnotationStructureChanged();

public slots:
    
protected:
    QString m_ID;
    QList<AnnotationStructureLevel *> m_levels;
};

} // namespace Core
} // namespace Praaline

#endif // CORPUSANNOTATIONSTRUCTURE_H
