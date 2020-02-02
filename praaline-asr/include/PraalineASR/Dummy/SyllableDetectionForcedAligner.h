#ifndef SYLLABLEDETECTIONFORCEDALIGNER_H
#define SYLLABLEDETECTIONFORCEDALIGNER_H

#include <QObject>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/ForcedAligner.h"

namespace Praaline {
namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SyllableDetectionForcedAligner : public ForcedAligner
{
    Q_OBJECT
public:
    explicit SyllableDetectionForcedAligner(QObject *parent = nullptr);
    ~SyllableDetectionForcedAligner() override;

    bool alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                     Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                     bool insertLeadingAndTrailingPauses,
                     QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput) override;
signals:

public slots:
};

} // namespace ASR
} // namespace Praaline

#endif // SYLLABLEDETECTIONFORCEDALIGNER_H
