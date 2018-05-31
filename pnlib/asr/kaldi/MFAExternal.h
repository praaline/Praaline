#ifndef MFAEXTERNAL_H
#define MFAEXTERNAL_H

#include <QObject>
#include "ForcedAligner.h"

namespace Praaline {
namespace ASR {

struct MFAExternalData;

class MFAExternal : public ForcedAligner
{
    Q_OBJECT
public:
    explicit MFAExternal(QObject *parent = 0);
    ~MFAExternal();

    bool alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                     Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                     bool insertLeadingAndTrailingPauses,
                     QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput) override;

private:
    MFAExternalData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // MFAEXTERNAL_H
