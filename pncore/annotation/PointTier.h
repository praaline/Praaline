#ifndef POINTTIER_H
#define POINTTIER_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QList>
#include "base/RealTime.h"
#include "AnnotationTier.h"
#include "Point.h"
#include "IntervalTier.h"

namespace Praaline {
namespace Core {

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
    AnnotationTier::TierType tierType() const
        { return AnnotationTier::TierType_Points; }
    int count() const
        { return m_points.count(); }
    bool isEmpty() const;
    void clear();
    Point *at(int index) const;
    Point *first() const;
    Point *last() const;
    QList<QString> getDistinctTextLabels() const;
    QList<QVariant> getDistinctAttributeValues(const QString &attributeID) const;
    void replaceTextLabels(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    void fillEmptyTextLabelsWith(const QString &filler);
    void replaceAttributeText(const QString &attributeID, const QString &before, const QString &after,
                              Qt::CaseSensitivity cs = Qt::CaseSensitive);
    void fillEmptyAttributeTextWith(const QString &attributeID,const QString &filler);


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
