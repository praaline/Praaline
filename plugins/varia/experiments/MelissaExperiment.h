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
    static QString exportSyntacticAnnotation(QPointer<Praaline::Core::CorpusCommunication> com);
    static void exportForEA(QPointer<Praaline::Core::CorpusCommunication> com);
    static void importPhonetisation(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString splitResponses(QPointer<Praaline::Core::CorpusCommunication> com);

    static QString exportForAlignment(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString reimportAlignment(QPointer<Praaline::Core::CorpusCommunication> com);

    static QString preprocessAlignment(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString processPausesInsertedByAligner(QPointer<Praaline::Core::CorpusCommunication> com);

    static QString reactionTimes(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // MELISSAEXPERIMENT_H
