#ifndef ANNOTATIONTIERGROUP_H
#define ANNOTATIONTIERGROUP_H

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

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include <QStringList>
#include "base/RealTime.h"

namespace Praaline {
namespace Core {

class AnnotationTier;
class PointTier;
class IntervalTier;
class SequenceTier;
class TreeTier;
class RelationTier;

class PRAALINE_CORE_SHARED_EXPORT AnnotationTierGroup : public QObject
{
    Q_OBJECT
public:
    explicit AnnotationTierGroup(QObject *parent = 0);
    virtual ~AnnotationTierGroup();

    QString ID() const { return m_ID; }
    void setID(const QString &ID) { m_ID = ID; }

    RealTime tMin() const;
    RealTime tMax() const;
    RealTime duration() const { return tMax() - tMin(); }

    AnnotationTier *tier(int index) const;
    AnnotationTier *tier(const QString &name) const;
    int tiersCount() const;
    bool hasTiers() const;
    QList<AnnotationTier *> tiers() const;
    void insertTier(int index, AnnotationTier *tier);
    void insertTierReplacing(int index, AnnotationTier *tier);
    void addTier(AnnotationTier *tier);
    void addTierReplacing(AnnotationTier *tier);
    void swapTiers(int oldIndex, int newIndex);
    void removeTierAt(int i);
    void removeTierByName(const QString &name);
    void renameTier(const QString &nameBefore, const QString &nameAfter);

    // Typed tier accessors
    IntervalTier *getIntervalTierByName(const QString &name) const;
    IntervalTier *getIntervalTierByIndex(int index) const;
    PointTier *getPointTierByName(const QString &name) const;
    PointTier *getPointTierByIndex(int index) const;
    SequenceTier *getSequenceTierByName(const QString &name) const;
    SequenceTier *getSequenceTierByIndex(int index) const;
    TreeTier *getTreeTierByName(const QString &name) const;
    TreeTier *getTreeTierByIndex(int index) const;
    RelationTier *getRelationTierByName(const QString &name) const;
    RelationTier *getRelationTierByIndex(int index) const;

    int getTierIndexByName(const QString &name) const;
    QStringList tierNames() const;

    // Methods
    void insertTierClone(int index, const AnnotationTier *tier, const QString &newName);
    bool reorderTiers(QStringList tierNamesInNewOrder);

signals:
    void tierInserted(AnnotationTier *tier);
    void tierDeleted(QString tierName);
    
public slots:

protected:
    QString m_ID;
    QList<AnnotationTier *> m_tiers;
    QString annotationID;
    QString speakerID;
};

typedef QMap<QString, QPointer<AnnotationTierGroup> > AnnotationTierGroupPerSpeakerMap;

} // namespace Core
} // namespace Praaline

#endif // ANNOTATIONTIERGROUP_H
