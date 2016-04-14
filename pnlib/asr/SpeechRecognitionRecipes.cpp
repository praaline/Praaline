#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QFile>
#include <QMutex>
#include "pncore/corpus/corpus.h"
#include "pncore/annotation/interval.h"
#include "pnlib/mediautil/AudioSegmenter.h"
#include "pnlib/asr/sphinx/SphinxFeatureExtractor.h"
#include "pnlib/asr/sphinx/SphinxRecogniser.h"
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
    QString filename = corpus->baseMediaPath();
    if (!filename.endsWith("/")) filename = filename.append("/");
    filename = filename.append(rec->filename());
    bool ret = AudioSegmenter::segment(filename, corpus->baseMediaPath(), list, QString(), 16000);
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

bool SpeechRecognitionRecipes::createSphinxFeatureFile(QPointer<Corpus> corpus, QPointer<CorpusRecording> rec,
                                                       SpeechRecognitionRecipes::Configuration config)
{
    if (!corpus) return false;
    if (!rec) return false;
    QStringList filenames;
    filenames << QString(rec->filename()).replace(".wav", ".16k.wav");
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

bool SpeechRecognitionRecipes::transcribeUtterancesWithSphinx(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                              QPointer<CorpusRecording> rec, const QString &annotationID,
                                                              const QString &tiernameSegmentation, const QString &tiernameTranscription,
                                                              const QString &pathAcousticModel, const QString &filenameLanguageModel,
                                                              const QString &filenamePronunciationDictionary, const QString &filenameMLLRMatrix)
{
    static QMutex mutex;
    if (!corpus) return false;
    if (!rec) return false;
    QPointer<SphinxRecogniser> sphinx = new SphinxRecogniser();
    // Configure Sphinx
    sphinx->setAcousticModel(pathAcousticModel);
    sphinx->setLanguageModel(filenameLanguageModel);
    sphinx->setPronunciationDictionary(filenamePronunciationDictionary);
    sphinx->setMLLRMatrix(filenameMLLRMatrix);
    // Transcribe
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    mutex.lock();
    tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
    mutex.unlock();
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        if (!tiers) continue;
        IntervalTier *tier_segment = tiers->getIntervalTierByName(tiernameSegmentation);
        if (!tier_segment) continue;
        QList<Interval *> utterances;
        QList<Interval *> segmentation;
        foreach (Interval *intv, tier_segment->intervals()) {
            if (intv->text() != "_") utterances << intv;
        }
        sphinx->recogniseUtterances_MFC(com, rec->ID(), utterances, segmentation);
        foreach (Interval *utterance, utterances) {
            qDebug() << utterance->text();
        }
        if (segmentation.count() > 1) {
            IntervalTier *tier_auto_transcribe = new IntervalTier(tiernameTranscription, segmentation,
                                                                  RealTime(0, 0), rec->duration());
            mutex.lock();
            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_auto_transcribe);
            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_segment);
            mutex.unlock();
        }
    }
    qDeleteAll(tiersAll);
    return true;
}

bool SpeechRecognitionRecipes::updateSegmentationFromTranscription(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                                   const QString &tiernameSegmentation, const QString &tiernameTranscription)
{
    static QMutex mutex;
    if (!corpus) return false;
    if (!com) return false;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        mutex.lock();
        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
        mutex.unlock();
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_transcription = tiers->getIntervalTierByName(tiernameTranscription);
            if (!tier_transcription) continue;
            IntervalTier *tier_segment = new IntervalTier(tier_transcription, tiernameSegmentation);
            tier_segment->fillEmptyAnnotationsWith("_");
            tier_segment->mergeIdenticalAnnotations("_");
            foreach (Interval *intv, tier_segment->intervals()) {
                QString text = intv->text();
                if ((text == "<sil>") || (text == "[b]") || (text == "[i]") || (text == "[mus]"))
                    intv->setText("");
                if (text.contains("("))
                    intv->setText(intv->text().remove(QRegExp("\\((.*)\\)")));
                if (intv->text() == "_" && intv->duration().toDouble() < 0.250)
                    intv->setText("");
            }
            tier_segment->mergeContiguousAnnotations(QStringList() << "_" << "<s>" << "</s>", " ");
            tier_segment->replaceText("<s>", "_");
            tier_segment->replaceText("</s>", "_");
            tier_segment->mergeIdenticalAnnotations("_");
            mutex.lock();
            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_segment);
            mutex.unlock();
        }
    }
    qDeleteAll(tiersAll);
    return true;
}


bool SpeechRecognitionRecipes::preapareAdapationData(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                                     const QString &levelTranscription, const QString &attributeIncludeInAdaptation,
                                                     const QStringList &speakerIDs,
                                                     const QString &pathAcousticModel, const QString &filenamePronunciationDictionary,
                                                     QStringList &outputTokensWithoutPhonetisationList)
{

    return true;
}
