#ifndef MACROPROSODYEXPERIMENT_H
#define MACROPROSODYEXPERIMENT_H

#include <QString>
#include <QPointer>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

class MacroprosodyExperiment
{
public:
    static void calculateZScoreForJoystickDataPerParticipant(const QList<QPointer<CorpusCommunication> > &communications, const QString &tierName);
    static void calculateZScoreForJoystickDataPerSampleAndParticipant(const QList<QPointer<CorpusCommunication> > &communications, const QString &tierName);
    static void createCombinedJoystickData(const QList<QPointer<CorpusCommunication> > &communications, const QString &tierName);


private:
    MacroprosodyExperiment() {}
};

#endif // MACROPROSODYEXPERIMENT_H
