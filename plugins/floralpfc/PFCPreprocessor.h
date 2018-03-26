#ifndef PFCPREPROCESSOR_H
#define PFCPREPROCESSOR_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class Interval;
}
}

struct PFCPreprocessorData;

class PFCPreprocessor
{
public:
    PFCPreprocessor();
    ~PFCPreprocessor();

    QString prepareTranscription(QPointer<Praaline::Core::CorpusCommunication> com);
    QString checkSpeakers(QPointer<Praaline::Core::CorpusCommunication> com);
    QString separateSpeakers(QPointer<Praaline::Core::CorpusCommunication> com);
    QString tokenise(QPointer<Praaline::Core::CorpusCommunication> com);
    QString tokmin_punctuation(QPointer<Praaline::Core::CorpusCommunication> com);
    QString liaisonCoding(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    PFCPreprocessorData *d;
};

#endif // PFCPREPROCESSOR_H
