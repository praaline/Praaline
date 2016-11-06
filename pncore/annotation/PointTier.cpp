#include <QString>
#include <QTextStream>
#include "PointTier.h"
#include "IntervalTier.h"
#include "AnnotationTier.h"

namespace Praaline {
namespace Core {

// ==========================================================================================================
// Constructors - destructor
// ==========================================================================================================
PointTier::PointTier(const QString &name, const RealTime tMin, const RealTime tMax, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
    m_tMin = tMin;
    m_tMax = tMax;
}

PointTier::PointTier(const QString &name, const QList<Point *> &points,
                     const RealTime tMin, const RealTime tMax, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
    m_tMin = tMin;
    m_tMax = tMax;
    m_points = points;
    if (m_points.count() == 0) return;
    qSort(m_points.begin(), m_points.end(), PointTier::ComparePoints);
    if (m_tMin > m_points.first()->m_time) m_tMin = m_points.first()->m_time;
    if (m_tMax < m_points.last()->m_time) m_tMax = m_points.last()->m_time;
}

PointTier::PointTier(const PointTier *copy, QString name, QObject *parent) :
    AnnotationTier(parent)
{
    if (!copy) return;
    m_name = (name.isEmpty()) ? copy->name() : name;
    m_tMin = copy->tMin();
    m_tMax = copy->tMax();
    // deep copy of points
    foreach (Point *point, copy->points()) {
        m_points << new Point(point->time(), point->text());
    }
}

PointTier::~PointTier()
{
    qDeleteAll(m_points);
}

// ==========================================================================================================
// Accessors and mutators for Points
// ==========================================================================================================

Point *PointTier::point(int index) const
{
    return m_points.value(index);
}

QList<Point *> PointTier::points() const
{
    return m_points;
}

bool PointTier::ComparePoints(Point *A, Point *B) {
    return (A->time() < B->time());
}

Point *PointTier::pointAtTime(RealTime t, RealTime threshold)
{
    int index = pointIndexAtTime(t, threshold);
    if (index == -1)
        return 0; // not found
    return m_points.at(index);
}

int PointTier::pointIndexAtTime(RealTime t, RealTime threshold) const
{
    if ((t < m_tMin) || (t > m_tMax)) return -1;
    // The points are sorted by time. Therefore, use binary search.
    int imin = 0;
    int imax = m_points.count() - 1;
    // continue searching while [imin, imax] is not empty
    while (imax >= imin) {
        // calculate the midpoint for roughly equal partition
        int imid = (imin + imax) / 2;
        // determine which subarray to search
        if (m_points.at(imid)->time() < t - threshold) {
            // change min index to search upper subarray
            imin = imid + 1;
        }
        else if (m_points.at(imid)->time() > t + threshold) {
            // change max index to search lower subarray
            imax = imid - 1;
        }
        else {
            // interval found at index imid
            return imid;
        }
    }
    // not found
    return -1;
}

void PointTier::addPoint(Point *point)
{
    m_points << point;
    qSort(m_points.begin(), m_points.end(), PointTier::ComparePoints);
}

void PointTier::addPoints(QList<Point *> points)
{
    m_points << points;
    qSort(m_points.begin(), m_points.end(), PointTier::ComparePoints);
}

void PointTier::removePointAt(int i)
{
    m_points.removeAt(i);
}

void PointTier::timeShift(const RealTime delta)
{
    m_tMin = m_tMin + delta;
    m_tMax = m_tMax + delta;
    foreach (Point *p, m_points) {
        p->m_time = p->m_time + delta;
    }
}

QList<RealTime> PointTier::tCenters() const
{
    QList<RealTime> ret;
    foreach (Point *p, m_points)
        ret << p->time();
    return ret;
}

void PointTier::replaceText(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Point *p, m_points) {
        p->replaceText(before, after, cs);
    }
}

QList<QString> PointTier::getDistinctTextLabels() const
{
    QList<QString> ret;
    foreach (Point *p, m_points) {
        if (!ret.contains(p->text())) {
            ret << p->text();
        }
    }
    return ret;
}

void PointTier::fillEmptyAnnotationsWith(const QString &filler)
{
    foreach (Point *p, m_points) {
        if (p->text().isEmpty()) p->setText(filler);
    }
}

void PointTier::fixBoundariesBasedOnTier(const IntervalTier *correctBoundariesTier, RealTime threshold)
{
    RealTime previous, next, proposed;
    for (int i = 0; i < m_points.count(); i++) {
        if (i - 1 >= 0)
            previous = m_points[i - 1]->time();
        else
            previous = RealTime(0, 0);
        if (i + 1 < m_points.count())
            next = correctBoundariesTier->getBoundaryClosestTo(m_points[i + 1]->time());
        else
            next = m_tMax; // of the tier
        // now correct
        proposed = correctBoundariesTier->getBoundaryClosestTo(m_points[i]->time());
        if ((proposed >= previous) && (proposed <= next) &&
            (RealTime::abs(proposed - m_points[i]->time()) < threshold))
            m_points[i]->m_time = proposed;
    }
}

QList<Point *> PointTier::getPointsContainedIn(const Interval *container) const
{
    QList<Point *> ret;
    foreach (Point *p, m_points) {
        if (container->contains(p->time())) {
            ret << p;
        }
    }
    return ret;
}

QList<Point *> PointTier::getPointsContainedIn(RealTime tMin, RealTime tMax) const
{
    QList<Point *> ret;
    int low = pointIndexAtTime(tMin); --low; if (low < 0) low = 0;
    int high = pointIndexAtTime(tMax); ++high; if (high >= countItems()) high = countItems() -1;
    if (m_points.at(low)->time() >= tMin) ret << m_points.at(low);
    for (int i = low + 1; i <= high - 1; ++i) ret << m_points.at(i);
    if (m_points.at(high)->time() <= tMax) ret << m_points.at(high);
    return ret;
}

IntervalTier *PointTier::getIntervalsMin(const QString &name, QObject *parent)
{
    IntervalTier *ret = new IntervalTier(name, m_tMin, m_tMax, parent);
    foreach (Point *p, m_points) {
        Interval *intv = ret->split(ret->countItems() - 1, p->time());
        intv->setText(p->text());
    }
    return ret;
}

IntervalTier *PointTier::getIntervalsMax(const QString &name, QObject *parent)
{
    QList<Interval *> intervals;
    RealTime tMin = m_tMin;
    foreach (Point *p, m_points) {
        intervals << new Interval(tMin, p->time(), p->text());
        tMin = p->time();
    }
    return new IntervalTier(name, intervals, m_tMin, m_tMax, parent);
}

QList<Point *> PointTier::findLocalMaxima(const RealTime &localMaxThreshold, const QString &compareAttributeID) const
{
    QList<Point *> ret;
    foreach (Point *p, m_points) {
        // test if local maximum
        QList<Point *> locals = getPointsContainedIn(p->time() - localMaxThreshold, p->time() + localMaxThreshold);
        bool localMax = true;
        foreach (Point *local, locals) {
            if (local == p) continue;
            if (local->attribute(compareAttributeID).toDouble() > p->attribute(compareAttributeID).toDouble())
                localMax = false;
        }
        if (localMax) ret << p;
    }
    return ret;
}

} // namespace Core
} // namespace Praaline
