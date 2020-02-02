#ifndef SPEECHRECOGNITIONRECIPES_H
#define SPEECHRECOGNITIONRECIPES_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {

namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
}

namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SpeechRecognitionRecipes
{
public:
    struct Configuration {
        QString sphinxHMModelPath;
        QString sphinxPronunciationDictionary;
        QString sphinxLanguageModelPath;
    };

    SpeechRecognitionRecipes();
    static bool downsampleWaveFile(Praaline::Core::CorpusRecording *rec, QString outputDirectory = QString());

    static bool createSphinxFeatureFiles(QList<Praaline::Core::CorpusCommunication *> &communications, Configuration config);
    static bool createSphinxFeatureFiles(QStringList filenames, SpeechRecognitionRecipes::Configuration config);
    static bool createSphinxFeatureFile(Praaline::Core::CorpusRecording *rec, Configuration config);

    static bool transcribeUtterancesWithSphinx(Praaline::Core::CorpusCommunication *com,
                                               Praaline::Core::CorpusRecording *rec, const QString &annotationID,
                                               const QString &tiernameSegmentation, const QString &tiernameTranscription,
                                               const QString &pathAcousticModel, const QString &filenameLanguageModel,
                                               const QString &filenamePronunciationDictionary, const QString &filenameMLLRMatrix);
    static bool updateSegmentationFromTranscription(Praaline::Core::CorpusCommunication *com,
                                                    const QString &tiernameSegmentation, const QString &tiernameTranscription);

    // Adaptation
    static bool preapareAdapationData(Praaline::Core::CorpusCommunication *com,
                                      const QString &levelTranscription, const QString &attributeIncludeInAdaptation,
                                      const QStringList &speakerIDs,
                                      const QString &pathAcousticModel, const QString &filenamePronunciationDictionary,
                                      QStringList &outputTokensWithoutPhonetisationList);
};

} // namespace ASR
} // namespace Praaline

#endif // SPEECHRECOGNITIONRECIPES_H
