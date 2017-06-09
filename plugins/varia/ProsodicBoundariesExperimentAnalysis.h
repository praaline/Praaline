#ifndef PROSODICBOUNDARIESEXPERIMENT_H
#define PROSODICBOUNDARIESEXPERIMENT_H

#include <QString>
#include <QPointer>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

class ProsodicBoundariesExperimentAnalysis
{
public:

    // Read out results
    static bool resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus);
    static bool resultsReadParticipantInfo(const QString &subjectID, const QString &expeType,
                                           const QString &filename, Corpus *corpus);
    // Analyse
    static void analysisCalculateDeltaRT(Corpus *corpus);
    static void analysisCreateAdjustedTappingTier(Corpus *corpus);
    static void analysisCalculateSmoothedTappingModel(Corpus *corpus, int maxNumberOfSubjects = 0);
    static void analysisAttributeTappingToSyllablesLocalMaxima(Corpus *corpus, QString levelForUnits, QString prefix);
    static void calculateDelayAndDispersion(Corpus *corpus, QString prefix);
    static void analysisCalculateCoverage(Corpus *corpus, QString prefix);
    static void analysisStabilisation(Corpus *corpus, int maxNumberOfSubjects, int iterations, QString prefix);    
    static void analysisCheckBoundaryRightAfterPause(Corpus *corpus);
    static void createProsodicUnits(Corpus *corpus);
    static QStringList printTranscriptionInProsodicUnits(Corpus *corpus);

    // Statistics
    static void statExtractFeaturesForModelling(const QString &filename, Corpus *corpus, QString prefix, bool multilevel);
    static void statInterAnnotatorAgreement(Corpus *corpus, QString prefix);
    static void statCorrespondanceNSandMS(Corpus *corpus, QString prefix);

private:
    ProsodicBoundariesExperimentAnalysis() {}
};

#endif // PROSODICBOUNDARIESEXPERIMENT_H
