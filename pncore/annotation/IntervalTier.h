#ifndef INTERVALTIER_H
#define INTERVALTIER_H

#include "pncore_global.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QVariant>
#include "base/RealTime.h"
#include "Interval.h"
#include "AnnotationTier.h"
#include "Point.h"
#include "PointTier.h"

namespace Praaline {
namespace Core {

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
    AnnotationTier::TierType tierType() const
        { return AnnotationTier::TierType_Intervals; }
    int count() const
        { return m_intervals.count(); }
    bool isEmpty() const;
    void clear();
    Interval *at(int index) const;
    Interval *first() const;
    Interval *last() const;
    QList<QString> getDistinctTextLabels() const;
    QList<QVariant> getDistinctAttributeValues(const QString &attributeID) const;
    void replaceTextLabels(const QString &before, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    void fillEmptyTextLabelsWith(const QString &filler);
    void replaceAttributeText(const QString &attributeID, const QString &before, const QString &after,
                              Qt::CaseSensitivity cs = Qt::CaseSensitive);
    void fillEmptyAttributeTextWith(const QString &attributeID,const QString &filler);

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
