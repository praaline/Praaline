#include <QString>

#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PraalineASR/Dummy/SyllableDetectionForcedAligner.h"

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
