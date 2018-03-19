#ifndef EXPERIMENTUTTERANCES_H
#define EXPERIMENTUTTERANCES_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
}
}

class ExperimentUtterances
{
public:
    ExperimentUtterances();

    static QString loadTranscriptions(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString align(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString syllabify(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString concatenate(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString createUnitTier(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString fixTiers(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString fixTranscription(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString rereadCorrectedTGs(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString resyllabifyMDs(QPointer<Praaline::Core::CorpusCommunication> com);

    static QString createStimuli(QPointer<Praaline::Core::Corpus> corpus);
    static QString createStimuli(QPointer<Praaline::Core::CorpusCommunication> comA,
                                 QPointer<Praaline::Core::CorpusCommunication> comB);

};

#endif // EXPERIMENTUTTERANCES_H
