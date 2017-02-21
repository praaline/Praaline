#ifndef POINTTIER_H
#define POINTTIER_H

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
#include "base/RealTime.h"
#include "AnnotationTier.h"
#include "Point.h"

namespace Praaline {
namespace Core {

class Interval;
class IntervalTier;

class PRAALINE_CORE_SHARED_EXPORT PointTier : public AnnotationTier
{
    Q_OBJECT
public:
    // Constructors, destructor
    PointTier(const QString &name = QString(),
              const RealTime tMin = RealTime(0, 0), const RealTime tMax = RealTime(0, 0), QObject *parent = 0);
    PointTier(const QString &name, const QList<Point *> &points,
              const RealTime tMin = RealTime(0, 0), const RealTime tMax = RealTime(0, 0), QObject *parent = 0);
    PointTier(const PointTier *copy, QString name = QString(), QObject *parent = 0);
    virtual ~PointTier();

    // Implementation of AnnotationTier
    AnnotationTier::TierType tierType() const override
        { return AnnotationTier::TierType_Points; }
    int count() const override
        { return m_points.count(); }
    bool isEmpty() const override;
    void clear() override;
    Point *at(int index) const override;
    Point *first() const override;
    Point *last() const override;
    QStringList getDistinctLabels() const override;
    QList<QVariant> getDistinctValues(const QString &attributeID) const override;
    void replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive) override;
    void fillEmptyWith(const QString &attributeID,const QString &filler) override;

    // Accessors for Points
    Point* point(int index) const;
    QList<Point *> points() const;

    Point *pointAtTime(RealTime t, RealTime threshold = RealTime(0, 0));
    int pointIndexAtTime(RealTime t, RealTime threshold = RealTime(0, 0)) const;

    // Mutators for Points
    void addPoint(Point *point);
    void addPoints(QList<Point *> points);
    void removePointAt(int i);

    // Methods
    QList<RealTime> times() const;
    void timeShift(const RealTime delta);
    void fixBoundariesBasedOnTier(const IntervalTier *correctBoundariesTier,
                                  RealTime threshold = RealTime(0, 100000000));
    QList<Point *> getPointsContainedIn(const Interval *container) const;
    QList<Point *> getPointsContainedIn(RealTime tMin, RealTime tMax) const;
    IntervalTier *getIntervalsMin(const QString &name, QObject *parent = 0);
    IntervalTier *getIntervalsMax(const QString &name, QObject *parent = 0);

    QList<Point *> findLocalMaxima(const RealTime &localMaxThreshold, const QString &compareAttributeID) const;

protected:
    QList<Point *> m_points;

private:
    static bool comparePoints(Point *A, Point *B);
};

} // namespace Core
} // namespace Praaline

#endif // POINTTIER_H
