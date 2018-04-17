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

    QString tokenPhonetisationAttributeID() const;
    void setTokenPhonetisationAttributeID(const QString &attributeID);

    bool alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                     Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                     bool insertLeadingAndTrailingPauses,
                     QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput);
    bool alignAllTokens(const QString &waveFilepath, Praaline::Core::IntervalTier *tierTokens,
                        QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput);
    bool alignUtterance(const QString &waveFilepath,
                        Praaline::Core::IntervalTier *tierUtterances, int &indexUtteranceToAlign,
                        Praaline::Core::IntervalTier *tierTokens, Praaline::Core::IntervalTier *tierPhones,
                        QString &outAlignerOutput, bool insertLeadingAndTrailingPauses = true);
    bool alignAllUtterances(const QString &waveFilepath,
                            Praaline::Core::IntervalTier *tierUtterances,
                            Praaline::Core::IntervalTier *tierTokens, Praaline::Core::IntervalTier *tierPhones,
                            bool insertLeadingAndTrailingPauses = true);
signals:
    void alignerMessage(QString);

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

    QList<SpeechToken> alignerTokensFromIntervalTier(bool insertLeadingAndTrailingPauses,
                                                     Praaline::Core::IntervalTier *tier_tokens, int indexFrom = 0, int indexTo = -1);
    void cleanUpTempFiles(const QString &waveResampledBase);
};

} // namespace ASR
} // namespace Praaline

#endif // HTKFORCEDALIGNER_H
