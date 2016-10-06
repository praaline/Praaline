#ifndef DISFLUENCIESEXPERIMENTS_H
#define DISFLUENCIESEXPERIMENTS_H

#include <QString>
#include <QPointer>
class Interval;
class IntervalTier;
class CorpusAnnotation;
class CorpusCommunication;
class Corpus;

class DisfluenciesExperiments
{
public:
    static bool resultsReadTapping(const QString &sessionID, const QString &filename, Corpus *corpus);
    static void analysisCalculateDeltaRT(Corpus *corpus);

private:
    DisfluenciesExperiments() {}
};

#endif // DISFLUENCIESEXPERIMENTS_H
