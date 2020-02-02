#include <QString>

#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "PraalineASR/Dummy/DummyForcedAligner.h"

namespace Praaline {
namespace ASR {

DummyForcedAligner::DummyForcedAligner(QObject *parent) :
    ForcedAligner(parent)
{
}

DummyForcedAligner::~DummyForcedAligner()
{
}

bool DummyForcedAligner::alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                                     IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                                     bool insertLeadingAndTrailingPauses,
                                     QList<Interval *> &outPhonesList, QString &outAlignerOutput)
{
    return false;
}

} // namespace ASR
} // namespace Praaline
