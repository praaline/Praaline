#ifndef DISFLUENCYEXPERIMENTS_H
#define DISFLUENCYEXPERIMENTS_H

#include <QString>
#include <QPointer>
class Interval;
class IntervalTier;
class CorpusAnnotation;
class CorpusCommunication;
class Corpus;

class DisfluencyExperiments
{
public:
    static bool resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus);

private:
    DisfluencyExperiments() {}
};

#endif // DISFLUENCYEXPERIMENTS_H
