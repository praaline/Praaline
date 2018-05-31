#include <QString>

#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "SyllableDetectionForcedAligner.h"

namespace Praaline {
namespace ASR {

SyllableDetectionForcedAligner::SyllableDetectionForcedAligner(QObject *parent) :
    ForcedAligner(parent)
{
}

SyllableDetectionForcedAligner::~SyllableDetectionForcedAligner()
{
}

bool SyllableDetectionForcedAligner::alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                                                 IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                                                 bool insertLeadingAndTrailingPauses,
                                                 QList<Interval *> &outPhonesList, QString &outAlignerOutput)
{
    return false;
}

} // namespace ASR
} // namespace Praaline
