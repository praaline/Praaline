#ifndef TIMELINETABLEMODELBASE_H
#define TIMELINETABLEMODELBASE_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QHash>
#include <QMap>
#include <QColor>
#include <QAbstractTableModel>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"

namespace Praaline {
namespace Core {
class AnnotationTierGroup;
}
}

class TimelineTableModelBase : public QAbstractTableModel
{
    Q_OBJECT

protected:
    struct TimelineData {
        QString speakerID;
        int index;
        RealTime tMin;
        RealTime tCenter;
        RealTime tMax;
    };

public:
    explicit TimelineTableModelBase(QObject *parent = nullptr);
    virtual ~TimelineTableModelBase();

    virtual int timelineIndexAtTime(const RealTime &time) const;
    virtual int timelineIndexAtTime(const RealTime &time, double &tMin_msec, double &tMax_msec) const;
    virtual RealTime timeAtTimelineIndex(int timelineIndex) const;
    virtual QModelIndex modelIndexAtTime(const RealTime &time) const;

    virtual RealTime getStartTime() const;
    virtual RealTime getEndTime() const;

signals:

public slots:

protected:
    QList<TimelineData> m_timeline;
    QHash<QString, QColor> m_speakerBackgroundColors;

    void calculateTimeline(Praaline::Core::SpeakerAnnotationTierGroupMap &tiers, const QString &timelineTier);
};

#endif // TIMELINETABLEMODELBASE_H
