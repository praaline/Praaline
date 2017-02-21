#ifndef INTERVALTIER_H
#define INTERVALTIER_H

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
#include <QHash>
#include <QVariant>
#include "base/RealTime.h"
#include "AnnotationTier.h"
#include "Interval.h"

namespace Praaline {
namespace Core {

class Point;
class PointTier;

class PRAALINE_CORE_SHARED_EXPORT IntervalTier : public AnnotationTier
{
    Q_OBJECT

public:
    // Constructors, destructor
    IntervalTier(const QString &name = QString(),
                 const RealTime tMin = RealTime(0, 0), const RealTime tMax = RealTime(0, 0), QObject *parent = 0);
    IntervalTier(const QString &name, const QList<Interval *> &intervals,
                 const RealTime tMin = RealTime(0, 0), const RealTime tMax = RealTime(0, 0), QObject *parent = 0);
    IntervalTier(const IntervalTier *copy, QString name = QString(), bool copyAttributes = true, QObject *parent = 0);
    IntervalTier(const QString &name, const IntervalTier *tierA, const IntervalTier *tierB,
                 const QString &textA, const QString &textB, const QString &textAB, QObject *parent = 0);
    virtual ~IntervalTier();

    // Implementation of AnnotationTier
    AnnotationTier::TierType tierType() const override
        { return AnnotationTier::TierType_Intervals; }
    int count() const override
        { return m_intervals.count(); }
    bool isEmpty() const override;
    void clear() override;
    Interval *at(int index) const override;
    Interval *first() const override;
    Interval *last() const override;
    QStringList getDistinctLabels() const override;
    QList<QVariant> getDistinctValues(const QString &attributeID) const override;
    void replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) override;
    void fillEmptyWith(const QString &attributeID,const QString &filler) override;

    // Accessors for Intervals
    Interval *interval(int index) const;
    QList<Interval *> intervals() const;

    Interval *intervalAtTime(RealTime t);
    int intervalIndexAtTime(RealTime t) const;

    // Mutators for Intervals
    Interval *addToEnd(RealTime tMax, const QString &text);
    Interval *split(RealTime at);
    Interval *split(int index, RealTime at);
    QList<Interval *> splitToEqual(int index, int numberOfIntervals);
    QList<Interval *> splitToProportions(int index, const QList<int> &proportions);
    Interval *merge(int indexFrom, int indexTo, const QString &separator = QString());
    void copyIntervalsFrom(const IntervalTier *copy, bool copyData = true);
    void replaceAllIntervals(QList<Interval *> &newIntervals);
    bool moveBoundary(int index, RealTime time);
    bool realignIntervals(int indexFrom, QList<RealTime> &newBoundaries);

    // Methods
    QList<RealTime> times() const;
    QList<RealTime> timesMin() const;
    QList<RealTime> timesCenter() const;
    QList<RealTime> timesMax() const;
    void timeShift(const RealTime delta);

    RealTime getBoundaryClosestTo(const RealTime timePoint) const;
    void fixBoundariesBasedOnTier(const IntervalTier *correctBoundariesTier,
                                  RealTime threshold = RealTime(0, 100000000));
    void mergeIdenticalAnnotations(const QString &text = QString(), const QStringList &ignoreIntervening = QStringList());
    void mergeContiguousAnnotations(const QStringList &separatorsOfIntervals, const QString &separatorWhenMerging);

    QList<Interval *> getIntervalsContainedIn(const Interval *container) const;
    QList<Interval *> getIntervalsContainedIn(const RealTime &timeStart, const RealTime &timeEnd) const;
    QPair<int, int> getIntervalIndexesContainedIn(const Interval *container) const;
    QPair<int, int> getIntervalIndexesContainedIn(const RealTime &timeStart, const RealTime &timeEnd) const;
    QList<Interval *> getIntervalsOverlappingWith(const Interval *contained) const;
    QList<Interval *> getIntervalsOverlappingWith(const RealTime &timeStart, const RealTime &timeEnd) const;
    QList<Interval *> getIntervalsOverlappingWith(const Interval *contained, const RealTime &threshold) const;
    QList<Interval *> getIntervalsOverlappingWith(const RealTime &timeStart, const RealTime &timeEnd, const RealTime &threshold) const;
    IntervalTier *getIntervalTierSubset(const RealTime &timeStart, const RealTime &timeEnd) const;

    PointTier *getPointsMin(const QString &name, QObject *parent = 0);
    PointTier *getPointsMax(const QString &name, QObject *parent = 0);
    void setIOBAnnotationAttribute(const QString attribute, const IntervalTier *tierAnnotation);
    QString getIntervalsText(int indexStart, int indexEnd, QString separator = " ");
    QString getIntervalsText(RealTime timeStart, RealTime timeEnd, QString separator = " ");
    QList<Interval *> getContext(int index, int delta) const;
    QList<Interval *> getContext(int index, RealTime delta) const;
    QString getContextSymmetricFormated(int index, int delta, QString sep = " ", QString left = "<", QString right = ">");

protected:
    QList<Interval *> m_intervals;

private:
    void fixEmptyIntervals();
    static bool compareIntervals(Interval *A, Interval *B);
};

} // namespace Core
} // namespace Praaline

#endif // INTERVALTIER_H
