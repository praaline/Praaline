#ifndef PROSODICBOUNDARIESEXPERIMENT_H
#define PROSODICBOUNDARIESEXPERIMENT_H

#include <QString>
#include <QPointer>
class Interval;
class IntervalTier;
class CorpusAnnotation;
class CorpusCommunication;
class Corpus;


class PBExpe
{
public:
    // Preparation
    static bool ipuIsMonological(Interval *ipu, IntervalTier *timeline, QString &speaker);
    static void measuresForPotentialStimuli(QPointer<CorpusAnnotation> annot, QList<Interval *> &stimuli, QTextStream &out,
                                            IntervalTier *tier_syll, IntervalTier *tier_tokmin);
    static void potentialStimuliFromSample(Corpus *corpus, QPointer<CorpusAnnotation> annot, QTextStream &out);
    static void potentialStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
    // Read out results
    static bool resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus);
    static bool resultsReadParticipantInfo(const QString &subjectID, const QString &expeType,
                                           const QString &filename, Corpus *corpus);
    // Analyse
    static void analysisCalculateDeltaRT(Corpus *corpus);
    static void analysisCreateAdjustedTappingTier(Corpus *corpus);
    static void analysisCalculateSmoothedTappingModel(Corpus *corpus);
    static void analysisAttributeTappingToSyllablesLocalMaxima(Corpus *corpus);
    static void analysisCalculateForce(Corpus *corpus);
    static void analysisCalculateAverageDelay(Corpus *corpus);
    static void analysisCalculateCoverage(Corpus *corpus);
    static void analysisFeaturesForModelling(Corpus *corpus);

private:
    PBExpe() {}
};

#endif // PROSODICBOUNDARIESEXPERIMENT_H
