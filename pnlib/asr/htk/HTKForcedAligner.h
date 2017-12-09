#ifndef HTKFORCEDALIGNER_H
#define HTKFORCEDALIGNER_H

#include <QObject>
#include "pncore/annotation/IntervalTier.h"
#include "pnlib/asr/SpeechToken.h"

namespace Praaline {
namespace ASR {

struct HTKForcedAlignerData;

class HTKForcedAligner : public QObject
{
    Q_OBJECT
public:
    explicit HTKForcedAligner(QObject *parent = 0);
    ~HTKForcedAligner();

    bool alignUtterance(const QString &waveFile,
                        Praaline::Core::IntervalTier *tier_tokens,
                        QList<Praaline::Core::Interval *> &list_phones,
                        QString &alignerOutput);

    void alignUtterances(const QString &waveFile,
                         Praaline::Core::IntervalTier *tier_utterances,
                         Praaline::Core::IntervalTier *tier_tokens,
                         Praaline::Core::IntervalTier *tier_phones);

signals:

public slots:

private:
    HTKForcedAlignerData *d;
    QString translatePhonemes(const QString &input);
    bool createFilesDCTandLAB(const QString &filenameBase, QList<SpeechToken> &atokens);
    bool runAligner(const QString &filenameBase, QList<SpeechToken> &atokens, QList<SpeechPhone> &aphones,
                    QString &alignerOutput);

    // Encode UTF entities (e.g. accented characters) to Latin1 for LAB files
    QString encodeEntities(const QString &src, const QString &force = QString());
    QString decodeEntities(const QString &src);

    QList<SpeechToken> alignerTokensFromIntervalTier(Praaline::Core::IntervalTier *tier_tokens, int indexFrom = 0, int indexTo = -1);
};

} // namespace ASR
} // namespace Praaline

#endif // HTKFORCEDALIGNER_H
