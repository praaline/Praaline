#ifndef SPEECHRATEESTIMATOR_H
#define SPEECHRATEESTIMATOR_H

#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/PointTier.h"

class SpeechRateEstimator
{
public:
    SpeechRateEstimator();

    static bool calculate(Praaline::Core::PointTier *measurements, Praaline::Core::IntervalTier *segments,
                          RealTime windowLeft, RealTime windowRight, const QString &attributeID);
};

#endif // SPEECHRATEESTIMATOR_H
