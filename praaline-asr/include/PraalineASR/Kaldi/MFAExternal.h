#ifndef MFAEXTERNAL_H
#define MFAEXTERNAL_H

#include <QObject>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/ForcedAligner.h"

namespace Praaline {
namespace ASR {

struct MFAExternalData;

class PRAALINE_ASR_SHARED_EXPORT MFAExternal : public ForcedAligner
{
    Q_OBJECT
public:
    explicit MFAExternal(QObject *parent = nullptr);
    ~MFAExternal() override;

    bool alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                     Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                     bool insertLeadingAndTrailingPauses,
                     QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput) override;

    bool startDictionary();
    bool finishDictionary(const QString &filename);

    void setSpeakerID(const QString &speakerID);
    void setOutputPath(const QString &path);
    void setOutputWaveFiles(bool);

private:
    MFAExternalData *d;

    // Encode UTF entities (e.g. accented characters) to Latin1 for LAB files
    QString encodeEntities(const QString &src, const QString &force = QString());
    QString decodeEntities(const QString &src);
    QString translatePhonemes(const QString &input);
};

} // namespace ASR
} // namespace Praaline

#endif // MFAEXTERNAL_H
