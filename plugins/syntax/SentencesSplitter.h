#ifndef SENTENCESSPLITTER_H
#define SENTENCESSPLITTER_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct SentencesSplitterData;

class SentencesSplitter
{
public:
    SentencesSplitter();
    ~SentencesSplitter();

    QString exportSentences(QPointer<Praaline::Core::CorpusCommunication> com);
    QString readBreaksFile(const QString &filename);
    QString importBreaks(QPointer<Praaline::Core::CorpusCommunication> com);
    QString createSentenceTier(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    SentencesSplitterData *d;
};

#endif // SENTENCESSPLITTER_H
