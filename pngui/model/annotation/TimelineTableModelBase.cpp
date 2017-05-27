#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QHash>
#include <QMap>
#include <QColor>
#include <QAbstractTableModel>
#include "pncore/base/RealTime.h"

#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "TimelineTableModelBase.h"

TimelineTableModelBase::TimelineTableModelBase(QObject *parent) : QAbstractTableModel(parent)
{
}

TimelineTableModelBase::~TimelineTableModelBase()
{
}

void TimelineTableModelBase::calculateTimeline(QMap<QString, QPointer<AnnotationTierGroup> > &tiers, const QString &timelineTier)
{
    QMultiMap<RealTime, TimelineData> timeline;
    int i = 0;
    foreach (QString speakerID, tiers.keys()) {
        IntervalTier *tier = tiers.value(speakerID)->getIntervalTierByName(timelineTier);
        if (!tier) continue;
        for (int index = 0; index < tier->count(); ++index) {
            TimelineData td;
            td.speakerID = speakerID;
            td.index = index;
            td.tMin = tier->interval(index)->tMin();
            td.tCenter = tier->interval(index)->tCenter();
            td.tMax = tier->interval(index)->tMax();
            timeline.insert(td.tMin, td);
        }
        m_speakerBackgroundColors.insert(speakerID, QColor(QColor::colorNames().at(i)));
        ++i;
    }
    m_timeline = timeline.values();
}

int TimelineTableModelBase::timelineIndexAtTime(const RealTime &time) const
{
    int timelineIndex = 0;
    for (timelineIndex = 0; timelineIndex < m_timeline.count(); ++timelineIndex) {
        if (m_timeline.at(timelineIndex).tMin >= time) break;
    }
    // We found the first time index AFTER the time we should be moving to => subtract 1.
    if (timelineIndex > 0) timelineIndex--;
    return timelineIndex;
}

int TimelineTableModelBase::timelineIndexAtTime(const RealTime &time, double &tMin_msec, double &tMax_msec) const
{
    int timelineIndex = 0;
    for (timelineIndex = 0; timelineIndex < m_timeline.count(); ++timelineIndex) {
        if (m_timeline.at(timelineIndex).tMin >= time) break;
    }
    tMin_msec = m_timeline.at(timelineIndex).tMin.toDouble() * 1000.0;
    tMax_msec = m_timeline.at(timelineIndex).tMax.toDouble() * 1000.0;
    // Alternative method of calculation:
    // double tCenter_msec = m_timeline.at(timelineIndex).tCenter.toDouble() * 1000.0;
    // tMax_msec = 2.0 * tCenter_msec - tMin_msec;
    return timelineIndex;
}

RealTime TimelineTableModelBase::timeAtTimelineIndex(int timelineIndex) const
{
    if ((timelineIndex < 0) || (timelineIndex > m_timeline.count() - 1))
        return RealTime(-1, 0);
    return m_timeline.at(timelineIndex).tMin;
}

QModelIndex TimelineTableModelBase::modelIndexAtTime(const RealTime &time) const
{
    int timelineIndex = timelineIndexAtTime(time);
    return TimelineTableModelBase::index(timelineIndex, 0);
}

RealTime TimelineTableModelBase::getStartTime() const
{
    if (m_timeline.count() == 0) return RealTime();
    return m_timeline.first().tMin;
}

RealTime TimelineTableModelBase::getEndTime() const
{
    if (m_timeline.count() == 0) return RealTime();
    return m_timeline.last().tMax;
}
