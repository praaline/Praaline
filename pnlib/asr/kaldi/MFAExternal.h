#ifndef MFAEXTERNAL_H
#define MFAEXTERNAL_H

#include <QObject>
#include "pnlib/asr/ForcedAligner.h"

namespace Praaline {
namespace ASR {

struct MFAExternalData;

class MFAExternal : public ForcedAligner
{
    Q_OBJECT
public:
    explicit MFAExternal(QObject *parent = nullptr);
    ~MFAExternal();

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
