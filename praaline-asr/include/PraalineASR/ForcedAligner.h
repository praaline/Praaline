#ifndef FORCEDALIGNER_H
#define FORCEDALIGNER_H

#include <QObject>

#include "PraalineCore/Base/RealTime.h"
#include "PraalineASR/PraalineASR_Global.h"
#include "PraalineASR/SpeechToken.h"

namespace Praaline {

namespace Core {
class IntervalTier;
class Interval;
}

namespace ASR {

struct ForcedAlignerData;

class PRAALINE_ASR_SHARED_EXPORT ForcedAligner : public QObject
{
    Q_OBJECT
public:
    explicit ForcedAligner(QObject *parent = nullptr);
    virtual ~ForcedAligner();

    QStringList phonemeset() const;
    void setPhonemeset(const QStringList &phonemes);

    bool usePronunciationVariants() const;
    void setUsePronunciationVariants(bool use);

    QString tokenPhonetisationAttributeID() const;
    void setTokenPhonetisationAttributeID(const QString &attributeID);

    QString phonetisationSeparatorForVariants() const;
    void setPhonetisationSeparatorForVariants(const QString &sep);

    QString phonetisationSeparatorForPhonemes() const;
    void setPhonetisationSeparatorForPhonemes(const QString &sep);

    virtual bool alignTokens(const QString &waveFilepath, RealTime timeFrom, RealTime timeTo,
                             Praaline::Core::IntervalTier *tierTokens, int &indexFrom, int &indexTo,
                             bool insertLeadingAndTrailingPauses,
                             QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput) = 0;

    virtual bool alignAllTokens(const QString &waveFilepath, Praaline::Core::IntervalTier *tierTokens,
                                QList<Praaline::Core::Interval *> &outPhonesList, QString &outAlignerOutput);
    virtual bool alignUtterance(const QString &waveFilepath,
                                Praaline::Core::IntervalTier *tierUtterances, int &indexUtteranceToAlign,
                                Praaline::Core::IntervalTier *tierTokens, Praaline::Core::IntervalTier *tierPhones,
                                QString &outAlignerOutput, bool insertLeadingAndTrailingPauses = true);
    virtual bool alignAllUtterances(const QString &waveFilepath,
                                    Praaline::Core::IntervalTier *tierUtterances,
                                    Praaline::Core::IntervalTier *tierTokens, Praaline::Core::IntervalTier *tierPhones,
                                    bool insertLeadingAndTrailingPauses = true);
signals:
    void alignerMessage(QString);

protected:
    QList<SpeechToken> alignerTokensFromIntervalTier(bool insertLeadingAndTrailingPauses,
                                                     Praaline::Core::IntervalTier *tierTokens,
                                                     int indexFrom = 0, int indexTo = -1);

private:
    ForcedAlignerData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // FORCEDALIGNER_H
