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
    static void actualStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications);
    // Read out results
    static bool resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus);
    static bool resultsReadParticipantInfo(const QString &subjectID, const QString &expeType,
                                           const QString &filename, Corpus *corpus);
    // Analyse
    static void analysisCalculateDeltaRT(Corpus *corpus);
    static void analysisCreateAdjustedTappingTier(Corpus *corpus);
    static void analysisCalculateSmoothedTappingModel(Corpus *corpus, int maxNumberOfSubjects = 0);
    static void analysisAttributeTappingToSyllablesLocalMaxima(Corpus *corpus, QString levelForUnits, QString prefix);
    static void analysisCalculateAverageDelay(Corpus *corpus, QString prefix);
    static void analysisCalculateCoverage(Corpus *corpus, QString prefix);
    static void analysisStabilisation(Corpus *corpus, int maxNumberOfSubjects, int iterations, QString prefix);    
    static void analysisCheckBoundaryRightAfterPause(Corpus *corpus);
    static void createProsodicUnits(Corpus *corpus);
    static QStringList printTranscriptionInProsodicUnits(Corpus *corpus);

    // Statistics
    static void statExtractFeaturesForModelling(Corpus *corpus, QString prefix, bool multilevel);
    static void statInterAnnotatorAgreement(Corpus *corpus, QString prefix);
    static void statCorrespondanceNSandMS(Corpus *corpus, QString prefix);

private:
    PBExpe() {}
};

#endif // PROSODICBOUNDARIESEXPERIMENT_H
