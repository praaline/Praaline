#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QFile>
#include "pncore/corpus/corpus.h"
#include "pncore/annotation/interval.h"
#include "pnlib/AudioSegmenter.h"
#include "pnlib/asr/sphinx/SphinxFeatureExtractor.h"
#include "SpeechRecognitionRecipes.h"

SpeechRecognitionRecipes::SpeechRecognitionRecipes()
{

}

bool SpeechRecognitionRecipes::downsampleWaveFile(QPointer<Corpus> corpus, QPointer<CorpusRecording> rec)
{
    if (!corpus) return false;
    if (!rec) return false;
    QList<Interval *> list;
    list << new Interval(RealTime(0, 0), rec->duration(), rec->filename().replace(".wav", ".16k"));
    bool ret = AudioSegmenter::segment(corpus->baseMediaPath() + "/" + rec->filename(), corpus->baseMediaPath(), list, QString(), 16000);
    qDeleteAll(list);
    return ret;
}

bool SpeechRecognitionRecipes::batchCreateSphinxFeatureFiles(QPointer<Corpus> corpus, QList<QPointer<CorpusCommunication> > &communications,
                                                             SpeechRecognitionRecipes::Configuration config)
{
    if (!corpus) return false;
    QStringList filenames;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            filenames << QString(rec->filename()).replace(".wav", ".16k.wav");
        }
    }
    QPointer<SphinxFeatureExtractor> FE = new SphinxFeatureExtractor();
    FE->setFeatureParametersFile(config.sphinxHMModelPath + "/feat.params");
    FE->batchCreateSphinxMFC(corpus->baseMediaPath(), filenames);
    foreach (QString filename, filenames) {
        QString filenameFEOut = corpus->baseMediaPath() + "/" + filename.replace(".wav", ".mfc");
        QString filenameSimple = QString(filenameFEOut).replace(".16k.mfc", ".mfc");
        QFile::rename(filenameFEOut, filenameSimple);
    }
    return true;
}
