#ifndef PROSODICBOUNDARIESEXPERIMENTPREPARATION_H
#define PROSODICBOUNDARIESEXPERIMENTPREPARATION_H

#include <QString>
#include <QPointer>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

class ProsodicBoundariesExperimentPreparation
{
public:
    // Preparation
    static bool ipuIsMonological(Interval *ipu, IntervalTier *timeline, QString &speaker);
    static void measuresForPotentialStimuli(QPointer<CorpusAnnotation> annot, QList<Interval *> &stimuli, QTextStream &out,
                                            IntervalTier *tier_syll, IntervalTier *tier_tokmin);
    static void potentialStimuliFromSample(Corpus *corpus, QPointer<CorpusAnnotation> annot, QTextStream &out);
    static void potentialStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
    static void actualStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);

private:
    ProsodicBoundariesExperimentPreparation() {}
};

#endif // PROSODICBOUNDARIESEXPERIMENTPREPARATION_H
