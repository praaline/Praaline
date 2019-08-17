#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pnlib/media/AudioSegmenter.h"
using namespace Praaline::Media;

#include "SphinxPronunciationDictionary.h"
#include "SphinxAcousticModelTrainer.h"

namespace Praaline {
namespace ASR {

struct SphinxAcousticModelTrainerData
{
    SphinxAcousticModelTrainerData() {}

    QString levelUtterances, attributeUtterances;
    QString levelTokens, attributeTokens;
    SphinxPronunciationDictionary dictionary;
    QString outputPath;
    QStringList speakersInclude;
    QStringList speakersExclude;
};

SphinxAcousticModelTrainer::SphinxAcousticModelTrainer(QObject *parent) :
    QObject(parent), d(new SphinxAcousticModelTrainerData)
{
    d->levelTokens = "segment"; // default
    d->levelTokens = "tok_min"; // default
}

SphinxAcousticModelTrainer::~SphinxAcousticModelTrainer()
{
    delete d;
}

void SphinxAcousticModelTrainer::setTierUtterances(const QString &levelID, const QString &attributeID)
{
    d->levelUtterances = levelID;
    d->attributeUtterances = attributeID;
}

void SphinxAcousticModelTrainer::setTierTokens(const QString &levelID, const QString &attributeID)
{
    d->levelTokens = levelID;
    d->attributeTokens = attributeID;
}

void SphinxAcousticModelTrainer::setPronunciationDictionary(const SphinxPronunciationDictionary &dic)
{
    d->dictionary = dic;
}

void SphinxAcousticModelTrainer::setOutputPath(const QString &path)
{
    d->outputPath = path;
}

void SphinxAcousticModelTrainer::setSpeakersIncludeFilter(const QStringList &speakerIDs)
{
    d->speakersInclude = speakerIDs;
}

void SphinxAcousticModelTrainer::setSpeakersExcludeFilter(const QStringList &speakerIDs)
{
    d->speakersExclude = speakerIDs;
}

bool SphinxAcousticModelTrainer::createFiles(
        QList<CorpusCommunication *> &communications,
        QStringList &outUnknownWordsList, bool splitTrainTest, bool createSoundSegments)
{
    // Default path
    if (d->outputPath.isEmpty()) {
        d->outputPath = "./adapt";
    }
    // Create files
    QString filenameBase = d->outputPath + "/etc/";
    QFile fileFileIDsTrain, fileFileIDsTest, fileTranscriptionTrain, fileTranscriptionTest, fileUtterancesTrain, fileUtterancesTest;
    QTextStream outFileIDsTrain, outFileIDsTest, outTranscriptionTrain, outTranscriptionTest, outUtterancesTrain, outUtterancesTest;
    // File IDs file
    fileFileIDsTrain.setFileName(filenameBase + "train.fileids");
    if ( !fileFileIDsTrain.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    outFileIDsTrain.setDevice(&fileFileIDsTrain);
    outFileIDsTrain.setCodec("ISO 8859-1");
    if (splitTrainTest) {
        fileFileIDsTest.setFileName(filenameBase + "test.fileids");
        if ( !fileFileIDsTest.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
        outFileIDsTest.setDevice(&fileFileIDsTest);
        outFileIDsTest.setCodec("ISO 8859-1");
    }
    // Transcripts file
    fileTranscriptionTrain.setFileName(filenameBase + "train.transcription");
    if ( !fileTranscriptionTrain.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    outTranscriptionTrain.setDevice(&fileTranscriptionTrain);
    outTranscriptionTrain.setCodec("UTF-8");
    if (splitTrainTest) {
        fileTranscriptionTest.setFileName(filenameBase + "test.transcription");
        if ( !fileTranscriptionTest.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
        outTranscriptionTest.setDevice(&fileTranscriptionTest);
        outTranscriptionTest.setCodec("UTF-8");

    }
    // Create utterances file
    fileUtterancesTrain.setFileName(filenameBase + "train.utt");
    if ( !fileUtterancesTrain.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    outUtterancesTrain.setDevice(&fileUtterancesTrain);
    outUtterancesTrain.setCodec("UTF-8");
    if (splitTrainTest) {
        fileUtterancesTest.setFileName(filenameBase + "test.utt");
        if ( !fileUtterancesTest.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
        outUtterancesTest.setDevice(&fileUtterancesTest);
        outUtterancesTest.setCodec("UTF-8");
    }
    // Process corpus
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        if (!com->repository()) continue;
        if (!com->repository()->annotations()) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (!annot) continue;
                // Process recording-annotation pair, only if this annotation corresponds to the recording
                if ((!annot->recordingID().isEmpty()) && (annot->recordingID() != rec->ID())) continue;
                QString annotationID = annot->ID();
                SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
                foreach (QString speakerID, tiersAll.keys()) {
                    if ((!d->speakersInclude.isEmpty()) && (!d->speakersInclude.contains(speakerID))) continue;
                    if ((!d->speakersExclude.isEmpty()) && (d->speakersExclude.contains(speakerID))) continue;
                    AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                    if (!tiers) continue;
                    IntervalTier *tier_utterance = tiers->getIntervalTierByName(d->levelUtterances);
                    if (!tier_utterance) continue;
                    IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelTokens);
                    // Write transcriptions and fileids, potentially also split wav files
                    QList<Interval *> list_utterances;
                    int i = 0;
                    foreach (Interval *intv, tier_utterance->intervals()) {
                        // Check if utterance is empty
                        if (intv->isPauseSilent()) { i++; continue; }
                        QList<Interval *> tokens;
                        if (tier_tokens) {
                            tokens = tier_tokens->getIntervalsContainedIn(intv);
                            if (tokens.count() < 4) { i++; continue; }
                        }
                        QString utteranceID = QString("%1_%2_%3").arg(annotationID).arg(speakerID).arg(i);
                        // Check if the file exists unless we are creating it
                        if ((!createSoundSegments) && (!QFile::exists(d->outputPath + "/wav/" + utteranceID + ".wav")))
                            continue;
                        // ok to proceed
                        Interval *utt = intv->clone();
                        utt->setAttribute("utteranceID", utteranceID);
                        list_utterances << utt;
                        QString transcriptionLine, utteranceLine;
                        if (tier_tokens) {
                            transcriptionLine.append("<s> ");
                            utteranceLine.append("<s> ");
                            foreach (Interval *token, tokens) {
                                if (token->isPauseSilent()) continue;
                                QString t = (d->attributeTokens.isEmpty()) ? token->text() : token->attribute(d->attributeTokens).toString();
                                transcriptionLine.append(t).append(" ");
                                utteranceLine.append(t).append(" ");
                                if (!d->dictionary.contains(t)) outUnknownWordsList << t;
                            }
                            transcriptionLine.append("</s> (").append(utteranceID).append(")");
                            utteranceLine.append("</s>");
                        } else {
                            QString transcript = (d->attributeUtterances.isEmpty()) ? utt->text() : utt->attribute(d->attributeUtterances).toString();
                            transcript = transcript.replace("'", "' ");
                            foreach (QString word, transcript.split(" ")) {
                                if (!d->dictionary.contains(word)) outUnknownWordsList << word;
                            }
                            transcriptionLine.append("<s> ").append(transcript).append(" </s> (").append(utteranceID).append(")");
                            utteranceLine.append("<s> ").append(transcript).append(" </s>");
                        }
                        // print to files
                        if (splitTrainTest && (i % 10 == 0)) {
                            outFileIDsTest << utteranceID << "\n";
                            outTranscriptionTest << transcriptionLine << "\n";
                            outUtterancesTest << utteranceLine << "\n";
                        } else {
                            outFileIDsTrain << utteranceID << "\n";
                            outTranscriptionTrain << transcriptionLine << "\n";
                            outUtterancesTrain << utteranceLine << "\n";
                        }
                        i++; // proceed to next utterance
                    }
                    if (createSoundSegments) {
                        AudioSegmenter::segment(rec->filePath(), d->outputPath, list_utterances, "utteranceID", 16000);
                    }
                    qDeleteAll(list_utterances);
                }
                qDeleteAll(tiersAll);
                emit printMessage(QString("Created Sphinx transcription files for %1/%2").arg(com->ID()).arg(rec->ID()));
            }
        }
    }
    fileFileIDsTrain.close();
    fileTranscriptionTrain.close();
    fileUtterancesTrain.close();
    if (splitTrainTest) {
        fileFileIDsTest.close();
        fileTranscriptionTest.close();
        fileUtterancesTest.close();
    }
    return true;
}

} // namespace ASR
} // namespace Praaline
