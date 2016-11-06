#ifndef SPEECHRECOGNITIONRECIPES_H
#define SPEECHRECOGNITIONRECIPES_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
}
}

class SpeechRecognitionRecipes
{
public:
    struct Configuration {
        QString sphinxHMModelPath;
        QString sphinxPronunciationDictionary;
    };

    SpeechRecognitionRecipes();
    static bool downsampleWaveFile(QPointer<Praaline::Core::Corpus> corpus, QPointer<Praaline::Core::CorpusRecording> rec);
    static bool batchCreateSphinxFeatureFiles(QPointer<Praaline::Core::Corpus> corpus,
                                              QList<QPointer<Praaline::Core::CorpusCommunication> > &communications,
                                              Configuration config);
    static bool createSphinxFeatureFile(QPointer<Praaline::Core::Corpus> corpus, QPointer<Praaline::Core::CorpusRecording> rec,
                                        Configuration config);
    static bool transcribeUtterancesWithSphinx(QPointer<Praaline::Core::Corpus> corpus,
                                               QPointer<Praaline::Core::CorpusCommunication> com,
                                               QPointer<Praaline::Core::CorpusRecording> rec, const QString &annotationID,
                                               const QString &tiernameSegmentation, const QString &tiernameTranscription,
                                               const QString &pathAcousticModel, const QString &filenameLanguageModel,
                                               const QString &filenamePronunciationDictionary, const QString &filenameMLLRMatrix);
    static bool updateSegmentationFromTranscription(QPointer<Praaline::Core::Corpus> corpus,
                                                    QPointer<Praaline::Core::CorpusCommunication> com,
                                                    const QString &tiernameSegmentation, const QString &tiernameTranscription);

    // Adaptation
    static bool preapareAdapationData(QPointer<Praaline::Core::Corpus> corpus,
                                      QPointer<Praaline::Core::CorpusCommunication> com,
                                      const QString &levelTranscription, const QString &attributeIncludeInAdaptation,
                                      const QStringList &speakerIDs,
                                      const QString &pathAcousticModel, const QString &filenamePronunciationDictionary,
                                      QStringList &outputTokensWithoutPhonetisationList);
};

#endif // SPEECHRECOGNITIONRECIPES_H
