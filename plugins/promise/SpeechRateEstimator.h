#ifndef SPEECHRATEESTIMATOR_H
#define SPEECHRATEESTIMATOR_H

#include "base/RealTime.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/PointTier.h"

class SpeechRateEstimator
{
public:
    SpeechRateEstimator();

    static bool calculate(Praaline::Core::PointTier *measurements, Praaline::Core::IntervalTier *segments,
                          RealTime windowLeft, RealTime windowRight, const QString &attributeID);
};

#endif // SPEECHRATEESTIMATOR_H
