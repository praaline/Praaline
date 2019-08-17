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
    QString prepareTranscription(Praaline::Core::CorpusCommunication *com);
    QString checkSpeakers(Praaline::Core::CorpusCommunication *com);
    QString separateSpeakers(Praaline::Core::CorpusCommunication *com);
    QString tokenise(Praaline::Core::CorpusCommunication *com);
    QString tokmin_punctuation(Praaline::Core::CorpusCommunication *com);
    QString liaisonCoding(Praaline::Core::CorpusCommunication *com);
    QString checkCharacterSet(Praaline::Core::CorpusCommunication *com);

private:
    PFCPreprocessorData *d;

    QString formatSegment(const QString &input);
    QList<QString> splitToken(const QString &input);
    bool startsWithPunctuation(const QString &text);
    bool endsWithPunctuation(const QString &text);
    bool isAcronym(const QString &text);
    bool tryFixingSpeakers(Praaline::Core::Interval *intv);
};

#endif // PFCPREPROCESSOR_H
