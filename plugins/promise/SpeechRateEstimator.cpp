#include <QPointer>
#include <QString>
#include <QPair>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/query/Measures.h"
using namespace Praaline::Core;

#include "SpeechRateEstimator.h"

SpeechRateEstimator::SpeechRateEstimator()
{

}

bool SpeechRateEstimator::calculate(PointTier *measurements, IntervalTier *segments,
                                    RealTime windowLeft, RealTime windowRight, const QString &attributeID)
{
    if (!segments || !measurements) return false;
    foreach (Point *centre, measurements->points()) {
        QPair<int, int> win = Measures::window(segments, centre->time(), windowLeft, windowRight, false);
        if (win.first < 0 || win.second < 0) {
            centre->setAttribute(attributeID, 0.0);
            continue;
        }
        int count(0); RealTime dur;
        for (int i = win.first; i <= win.second; ++i) {
            if (!segments->interval(i)->isPauseSilent()) {
                count++;
                dur = dur + segments->interval(i)->duration();
            }
        }
        double rate = ((double)count) / dur.toDouble();
        centre->setAttribute(attributeID, rate);
    }
    return true;
}
