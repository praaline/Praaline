#ifndef SENTENCE_H
#define SENTENCE_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class Sentence
{
public:
    Sentence();

    QString exportSentences(QPointer<Praaline::Core::CorpusCommunication> com);
    QString importBreaks(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // SENTENCE_H
