#ifndef DISFLUENCIESEXPERIMENTS_H
#define DISFLUENCIESEXPERIMENTS_H

#include <QString>
#include <QPointer>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;


class DisfluenciesExperiments
{
public:
    static bool resultsReadTapping(const QString &sessionID, const QString &filename, Corpus *corpus);
    static void analysisCalculateDeltaRT(Corpus *corpus);
    static void analysisCreateAdjustedTappingTier(Corpus *corpus);

private:
    DisfluenciesExperiments() {}
};

#endif // DISFLUENCIESEXPERIMENTS_H
