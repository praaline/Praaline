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
    Q_UNUSED(waveFilepath)
    Q_UNUSED(timeFrom)
    Q_UNUSED(timeTo)
    Q_UNUSED(tierTokens)
    Q_UNUSED(indexFrom)
    Q_UNUSED(indexTo)
    Q_UNUSED(insertLeadingAndTrailingPauses)
    Q_UNUSED(outPhonesList)
    Q_UNUSED(outAlignerOutput)
    return false;
}

} // namespace ASR
} // namespace Praaline
