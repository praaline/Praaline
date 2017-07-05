#ifndef MELISSAEXPERIMENT_H
#define MELISSAEXPERIMENT_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class MelissaExperiment
{
public:
    MelissaExperiment();

    static QString multiplex(int participantNo);
    static void prepareStimuliCorpus(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // MELISSAEXPERIMENT_H
