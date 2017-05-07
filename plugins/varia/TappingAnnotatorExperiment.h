#ifndef TAPPINGANNOTATOREXPERIMENT_H
#define TAPPINGANNOTATOREXPERIMENT_H

#include <QString>
#include <QXmlStreamReader>

namespace Praaline {
namespace Core {
class CorpusObject;
class CorpusSpeaker;
class CorpusRepository;
class IntervalTier;
}
}

class TappingAnnotatorExperiment
{
public:
    TappingAnnotatorExperiment();

    bool readResultsFile(Praaline::Core::CorpusRepository *repository, const QString &corpusID, const QString &filename);

private:
    Praaline::Core::IntervalTier *readTappingWithHold(QXmlStreamReader &xml);
    void readAttributesToCorpusObject(QXmlStreamReader &xml, CorpusObject *obj, QStringList attributes);
    bool readQuestionnaire(QXmlStreamReader &xml, Praaline::Core::CorpusSpeaker *participant);
};

#endif // TAPPINGANNOTATOREXPERIMENT_H
