#ifndef SYLLABLEDETECTIONFORCEDALIGNER_H
#define SYLLABLEDETECTIONFORCEDALIGNER_H

#include <QObject>
#include "ForcedAligner.h"

namespace Praaline {
namespace ASR {

class SyllableDetectionForcedAligner : public ForcedAligner
{
    Q_OBJECT
public:
    explicit SyllableDetectionForcedAligner(QObject *parent = nullptr);
    ~SyllableDetectionForcedAligner();

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
