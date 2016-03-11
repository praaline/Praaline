#ifndef SPEECHRECOGNITIONRECIPES_H
#define SPEECHRECOGNITIONRECIPES_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

class Corpus;
class CorpusCommunication;
class CorpusRecording;

class SpeechRecognitionRecipes
{
public:
    struct Configuration {
        QString sphinxHMModelPath;
        QString sphinxPronunciationDictionary;
    };

    SpeechRecognitionRecipes();
    static bool downsampleWaveFile(QPointer<Corpus> corpus, QPointer<CorpusRecording> rec);
    static bool batchCreateSphinxFeatureFiles(QPointer<Corpus> corpus, QList<QPointer<CorpusCommunication> > &communications,
                                              Configuration config);
    static bool createSphinxFeatureFile(QPointer<Corpus> corpus, QPointer<CorpusRecording> rec,
                                        Configuration config);
    static bool transcribeUtterancesWithSphinx(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                               QPointer<CorpusRecording> rec, const QString &annotationID,
                                               const QString &tiernameSegmentation, const QString &tiernameTranscription,
                                               const QString &pathAcousticModel, const QString &filenameLanguageModel,
                                               const QString &filenamePronunciationDictionary, const QString &filenameMLLRMatrix);
    static bool updateSegmentationFromTranscription(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                    const QString &tiernameSegmentation, const QString &tiernameTranscription);

    // Adaptation
    static bool preapareAdapationData(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                      const QString &levelTranscription, const QString &attributeIncludeInAdaptation,
                                      const QStringList &speakerIDs,
                                      const QString &pathAcousticModel, const QString &filenamePronunciationDictionary,
                                      QStringList &outputTokensWithoutPhonetisationList);

};

#endif // SPEECHRECOGNITIONRECIPES_H
