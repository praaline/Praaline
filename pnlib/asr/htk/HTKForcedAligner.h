#ifndef HTKFORCEDALIGNER_H
#define HTKFORCEDALIGNER_H

#include <QObject>
#include "pncore/annotation/intervaltier.h"
#include "SpeechToken.h"

struct HTKForcedAlignerData;

class HTKForcedAligner : public QObject
{
    Q_OBJECT
public:
    explicit HTKForcedAligner(QObject *parent = 0);
    ~HTKForcedAligner();

    void alignUtterances(QString waveFile, IntervalTier *tier_utterances, IntervalTier *tier_tokens, IntervalTier *tier_phones);

signals:

public slots:

private:
    HTKForcedAlignerData *d;
    QString translatePhonemes(QString input);
    bool createFilesDCTandLAB(const QString &filenameBase, QList<SpeechToken> &atokens);
    bool runAligner(const QString &filenameBase, QList<SpeechToken> &atokens, QList<SpeechPhone> &aphones);
};

#endif // HTKFORCEDALIGNER_H
