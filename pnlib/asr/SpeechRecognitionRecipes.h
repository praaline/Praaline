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
    };

    SpeechRecognitionRecipes();
    static bool downsampleWaveFile(QPointer<Corpus> corpus, QPointer<CorpusRecording> rec);
    static bool batchCreateSphinxFeatureFiles(QPointer<Corpus> corpus, QList<QPointer<CorpusCommunication> > &communications,
                                              Configuration config);

};

#endif // SPEECHRECOGNITIONRECIPES_H
