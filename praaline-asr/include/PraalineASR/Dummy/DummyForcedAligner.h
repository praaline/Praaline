#ifndef DUMMYFORCEDALIGNER_H
#define DUMMYFORCEDALIGNER_H

#include <QObject>

#include "PraalineCore/Base/RealTime.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/ForcedAligner.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT DummyForcedAligner : public ForcedAligner
{
    Q_OBJECT
public:
    explicit DummyForcedAligner(QObject *parent = nullptr);
    ~DummyForcedAligner() override;

    bool alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                     Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                     bool insertLeadingAndTrailingPauses,
                     QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput) override;

signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // DUMMYFORCEDALIGNER_H
