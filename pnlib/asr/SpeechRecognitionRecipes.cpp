#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QFile>
#include <QMutex>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/Interval.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pnlib/media/AudioSegmenter.h"
#include "pnlib/asr/sphinx/SphinxFeatureExtractor.h"
#include "pnlib/asr/sphinx/SphinxRecogniser.h"
#include "SpeechRecognitionRecipes.h"

using namespace Praaline::Core;
using namespace Praaline::Media;

namespace Praaline {
namespace ASR {

SpeechRecognitionRecipes::SpeechRecognitionRecipes()
{

}

//bool SpeechRecognitionRecipes::downsampleWaveFile(QString filenameInput, QString filenameOutput, QString outputDirectory)
//{
//    QList<Interval *> list;
//    list << new Interval(RealTime(0, 0), rec->duration(), rec->filename().replace(".wav", ".16k"));
//    QString dir = (outputDirectory.isEmpty()) ? rec->basePath() : outputDirectory;
//    bool ret = AudioSegmenter::segment(rec->filePath(), dir, list, QString(), 16000);
//    qDeleteAll(list);
//    return ret;
//}


bool SpeechRecognitionRecipes::downsampleWaveFile(CorpusRecording *rec, QString outputDirectory)
{
    if (!rec) return false;
    if (!rec->repository()) return false;
    QList<Interval *> list;
    list << new Interval(RealTime(0, 0), rec->duration(), rec->filename().replace(".wav", ".16k"));
    QString dir = (outputDirectory.isEmpty()) ? rec->basePath() : outputDirectory;
    bool ret = AudioSegmenter::segment(rec->filePath(), dir, list, QString(), 16000);
    qDeleteAll(list);
    return ret;
}

bool SpeechRecognitionRecipes::createSphinxFeatureFiles(QList<CorpusCommunication *> &communications,
                                                        SpeechRecognitionRecipes::Configuration config)
{
    QStringList filenames;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            filenames << QString(rec->filePath()).replace(".wav", ".16k.wav");
        }
    }
    QPointer<SphinxFeatureExtractor> FE = new SphinxFeatureExtractor();
    FE->setFeatureParametersFile(config.sphinxHMModelPath + "/feat.params");
    FE->batchCreateSphinxMFC(filenames);
    foreach (QString filename, filenames) {
        QString filenameFEOut = QString(filename).replace(".wav", ".mfc");
        QString filenameSimple = QString(filenameFEOut).replace(".16k.mfc", ".mfc");
        QFile::rename(filenameFEOut, filenameSimple);
    }
    return true;
}

bool SpeechRecognitionRecipes::createSphinxFeatureFiles(QStringList filenames,
                                                        SpeechRecognitionRecipes::Configuration config)
{
    QPointer<SphinxFeatureExtractor> FE = new SphinxFeatureExtractor();
    FE->setFeatureParametersFile(config.sphinxHMModelPath + "/feat.params");
    FE->batchCreateSphinxMFC(filenames);
    return true;
}

bool SpeechRecognitionRecipes::createSphinxFeatureFile(CorpusRecording *rec,
                                                       SpeechRecognitionRecipes::Configuration config)
{
    if (!rec) return false;
    QStringList filenames;
    filenames << QString(rec->filePath()).replace(".wav", ".16k.wav");
    bool ret = createSphinxFeatureFiles(filenames, config);
    if (ret) {
        foreach (QString filename, filenames) {
            QString filenameFEOut = QString(filename).replace(".wav", ".mfc");
            QString filenameSimple = QString(filenameFEOut).replace(".16k.mfc", ".mfc");
            QFile::rename(filenameFEOut, filenameSimple);
        }
    }
    return ret;
}

bool SpeechRecognitionRecipes::transcribeUtterancesWithSphinx(CorpusCommunication *com,
                                                              CorpusRecording *rec, const QString &annotationID,
                                                              const QString &tiernameSegmentation, const QString &tiernameTranscription,
                                                              const QString &pathAcousticModel, const QString &filenameLanguageModel,
                                                              const QString &filenamePronunciationDictionary, const QString &filenameMLLRMatrix)
{
    static QMutex mutex;
    if (!com) return false;
    if (!com->repository()) return false;
    QPointer<SphinxRecogniser> sphinx = new SphinxRecogniser();
    // Configure Sphinx
    sphinx->setAcousticModel(pathAcousticModel);
    sphinx->setLanguageModel(filenameLanguageModel);
    sphinx->setPronunciationDictionary(filenamePronunciationDictionary);
    sphinx->setMLLRMatrix(filenameMLLRMatrix);
    // Transcribe
    SpeakerAnnotationTierGroupMap tiersAll;
    mutex.lock();
    tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
    mutex.unlock();
    foreach (QString speakerID, tiersAll.keys()) {
        AnnotationTierGroup *tiers = tiersAll.value(speakerID);
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
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_auto_transcribe);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_segment);
            mutex.unlock();
        }
    }
    qDeleteAll(tiersAll);
    return true;
}

bool SpeechRecognitionRecipes::updateSegmentationFromTranscription(CorpusCommunication *com,
                                                                   const QString &tiernameSegmentation, const QString &tiernameTranscription)
{
    static QMutex mutex;
    if (!com) return false;
    if (!com->repository()) return false;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        mutex.lock();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        mutex.unlock();
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_transcription = tiers->getIntervalTierByName(tiernameTranscription);
            if (!tier_transcription) continue;
            IntervalTier *tier_segment = tier_transcription->clone(tiernameSegmentation);
            tier_segment->fillEmptyWith("", "_");
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
            tier_segment->replace("", "<s>", "_");
            tier_segment->replace("", "</s>", "_");
            tier_segment->mergeIdenticalAnnotations("_");
            mutex.lock();
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_segment);
            mutex.unlock();
        }
    }
    qDeleteAll(tiersAll);
    return true;
}


bool SpeechRecognitionRecipes::preapareAdapationData(CorpusCommunication *com,
                                                     const QString &levelTranscription, const QString &attributeIncludeInAdaptation,
                                                     const QStringList &speakerIDs,
                                                     const QString &pathAcousticModel, const QString &filenamePronunciationDictionary,
                                                     QStringList &outputTokensWithoutPhonetisationList)
{

    return true;
}

} // namespace ASR
} // namespace Praaline
