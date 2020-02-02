#ifndef ANALYSERSEGMENTDURATION_H
#define ANALYSERSEGMENTDURATION_H

#include <QObject>
#include <QString>
#include <QList>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Statistics/StatisticalMeasureDefinition.h"

namespace Praaline {
namespace Core {
class Interval;
class IntervalTier;
}
}
using namespace Praaline::Core;

struct AnalyserSegmentDurationData;

class AnalyserSegmentDuration : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserSegmentDuration(QObject *parent = nullptr);
    virtual ~AnalyserSegmentDuration();

    QList<QString> measureIDs(const QString &groupingLevel);
    StatisticalMeasureDefinition measureDefinition(const QString &groupingLevel, const QString &measureID);

signals:

public slots:

private:
    AnalyserSegmentDurationData *d;
    IntervalTier *categorizePhones(IntervalTier *tier_phone);
    void calculateStatistics(const QList<Interval *> &intervals, const QString &name, RealTime threshold);

};

#endif // ANALYSERSEGMENTDURATION_H
