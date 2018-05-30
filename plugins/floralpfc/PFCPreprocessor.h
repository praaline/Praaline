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

    QString renameTextgridTiers(const QString& directory);
    QString prepareTranscription(QPointer<Praaline::Core::CorpusCommunication> com);
    QString checkSpeakers(QPointer<Praaline::Core::CorpusCommunication> com);
    QString separateSpeakers(QPointer<Praaline::Core::CorpusCommunication> com);
    QString tokenise(QPointer<Praaline::Core::CorpusCommunication> com);
    QString tokmin_punctuation(QPointer<Praaline::Core::CorpusCommunication> com);
    QString liaisonCoding(QPointer<Praaline::Core::CorpusCommunication> com);
    QString checkCharacterSet(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    PFCPreprocessorData *d;

    QString formatSegment(const QString &input);
    QList<QString> splitToken(const QString &input);
    bool startsWithPunctuation(const QString &text);
    bool endsWithPunctuation(const QString &text);
};

#endif // PFCPREPROCESSOR_H
