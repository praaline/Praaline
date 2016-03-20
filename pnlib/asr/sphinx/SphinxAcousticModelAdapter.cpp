#include <QObject>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "pncore/annotation/intervaltier.h"
#include "pnlib/sox/AudioSegmenter.h"

#include "SphinxAcousticModelAdapter.h"

struct SphinxAcousticModelAdapterData {
    QString adaptationPath;
    QString tiernameUtterances;
    QString tiernameTokens;
    QMultiHash<QString, QString> dictionary;
};

SphinxAcousticModelAdapter::SphinxAcousticModelAdapter(QObject *parent)
    : QObject(parent), d(new SphinxAcousticModelAdapterData)
{
    d->tiernameUtterances = "segment";
    d->tiernameTokens = "tok_min";
}

SphinxAcousticModelAdapter::~SphinxAcousticModelAdapter()
{
    delete d;
}

bool SphinxAcousticModelAdapter::loadDictionary(const QString &filename)
{
    d->dictionary.clear();
    QFile fileDic(filename);
    if ( !fileDic.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream dic(&fileDic);
    dic.setCodec("UTF-8");
    while (!dic.atEnd()) {
        QString line = dic.readLine();
        if (!line.contains(" ")) continue;
        QString word = line.section(" ", 0, 0);
        QString phonetisation = line.section(" ", 1, -1);
        d->dictionary.insert(word, phonetisation);
    }
    fileDic.close();
    return true;
}

bool SphinxAcousticModelAdapter::unknownWordsInVocabFile(const QString &filename, QStringList &listUnknownWords)
{
    QFile fileVocab(filename);
    if ( !fileVocab.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    QTextStream vocab(&fileVocab);
    vocab.setCodec("UTF-8");
    while (!vocab.atEnd()) {
        QString word = vocab.readLine();
        if (word.startsWith("#") || word == "<s>" || word == "</s>") continue;
        if (!d->dictionary.contains(word))
            listUnknownWords << word;
    }
    return true;
}

bool SphinxAcousticModelAdapter::createAdaptationFiles(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications,
                                                       QStringList &listUnknownWords, bool segmentSound)
{
    if (d->adaptationPath.isEmpty()) {
        d->adaptationPath = corpus->baseMediaPath() + "/adapt";
    }

    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        CorpusRecording *rec = com->recordings().first();

        // Create fileids file
        QFile fileFileIDs(d->adaptationPath + "/" + com->ID() + ".fileids");
        if ( !fileFileIDs.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
        QTextStream fileIDs(&fileFileIDs);
        fileIDs.setCodec("ISO 8859-1");
        // Create transcripts file
        QFile fileTranscription(d->adaptationPath + "/" + com->ID() + ".transcription");
        if ( !fileTranscription.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
        QTextStream transcription(&fileTranscription);
        transcription.setCodec("UTF-8");

        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                if (speakerID.startsWith("(")) continue;
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_segment = tiers->getIntervalTierByName(d->tiernameUtterances);
                if (!tier_segment) continue;
                IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->tiernameTokens);
                // Split wav files and write transcriptions and fileids
                QList<Interval *> list_utterances;
                int i = 0;
                foreach (Interval *intv, tier_segment->intervals()) {
                    if (intv->isPauseSilent()) { i++; continue; }
                    Interval *utt = new Interval(intv);
                    QString spk = speakerID.replace(annotationID, "");
                    if (spk.startsWith("_")) spk = spk.remove(0, 1);
                    if (spk == "L2") continue;
                    QString utteranceID = QString("%1_%2_%3").arg(annotationID).arg(spk).arg(i);
                    utt->setAttribute("utteranceID", utteranceID);
                    list_utterances << utt;
                    i++;
                    fileIDs << utteranceID << "\n";
                    if (tier_tokens) {
                        transcription << "<s> ";
                        foreach (Interval *token, tier_tokens->getIntervalsContainedIn(intv)) {
                            transcription << token->text() << " ";
                            if (!d->dictionary.contains(token->text()))
                                listUnknownWords << token->text();
                        }
                        transcription << "</s> (" << utteranceID << ")\n";
                    } else {
                        QString tran = utt->text();
                        tran = tran.replace("'", "' ");
                        foreach (QString word, tran.split(" ")) {
                            if (!d->dictionary.contains(word))
                                listUnknownWords << word;
                        }
                        transcription << "<s> " << tran << " </s> (" << utteranceID << ")\n";
                    }
                }
                if (segmentSound) {
                    AudioSegmenter::segment(corpus->baseMediaPath() + "/" + rec->filename(),
                                            corpus->baseMediaPath() + "/adapt", list_utterances, "utteranceID", 16000);
                }
            }
        }
        qDeleteAll(tiersAll);
        emit printMessage(com->ID());
        fileFileIDs.close();
        fileTranscription.close();
    }
    return true;
}

