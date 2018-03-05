#include <QDebug>
#include <QString>
#include <QList>
#include <QPointer>
#include <QRegularExpression>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/phon/PhonTranscription.h"
using namespace Praaline::Core;


#include "StutteredSpeechAlignmentExperiments.h"

StutteredSpeechAlignmentExperiments::StutteredSpeechAlignmentExperiments()
{

}

void importPhonTranscriptionsIvana(const QList<QPointer<CorpusCommunication> > &communications)
{
    QString path = "/home/george/Dropbox/RECHERCHES_BEGAIEMENT/CORPUS_IVANA/";
    foreach (QPointer<CorpusCommunication> com, communications) {
        Corpus *corpus = com->corpus();
        if (!corpus) continue;
        PhonTranscription tr;
        tr.load(path + com->ID() + ".xml");
        QString annotationID = com->ID();
        QStringList participantIDs = tr.participantIDs();
        if (participantIDs.isEmpty()) participantIDs << "";
        QStringList speakerIDs;
        foreach (QString participantID, participantIDs) {
            QString speakerID = com->ID().left(5);
            if (!participantID.isEmpty()) speakerID = speakerID + "_" + participantID;
            speakerIDs << speakerID;
        }
        for (int i = 0; i < speakerIDs.count(); ++i) {
            QString speakerID = speakerIDs.at(i);
            QString participantID = participantIDs.at(i);
            // If speaker does not exist, add it
            if (!corpus->hasSpeaker(speakerID)) {
                CorpusSpeaker *spk = new CorpusSpeaker(speakerID);
                spk->setName(tr.sessionID());
                corpus->addSpeaker(spk);
                corpus->save();
            }
            QList<Interval *> intervals_transcription;
            for (int j = 0; j < tr.segments().count(); ++j) {
                PhonTranscription::Segment segment = tr.segments().at(j);
                if (segment.speakerID != participantID) continue;
                QString transcription_model = segment.orthography.join(" ");
                QString transcription = segment.groupTiers.value("Orthography Actual").join(" ");
                if (transcription.isEmpty()) transcription = transcription_model;
                QString ortho = transcription;
                ortho = ortho.remove(QRegularExpression("\\((.*)\\)"));
                ortho = ortho.remove("/").remove(".").remove("<").remove(">").remove("*");
                ortho = ortho.replace("  ", " ").replace("  ", " ");
                // Start and end times. Check sanity, Phon allows for small overlaps.
                RealTime tMin = segment.startTime;
                RealTime tMax = segment.startTime + segment.duration;
                if (j < tr.segments().count() - 1) {
                    if (tMax > tr.segments().at(j+1).startTime) {
                        tMax = tr.segments().at(j+1).startTime;
                    }
                }
                Interval *intv = new Interval(tMin, tMax, transcription);
                intv->setAttribute("transcription_model", transcription_model);
                intv->setAttribute("ortho", ortho);
                intervals_transcription << intv;
            }
            IntervalTier *tier_transcription = new IntervalTier("transcription", intervals_transcription);
            corpus->repository()->annotations()->saveTier(annotationID, speakerID, tier_transcription);
        }
    }
}

void prepareClassifierFiles(const QList<QPointer<CorpusCommunication> > &communications)
{
    QString path = "/home/george/Dropbox/RECHERCHES_BEGAIEMENT/CORPUS_DOUBLE_TACHE/";
    foreach (QPointer<CorpusCommunication> com, communications) {
        QString filenameIn = path + "/features/" + com->ID() + ".arff";
        QString filenameOut = path + "/features2/" + com->ID() + ".arff";
        QFile fileIn(filenameIn);
        if (!fileIn.open( QIODevice::ReadOnly | QIODevice::Text )) continue;
        QTextStream stream(&fileIn);
        QFile fileOut(filenameOut);
        if (!fileOut.open( QIODevice::WriteOnly | QIODevice::Text )) continue;
        QTextStream out(&fileOut);

        QString speakerID = com->ID().section("_", 0, 0);
        AnnotationTier *tier = com->corpus()->repository()->annotations()->getTier(com->ID(), speakerID, "disfluences_begues");
        IntervalTier *tier_disf = qobject_cast<IntervalTier *>(tier);
        if (!tier_disf) continue;

        do {
            QString line = stream.readLine();
            if (line.startsWith("@attribute class {")) {
                out << "@attribute class { garbage, block, lengthening }\n";
            }
            else if (line.startsWith("'file'")) {
                int frame = line.section(",", 1, 1).toInt();
                RealTime t = RealTime::fromMilliseconds(frame * 10);
                Interval *intv = tier_disf->intervalAtTime(t);
                if (intv && intv->text() == "pr") {
                    out << line.replace(",garbage", ",lengthening") << "\n";
                }
                else if (intv && intv->text() == "bl") {
                    out << line.replace(",garbage", ",block") << "\n";
                }
                else {
                    out << line << "\n";
                }
            }
            else {
                out << line << "\n";
            }
        } while (!stream.atEnd());

        fileIn.close();
        fileOut.close();
    }
}

