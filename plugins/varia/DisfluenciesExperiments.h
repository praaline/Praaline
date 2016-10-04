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
    static bool resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus);

private:
    DisfluenciesExperiments() {}
};

#endif // DISFLUENCIESEXPERIMENTS_H
