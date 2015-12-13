#ifndef POINTTIER_H
#define POINTTIER_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QList>
#include "base/RealTime.h"
#include "annotationtier.h"
#include "point.h"
#include "intervaltier.h"

class IntervalTier;

class PRAALINE_CORE_SHARED_EXPORT PointTier : public AnnotationTier
{
    Q_OBJECT
public:
    // Constructors, destructor
    PointTier(const QString &name = QString(),
              const RealTime tMin = RealTime(0, 0), const RealTime tMax = RealTime(0, 0), QObject *parent = 0);
    PointTier(const QString &name, const RealTime tMin, const RealTime tMax,
              const QList<Point *> &points, QObject *parent = 0);
    PointTier(const PointTier *copy, QString name = QString(), QObject *parent = 0);
    virtual ~PointTier();

    AnnotationTier::TierType tierType() const
        { return AnnotationTier::TierType_Points; }
    QString name() const
        { return m_name; }
    void setName(const QString &name)
        { m_name = name; }
    RealTime tMin() const
        { return m_tMin; }
    RealTime tMax() const
        { return m_tMax; }
    //void resize(const new_tMin, const new_tMax);
    int countItems() const
        { return m_points.count(); }
    bool isEmpty() const
        { return m_points.isEmpty(); }
    void clear()
        { m_points.clear(); }

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
    // implementing AnnotationTier
    int count() const { return m_points.count(); }
    void replaceText(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QList<QString> getDistinctTextLabels() const;
    void fillEmptyAnnotationsWith(const QString &filler);
    QList<RealTime> tCenters() const;
    // other
    void timeShift(const RealTime delta);
    void fixBoundariesBasedOnTier(const IntervalTier *correctBoundariesTier,
                                  RealTime threshold = RealTime(0, 100000000));
    QList<Point *> getPointsContainedIn(const Interval *container) const;
    QList<Point *> getPointsContainedIn(RealTime tMin, RealTime tMax) const;
    IntervalTier *getIntervalsMin(const QString &name, QObject *parent = 0);
    IntervalTier *getIntervalsMax(const QString &name, QObject *parent = 0);

    QList<Point *> findLocalMaxima(const RealTime &localMaxThreshold, const QString &compareAttributeID) const;

protected:
    QString m_name;
    RealTime m_tMin;
    RealTime m_tMax;
    QList<Point *> m_points;

private:
    static bool ComparePoints(Point *A, Point *B);
};

#endif // POINTTIER_H
