#ifndef DUMMYFORCEDALIGNER_H
#define DUMMYFORCEDALIGNER_H

#include <QObject>
#include "ForcedAligner.h"

namespace Praaline {
namespace ASR {

class DummyForcedAligner : public ForcedAligner
{
    Q_OBJECT
public:
    explicit DummyForcedAligner(QObject *parent = nullptr);
    ~DummyForcedAligner();

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
