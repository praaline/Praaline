#ifndef NCCFR_H
#define NCCFR_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
}
}

class NCCFR
{
public:
    NCCFR();

    QString prepareTranscription(QPointer<Praaline::Core::CorpusCommunication> com);
    QString align(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // NCCFR_H
