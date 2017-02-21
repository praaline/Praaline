#include <QString>
#include <QList>
#include <QTextStream>
#include "Interval.h"
#include "IntervalTier.h"
#include "Point.h"
#include "PointTier.h"
#include "AnnotationTier.h"

namespace Praaline {
namespace Core {

// ==========================================================================================================
// Constructors - destructor
// ==========================================================================================================
IntervalTier::IntervalTier(const QString &name, const RealTime tMin, const RealTime tMax, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
    m_tMin = tMin;
    m_tMax = tMax;
    m_intervals << new Interval(tMin, tMax, ""); // initial blank interval
}

IntervalTier::IntervalTier(const QString &name, const QList<Interval *> &intervals,
                           const RealTime tMin, const RealTime tMax, QObject *parent) :
    AnnotationTier(parent)
{
    m_name = name;
    if (intervals.count() == 0) { // special case
        m_tMin = tMin;
        m_tMax = tMax;
        m_intervals << new Interval(tMin, tMax, ""); // initial blank interval
        return;
    }
    m_intervals = intervals;
    qSort(m_intervals.begin(), m_intervals.end(), IntervalTier::compareIntervals);
    m_tMin = qMin(m_intervals.first()->tMin(), tMin);
    m_tMax = qMax(m_intervals.last()->tMax(), tMax);
    fixEmptyIntervals();
}

// Deep copy constructor
// Makes an exact copy of another tier, deep copying the intervals.
// If name is not given, it will be the same as the original tier.
IntervalTier::IntervalTier(const IntervalTier *copy, QString name, bool copyAttributes, QObject *parent) :
    AnnotationTier(parent)
{
    if (!copy) {
        m_name = "error"; m_tMin = RealTime(0, 0); m_tMax = RealTime(1, 0);
        m_intervals << new Interval(m_tMin, m_tMax, ""); // initial blank interval
        return;
    }
    m_name = (name.isEmpty()) ? copy->name() : name;
    m_tMin = copy->tMin();
    m_tMax = copy->tMax();
    // deep copy of intervals
    foreach (Interval *intv, copy->intervals()) {
        m_intervals << new Interval(intv, copyAttributes);
    }
}

// Cartesian product constructor
// Combines two tiers, copying intervals from both and annotating overlaps.
IntervalTier::IntervalTier(const QString &name, const IntervalTier *tierA, const IntervalTier *tierB,
                           const QString &textA, const QString &textB, const QString &textAB,
                           QObject *parent) :
    AnnotationTier(parent)
{
    if (!tierA || !tierB) {
        m_name = "error"; m_tMin = RealTime(0, 0); m_tMax = RealTime(1, 0);
        m_intervals << new Interval(m_tMin, m_tMax, ""); // initial blank interval
        return;
    }
    m_name = name;
    m_tMin = qMin(tierA->tMin(), tierB->tMin());
    m_tMax = qMax(tierA->tMax(), tierB->tMax());
    m_intervals << new Interval(m_tMin, m_tMax, ""); // initial blank interval

    RealTime cursor = m_tMin;
    int i = 0, j = 0;
    while ((i < tierA->count()) && (j < tierB->count())) {
        RealTime tMax_A = tierA->interval(i)->tMax();
        RealTime tMax_B = tierB->interval(j)->tMax();
        bool active_A = !tierA->interval(i)->isPauseSilent();
        bool active_B = !tierB->interval(j)->isPauseSilent();
        QString text;
        if (tMax_A < tMax_B) {
            cursor = tMax_A; i++;
        } else if (tMax_A > tMax_B) {
            cursor = tMax_B; j++;
        }
        else {
            cursor = tMax_A; i++; j++;
        }
        if (active_A && active_B) {
            text = textAB;
        } else if (active_A && !active_B) {
            text = textA;
        } else if (!active_A && active_B) {
            text = textB;
        } // else empty
        this->addToEnd(cursor, text);
    }
}

IntervalTier::~IntervalTier()
{
    qDeleteAll(m_intervals);
}

// ==============================================================================================================================
// Implementation of AnnotationTier
// ==============================================================================================================================

bool IntervalTier::isEmpty() const
{
    if (m_intervals.count() == 1)
        return m_intervals.at(0)->text().isEmpty();
    // else
    return false;
}

void IntervalTier::clear()
{
    qDeleteAll(m_intervals);
    m_intervals.clear();
    m_intervals << new Interval(m_tMin, m_tMax, ""); // initial blank interval
}

Interval *IntervalTier::at(int index) const
{
    return m_intervals.at(index);
}

Interval *IntervalTier::first() const
{
    if (isEmpty()) return Q_NULLPTR;
    return m_intervals.first();
}

Interval *IntervalTier::last() const
{
    if (isEmpty()) return Q_NULLPTR;
    return m_intervals.last();
}

QStringList IntervalTier::getDistinctLabels() const
{
    QStringList ret;
    foreach (Interval *intv, m_intervals) {
        if (!ret.contains(intv->text())) ret << intv->text();
    }
    return ret;
}

QList<QVariant> IntervalTier::getDistinctValues(const QString &attributeID) const
{
    QList<QVariant> ret;
    foreach (Interval *intv, m_intervals) {
        QVariant value = (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID);
        if (!ret.contains(value)) ret << value;
    }
    return ret;
}

void IntervalTier::replace(const QString &attributeID, const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    foreach (Interval *intv, m_intervals) {
        if (attributeID.isEmpty())
            intv->replaceText(before, after, cs);
        else
            intv->replaceAttributeText(attributeID, before, after, cs);
    }
}

void IntervalTier::fillEmptyWith(const QString &attributeID, const QString &filler)
{
    foreach (Interval *intv, m_intervals) {
        if (attributeID.isEmpty()) {
            if (intv->text().isEmpty())
                intv->setText(filler);
        } else {
            if (intv->attribute(attributeID).toString().isEmpty())
                intv->setAttribute(attributeID, filler);
        }
    }
}

// ==============================================================================================================================
// Accessors and mutators for Intervals
// ==============================================================================================================================

Interval* IntervalTier::interval(int index) const
{
    return m_intervals.value(index);
}

QList<Interval *> IntervalTier::intervals() const
{
    return m_intervals;
}

bool IntervalTier::compareIntervals(Interval *A, Interval *B) {
    return (A->tMin() < B->tMin());
}

Interval *IntervalTier::intervalAtTime(RealTime t)
{
    int index = intervalIndexAtTime(t);
    if (index == -1)
        return 0; // not found
    return m_intervals.at(index);
}

int IntervalTier::intervalIndexAtTime(RealTime t) const
{
    if ((t < m_tMin) || (t > m_tMax)) return -1;
    // The intervals are sorted by time. Therefore, use binary search.
    int imin = 0;
    int imax = m_intervals.count() - 1;
    // continue searching while [imin, imax] is not empty
    while (imax >= imin) {
        // calculate the midpoint for roughly equal partition
        int imid = (imin + imax) / 2;
        // determine which subarray to search
        if (m_intervals.at(imid)->tMax() < t) {
            // change min index to search upper subarray
            imin = imid + 1;
        }
        else if (m_intervals.at(imid)->tMin() > t) {
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

void IntervalTier::copyIntervalsFrom(const IntervalTier *copy, bool copyData)
{
    if (!copy) return;
    m_tMin = copy->tMin();
    m_tMax = copy->tMax();
    // deep copy of intervals
    m_intervals.clear();
    foreach (Interval *intv, copy->intervals()) {
        if (copyData)
            m_intervals << new Interval(intv);
        else
            m_intervals << new Interval(intv->tMin(), intv->tMax(), "");
    }
}

void IntervalTier::replaceAllIntervals(QList<Interval *> &newIntervals)
{
    clear();
    m_intervals = newIntervals;
    if (m_intervals.count() == 0) {
        m_intervals << new Interval(m_tMin, m_tMax, ""); // initial blank interval
        return;
    }
    qSort(m_intervals.begin(), m_intervals.end(), IntervalTier::compareIntervals);
    fixEmptyIntervals();
    m_tMin = m_intervals.at(0)->m_tMin;
    m_tMax = m_intervals.at(m_intervals.count() - 1)->m_tMax;
}

Interval *IntervalTier::split(RealTime at)
{
    int index = intervalIndexAtTime(at);
    if (index <0) return 0;
    return split(index, at);
}

Interval *IntervalTier::split(int index, RealTime at)
{
    // check index
    if ((index < 0) || (index >= m_intervals.count())) return 0;
    Interval *intvOrig = m_intervals.at(index);
    // check that the split point is inside the interval
    if (!((intvOrig->tMin() < at) && (at < intvOrig->tMax()))) return 0;
    // create the two new intervals
    Interval *intvNewA = new Interval(intvOrig->tMin(), at, intvOrig->text());
    foreach (QString attributeID, intvOrig->attributes().keys())
        intvNewA->setAttribute(attributeID, intvOrig->attribute(attributeID));
    Interval *intvNewB = new Interval(at, intvOrig->tMax(), "");
    // and replace in the tier
    m_intervals.removeAt(index);
    m_intervals.insert(index, intvNewB);
    m_intervals.insert(index, intvNewA);
    return intvNewB;
}

Interval *IntervalTier::addToEnd(RealTime tMax, const QString &text)
{
    Interval *intv;
    intv = split(count() - 1, tMax);
    if (intv == 0) return 0; // problem with tMax
    intv = interval(count() - 2);
    intv->setText(text);
    return intv;
}

QList<Interval *> IntervalTier::splitToEqual(int index, int numberOfIntervals)
{
    QList<int> proportions;
    for (int i = 1; i <= numberOfIntervals; i++) proportions.append(1);
    return splitToProportions(index, proportions);
}

QList<Interval *> IntervalTier::splitToProportions(int index, const QList<int> &proportions)
{
    QList<Interval *> ret;
    // checks
    if ((index < 0) || (index >= m_intervals.count())) return ret;
    if (proportions.count() <= 1) return ret;
    int sumProportions = 0;
    foreach (int p, proportions) sumProportions += p;
    Interval *intvOrig = m_intervals.at(index);
    RealTime durOrig = intvOrig->duration();
    RealTime step = durOrig / sumProportions;
    RealTime tMin = intvOrig->tMin();
    for (int i = 1; i <= proportions.count(); i++) {
        RealTime tMax = (i < proportions.count()) ? tMin + step * proportions.at(i-1) : intvOrig->tMax();
        if (i == 1) {
            Interval *s = new Interval(tMin, tMax, intvOrig->text());
            foreach (QString attributeID, intvOrig->attributes().keys())
                s->setAttribute(attributeID, intvOrig->attribute(attributeID));
            ret << s;
        }
        else {
            ret << new Interval(tMin, tMax, "");
        }
        tMin = tMax;
    }
    m_intervals.removeAt(index);
    for (int i = proportions.count(); i >= 1; i--)
        m_intervals.insert(index, ret.at(i - 1));
    return ret;
}

Interval *IntervalTier::merge(int indexFrom, int indexTo, const QString &separator)
{
    // check indices
    if ((indexFrom < 0) || (indexFrom >= m_intervals.count())) return 0;
    if ((indexTo < 0) || (indexTo >= m_intervals.count())) return 0;
    QList<Interval *> list = m_intervals.mid(indexFrom, indexTo - indexFrom + 1);
    Interval *intvNew = new Interval(list, separator);
    for (int i = indexFrom; i <= indexTo; i++)
        m_intervals.removeAt(indexFrom);
    m_intervals.insert(indexFrom, intvNew);
    return intvNew;
}

void IntervalTier::mergeIdenticalAnnotations(const QString &text, const QStringList &ignoreIntervening)
{
    int right = m_intervals.count() - 1;
    while (right > 0) {
        int left = right - 1;
        QString A = m_intervals[left]->text();
        while ((!ignoreIntervening.isEmpty()) && (ignoreIntervening.contains(A)) && (left > 1)) {
            left--;
            A = m_intervals[left]->text();
        }
        QString B = m_intervals[right]->text();
        if (((A == text) && (A == B)) || (text.isEmpty() && (A == B))) {
            Interval *merged = merge(left, right);
            merged->setText(A);
        }
        right = left;
    }
}

void IntervalTier::mergeContiguousAnnotations(const QStringList &separatorsOfIntervals,
                                              const QString &separatorWhenMerging)
{
    int i = m_intervals.count() - 1;
    while (i > 0) {
        QString A = m_intervals[i - 1]->text();
        QString B = m_intervals[i]->text();
        if ((!separatorsOfIntervals.contains(A)) && (!separatorsOfIntervals.contains(B))) {
            merge(i - 1, i, separatorWhenMerging);
        }
        i--;
    }
}

QList<RealTime> IntervalTier::times() const
{
    QList<RealTime> ret;
    foreach (Interval *intv, m_intervals) ret << intv->tMin();
    return ret;
}

QList<RealTime> IntervalTier::timesMin() const
{
    return times();
}

QList<RealTime> IntervalTier::timesCenter() const
{
    QList<RealTime> ret;
    foreach (Interval *intv, m_intervals) ret << intv->tCenter();
    return ret;
}

QList<RealTime> IntervalTier::timesMax() const
{
    QList<RealTime> ret;
    foreach (Interval *intv, m_intervals) ret << intv->tMax();
    return ret;
}

void IntervalTier::timeShift(const RealTime delta)
{
    m_tMin = m_tMin + delta;
    m_tMax = m_tMax + delta;
    foreach (Interval *intv, m_intervals) {
        intv->m_tMin = intv->m_tMin + delta;
        intv->m_tMax = intv->m_tMax + delta;
    }
}

RealTime IntervalTier::getBoundaryClosestTo(const RealTime timePoint) const
{
    if (m_intervals.count() < 2)
        return RealTime(-1, 0); // to have boundaries you need at least two intervals
    RealTime ClosestBoundary = m_intervals[0]->tMin();
    foreach (Interval *intv, m_intervals) {
        if (RealTime::abs(timePoint - intv->tMin()) < RealTime::abs(timePoint - ClosestBoundary))
            ClosestBoundary = intv->tMin();
        if (RealTime::abs(timePoint - intv->tMax()) < RealTime::abs(timePoint - ClosestBoundary))
            ClosestBoundary = intv->tMax();
    }
    return ClosestBoundary;
}

void IntervalTier::fixBoundariesBasedOnTier(const IntervalTier *correctBoundariesTier, RealTime threshold)
{
    RealTime previous_tMax, next_tMin, proposed;
    for (int i = 0; i < m_intervals.count(); i++) {
        if (i - 1 >= 0)
            previous_tMax = m_intervals[i - 1]->tMax();
        else
            previous_tMax = RealTime(0, 0);
        if (i + 1 < m_intervals.count())
            next_tMin = correctBoundariesTier->getBoundaryClosestTo(m_intervals[i + 1]->tMin());
        else
            next_tMin = m_tMax; // of the tier
        // now correct
        proposed = correctBoundariesTier->getBoundaryClosestTo(m_intervals[i]->tMin());
        if ( (proposed >= previous_tMax) && (RealTime::abs(proposed - m_intervals[i]->tMin()) < threshold) &&
             (proposed < m_intervals[i]->tMax()) ) {
            m_intervals[i]->m_tMin = proposed;
        }
        proposed = correctBoundariesTier->getBoundaryClosestTo(m_intervals[i]->tMax());
        if ( (proposed <= next_tMin) && (RealTime::abs(proposed - m_intervals[i]->tMax()) < threshold) &&
             (proposed > m_intervals[i]->tMin()) ) {
            m_intervals[i]->m_tMax = proposed;
        }
    }
}

QList<Interval *> IntervalTier::getIntervalsContainedIn(const RealTime &timeStart, const RealTime &timeEnd) const
{
    Interval *container = new Interval(timeStart, timeEnd, "");
    QList<Interval *> ret = getIntervalsContainedIn(container);
    delete container;
    return ret;
}

QList<Interval *> IntervalTier::getIntervalsContainedIn(const Interval *container) const
{
    QList<Interval *> ret;
    foreach (Interval *intv, m_intervals) {
        if (container->contains(intv->tMin()) && container->contains(intv->tMax())) {
            ret << intv;
        }
    }
    return ret;
}

QPair<int, int> IntervalTier::getIntervalIndexesContainedIn(const RealTime &timeStart, const RealTime &timeEnd) const
{
    Interval *container = new Interval(timeStart, timeEnd, "");
    QPair<int, int> ret = getIntervalIndexesContainedIn(container);
    delete container;
    return ret;
}

QPair<int, int> IntervalTier::getIntervalIndexesContainedIn(const Interval *container) const
{
    int left = 0; int right = 0;
    RealTime center = (container->tMin() + container->tMax()) / 2.0;
    left = right = intervalIndexAtTime(center);
    while ((left - 1) >= 0 && (left - 1) < m_intervals.count() && container->contains(m_intervals[left - 1]->tMin()))
        left--;
    while ((right + 1) >= 0 && (right + 1) < m_intervals.count() && container->contains(m_intervals[right + 1]->tMax()))
        right++;
    return QPair<int, int>(left, right);
}

QList<Interval *> IntervalTier::getIntervalsOverlappingWith(const RealTime &timeStart, const RealTime &timeEnd) const
{
    Interval *contained = new Interval(timeStart, timeEnd, "");
    QList<Interval *> ret = getIntervalsOverlappingWith(contained);
    delete contained;
    return ret;
}

QList<Interval *> IntervalTier::getIntervalsOverlappingWith(const Interval *contained) const
{
    QList<Interval *> ret;
    foreach (Interval *intv, m_intervals) {
        if (intv->overlaps(*contained)) {
            ret << intv;
        }
    }
    return ret;
}

QList<Interval *> IntervalTier::getIntervalsOverlappingWith(const RealTime &timeStart, const RealTime &timeEnd, const RealTime &threshold) const
{
    Interval *contained = new Interval(timeStart, timeEnd, "");
    QList<Interval *> ret = getIntervalsOverlappingWith(contained, threshold);
    delete contained;
    return ret;
}

QList<Interval *> IntervalTier::getIntervalsOverlappingWith(const Interval *contained, const RealTime &threshold) const
{
    QList<Interval *> ret;
    foreach (Interval *intv, m_intervals) {
        if (intv->overlaps(*contained, threshold)) {
            ret << intv;
        }
    }
    return ret;
}

IntervalTier *IntervalTier::getIntervalTierSubset(const RealTime &timeStart, const RealTime &timeEnd) const
{
    QList<Interval *> intervals;
    foreach (Interval *intv, getIntervalsOverlappingWith(timeStart, timeEnd)) {
        intervals << new Interval(intv); // deep copy
    }
    IntervalTier *ret = new IntervalTier(name(), intervals, timeStart, timeEnd);
    if (ret->count() > 0) {
        ret->timeShift(-timeStart);
        if (ret->first()->tMin() < RealTime(0, 0) && ret->first()->tMax() > RealTime(0, 0))
            ret->first()->m_tMin = RealTime(0, 0);
        if (ret->last()->tMax() > (timeEnd - timeStart) && ret->last()->tMin() < (timeEnd - timeStart))
            ret->last()->m_tMax = (timeEnd - timeStart);
    }
    return ret;
}

PointTier *IntervalTier::getPointsMin(const QString &name, QObject *parent)
{
    PointTier *ret = new PointTier(name, m_tMin, m_tMax, parent);
    foreach (Interval *intv, m_intervals) {
        ret->addPoint(new Point(intv->tMin(), intv->text()));
    }
    return ret;
}

PointTier *IntervalTier::getPointsMax(const QString &name, QObject *parent)
{
    PointTier *ret = new PointTier(name, m_tMin, m_tMax, parent);
    foreach (Interval *intv, m_intervals) {
        ret->addPoint(new Point(intv->tMax(), intv->text()));
    }
    return ret;
}

QString IntervalTier::getIntervalsText(int indexStart, int indexEnd, QString separator)
{
    if (indexStart < 0 || indexStart >= m_intervals.count()) return QString();
    if (indexEnd < 0 || indexEnd >= m_intervals.count()) return QString();
    QString ret;
    for (int i = indexStart; i <= indexEnd; i++) {
        ret.append(m_intervals.at(i)->text()).append(separator);
    }
    if (separator.length() > 0)
        ret.chop(separator.length());
    return ret;
}

QString IntervalTier::getIntervalsText(RealTime timeStart, RealTime timeEnd, QString separator)
{
    int indexStart = this->intervalIndexAtTime(timeStart);
    int indexEnd = this->intervalIndexAtTime(timeEnd);
    return getIntervalsText(indexStart, indexEnd, separator);
}

void IntervalTier::setIOBAnnotationAttribute(const QString attribute, const IntervalTier *tierAnnotation)
{
    int indexAnnotationPrevious = -2;
    foreach(Interval *intv, m_intervals) {
        int indexAnnotation = tierAnnotation->intervalIndexAtTime(intv->tCenter());
        if (indexAnnotation < 0) continue;
        if (indexAnnotation == indexAnnotationPrevious) {
            intv->setAttribute(attribute, tierAnnotation->interval(indexAnnotation)->text() + "-I");
        }
        else {
            intv->setAttribute(attribute, tierAnnotation->interval(indexAnnotation)->text() + "-B");
        }
        indexAnnotationPrevious = indexAnnotation;
    }
}

QList<Interval *> IntervalTier::getContext(int index, int delta) const
{
    QList<Interval *> ret;
    if (index < 0 || index >= m_intervals.count()) return ret;
    if (delta == 0) return ret;
    if (delta < 0) {
        int left = index + delta; // adding a negative number
        if (left < 0) left = 0;
        for (int i = left; i < index; i++)
            ret << m_intervals.at(i);
    }
    else {
        int right = index + delta;
        if (right >= m_intervals.count()) right = m_intervals.count() - 1;
        for (int i = index + 1; i <= right; i++)
            ret << m_intervals.at(i);
    }
    return ret;
}

QList<Interval *> IntervalTier::getContext(int index, RealTime delta) const
{
    RealTime zero(0, 0);
    QList<Interval *> ret;
    if (index < 0 || index >= m_intervals.count()) return ret;
    if (delta == zero) return ret;
    if (delta < zero) {
        RealTime left = m_intervals.at(index)->tMin() + delta;
        int i = index - 1;
        while (i >= 0) {
            ret << m_intervals.at(i);
            if (m_intervals.at(i)->tMin() < left)
                break;
            i--;
        }
    }
    else {
        RealTime right = m_intervals.at(index)->tMax() + delta;
        int i = index + 1;
        while (i < m_intervals.count()) {
            ret << m_intervals.at(i);
            if (m_intervals.at(i)->tMax() > right)
                break;
            i++;
        }
    }
    return ret;
}

QString IntervalTier::getContextSymmetricFormated(int index, int delta, QString sep, QString left, QString right)
{
    if (index < 0 || index >= m_intervals.count()) return QString();
    QString ret;
    QList<Interval *> intervals_left = getContext(index, -delta);
    QList<Interval *> intervals_right = getContext(index, delta);
    foreach (Interval *intv, intervals_left)
        ret.append(intv->text()).append(sep);
    ret.append(left).append(m_intervals.at(index)->text()).append(right).append(sep);
    foreach (Interval *intv, intervals_right)
        ret.append(intv->text()).append(sep);
    if (!sep.isEmpty()) ret.chop(sep.length());
    return ret;
}


// private
void IntervalTier::fixEmptyIntervals()
{
    QList<Interval *> new_intervals;
    RealTime previous_tMax = m_tMin;

    foreach (Interval *intv, m_intervals) {
        if (previous_tMax < intv->tMin()) {
            new_intervals << new Interval(previous_tMax, intv->tMin(), "");
        }
        new_intervals << intv;
        previous_tMax = intv->tMax();
    }
    if (previous_tMax < m_tMax) {
        new_intervals << new Interval(previous_tMax, m_tMax, "");
    }
    m_intervals = new_intervals;
}

bool IntervalTier::moveBoundary(int index, RealTime time)
{
    // Moves boundary between intervals [index] and [index + 1] to time
    if ((index < 0) || (index > m_intervals.count() - 1)) return false;
    // Checks to maintain tier integrity
    // New boundary cannot move before the preceding interval's tMin
    if (index > 0) { if (time <= m_intervals.at(index - 1)->tMin()) return false; }
    // New boundary cannot move after the interval's tMax
    if (time > m_intervals.at(index)->tMax()) return false;
    // Move boundary
    if (index > 0) m_intervals[index - 1]->m_tMax = time;
    m_intervals[index]->m_tMin = time;
    return true;
}

bool IntervalTier::realignIntervals(int indexFrom, QList<RealTime> &newBoundaries)
{
    // Example:
    // indexFrom is 4, count of new boundaries 6, will change the boundaries of 5 intervals
    // boundary i is the new tMax of interval i and the new tMin of interval i+1
    // intervals   | 4 | 5 | 6 | 7 | 8 |
    // boundaries  3   4   5   6   7   8
    if (indexFrom < 0) return false;
    if (newBoundaries.count() < 2) return true;
    // Checks to maintain tier integrity
    // The first new tMin cannot be before the preceding Interval's tMin
    if (indexFrom > 0) { if (newBoundaries.first() <= m_intervals.at(indexFrom - 1)->tMin()) return false; }
    // The last new tMax cannot be after the following Interval's tMax
    int indexTo = indexFrom + newBoundaries.count() - 2;
    if (indexTo > m_intervals.count() - 1) indexTo = m_intervals.count() - 1;
    if (indexTo < m_intervals.count() - 1) { if (newBoundaries.last() >= m_intervals.at(indexTo + 1)->tMax()) return false; }
    // New boundaries should be in strictly ascending order
    for (int i = 1; i < newBoundaries.count(); ++i) {
        if (newBoundaries.at(i - 1) >= newBoundaries.at(i)) return false;
    }
    // If all is OK, then start realigning
    for (int i = 0; i < newBoundaries.count(); ++i) {
        int index = indexFrom + i;
        if (index > 0) m_intervals[index - 1]->m_tMax = newBoundaries[i];
        if (index < m_intervals.count()) m_intervals[index]->m_tMin = newBoundaries[i];
    }
    return true;
}

} // namespace Core
} // namespace Praaline
