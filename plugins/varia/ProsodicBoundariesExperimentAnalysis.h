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
    static void analysisCreateAdjustedTappingTier(Corpus *corpus, const QString &tierName = "tapping");
    static void analysisCalculateSmoothedTappingModel(Corpus *corpus, int maxNumberOfSubjects = 0,  const QString &tierName = "tapping");
    static void analysisAttributeTappingToSyllablesLocalMaxima(Corpus *corpus, const QString &levelForUnits,
                                                               const QString &prefix, const QString &tierName = "tapping");
    static void calculateDelayAndDispersion(Corpus *corpus, QString prefix);
    static void analysisCalculateCoverage(Corpus *corpus, QString prefix);
    static void analysisStabilisation(Corpus *corpus, int maxNumberOfSubjects, int iterations, QString prefix);    
    static void analysisCheckBoundaryRightAfterPause(Corpus *corpus);

    // Statistics
    static void statExtractFeaturesForModelling(const QString &filename, Corpus *corpus, QString prefix, bool multilevel);
    static void statExtractFeaturesForModellingPerSubject(const QString &filename, Corpus *corpus, QString prefix, QStringList subjectIDs);
    static void statInterAnnotatorAgreement(const QString &filenameCohen, const QString &filenameFleiss, Corpus *corpus,
                                            const QString &prefix, const QString &tapping_level);
    static void statCorrespondanceNSandMS(const QString &filenameTable, const QString &filenameBookmarks, Corpus *corpus, QString prefix);
    static void statCorrespondanceInternal(const QString &filenameTable, const QString &filenameBookmarks, Corpus *corpus,
                                           const QString &prefixLeft, const QString &prefixRight, double limitLeft = 0.0, double limitRight = 0.0);

private:
    ProsodicBoundariesExperimentAnalysis() {}
};

#endif // PROSODICBOUNDARIESEXPERIMENT_H
