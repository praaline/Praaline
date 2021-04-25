#ifndef ANONYMISERSCRIPT_H
#define ANONYMISERSCRIPT_H

#include <QObject>
#include <QString>

namespace Praaline {
namespace Core {
class CorpusRecording;
class IntervalTier;
}
}

#include "AnnotationPluginPraatScript.h"

class AnonymiserScript : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit AnonymiserScript(QObject *parent = nullptr);

    double pitchAnalysisDuration;
    double pitchAnalysisTimestep;
    bool automaticMinMaxF0;
    double minF0;
    double maxF0;
    double scaleIntensity;

    void run(Praaline::Core::CorpusRecording *rec, Praaline::Core::IntervalTier *buzztier, QString anonymisedFilename);

signals:

};

#endif // ANONYMISERSCRIPT_H
