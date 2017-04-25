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

namespace ASR {

class SpeechRecognitionRecipes
{
public:
    struct Configuration {
        QString sphinxHMModelPath;
        QString sphinxPronunciationDictionary;
    };

    SpeechRecognitionRecipes();
    static bool downsampleWaveFile(QPointer<Praaline::Core::CorpusRecording> rec, QString outputDirectory = QString());
    static bool batchCreateSphinxFeatureFiles(QList<QPointer<Praaline::Core::CorpusCommunication> > &communications,
                                              Configuration config);
    static bool createSphinxFeatureFile(QPointer<Praaline::Core::CorpusRecording> rec, Configuration config);
    static bool transcribeUtterancesWithSphinx(QPointer<Praaline::Core::CorpusCommunication> com,
                                               QPointer<Praaline::Core::CorpusRecording> rec, const QString &annotationID,
                                               const QString &tiernameSegmentation, const QString &tiernameTranscription,
                                               const QString &pathAcousticModel, const QString &filenameLanguageModel,
                                               const QString &filenamePronunciationDictionary, const QString &filenameMLLRMatrix);
    static bool updateSegmentationFromTranscription(QPointer<Praaline::Core::CorpusCommunication> com,
                                                    const QString &tiernameSegmentation, const QString &tiernameTranscription);

    // Adaptation
    static bool preapareAdapationData(QPointer<Praaline::Core::CorpusCommunication> com,
                                      const QString &levelTranscription, const QString &attributeIncludeInAdaptation,
                                      const QStringList &speakerIDs,
                                      const QString &pathAcousticModel, const QString &filenamePronunciationDictionary,
                                      QStringList &outputTokensWithoutPhonetisationList);
};

} // namespace ASR
} // namespace Praaline

#endif // SPEECHRECOGNITIONRECIPES_H
