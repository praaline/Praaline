#ifndef OPENSMILEVAD_H
#define OPENSMILEVAD_H

#include <QObject>
#include <QString>
#include <QList>
#include <QPair>
#include "pncore/base/RealTime.h"

class CorpusRecording;
class IntervalTier;

class OpenSmileVAD : public QObject
{
    Q_OBJECT
public:
    explicit OpenSmileVAD(QObject *parent = 0);

    static bool runVAD(const QString &filenameInputWave, QList<QPair<double, double> > &resultVADActivation);
    static IntervalTier *splitToUtterances(QPointer<CorpusRecording> rec,
                                           RealTime minimumDurationSilent, RealTime minimumDurationVoice,
                                           QString textSilent, QString textVoice);
signals:

public slots:
};

#endif // OPENSMILEVAD_H
