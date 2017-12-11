#ifndef EXPERIMENTUTTERANCES_H
#define EXPERIMENTUTTERANCES_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class ExperimentUtterances
{
public:
    ExperimentUtterances();

    static QString loadTranscriptions(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString align(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString concatenate(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString createUnitTier(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString averageProsody(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // EXPERIMENTUTTERANCES_H
