#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFileInfo>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusRecording.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "MyExperiments.h"

MyExperiments::MyExperiments()
{

}

void MyExperiments::createTextgridsFromAutosyll(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    if (!corpus || !com) return;
    if (com->property("transcriptionMode").toString() != "mmm") return;
    QString outputPath = "/home/george/zeinab";

    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            QScopedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_autosyll = tiers->getIntervalTierByName("auto_syllables");
                QList<Interval *> pauses;
                foreach (Interval *intv, tier_autosyll->intervals()) {
                    if (intv->text() == "_") pauses << new Interval(intv);
                }
                bool subjectIDisOdd = (speakerID.remove("S").toInt() % 2 != 0);
                QString config = com->property("subjectConfig").toString();

                if (rec->ID().contains("DRIVER")) {
                    if ((subjectIDisOdd && (config == "A")) || (!subjectIDisOdd && (config == "B"))) {
                        QString tierID = QString("%1_%2").arg(speakerID).arg("DRIV");
                        IntervalTier *tier_utterances = new IntervalTier(tierID, pauses, tier_autosyll->tMin(),
                                                                         tier_autosyll->tMax());
                        txg->addTier(tier_utterances);
                    }
                } else {
                    if ((!subjectIDisOdd && (config == "A")) || (subjectIDisOdd && (config == "B"))) {
                        QString tierID = QString("%1_%2").arg(speakerID).arg("PASS");
                        IntervalTier *tier_utterances = new IntervalTier(tierID, pauses, tier_autosyll->tMin(),
                                                                         tier_autosyll->tMax());
                        txg->addTier(tier_utterances);
                    }
                }
            }
            QFile::copy(corpus->baseMediaPath() + "/" + rec->filename(), outputPath + "/" + rec->ID() + ".wav");
            PraatTextGrid::save(outputPath + "/" + rec->ID() + ".TextGrid", txg.data());
            qDeleteAll(tiersAll);
        }
    }
}

QString sampleIDtoSpeakerID(QString sampleID, QString role)
{
    int groupNo = sampleID.left(3).right(2).toInt();
    QString subject1 = QString("S%1").arg(groupNo * 2 - 1);
    QString subject2 = QString("S%1").arg(groupNo * 2);
    if (sampleID.right(1) == "A") {
        if (role == "DRIVER") return subject1; else if (role == "PASSENGER") return subject2;
    } else if (sampleID.right(1) == "B") {
        if (role == "DRIVER") return subject2; else if (role == "PASSENGER") return subject1;
    }
    return "";
}

void MyExperiments::createBasicMetadata(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    // 1234567890123456789012345678901234567890
    // G06A_RADIO1_DRIVER_01_SUMMARY
    QStringList fields = com->ID().split("_");
    com->setProperty("subtask", fields.at(1));
    if (fields.at(1) == "RADIO1" || fields.at(1) == "RADIO2") {
        com->setProperty("drivingCondition", "EASY");
    } else if (fields.at(1) == "RADIO3" || fields.at(1) == "RADIO4") {
        com->setProperty("drivingCondition", "DIFF");
    }
    com->setProperty("sectionOrder", fields.at(3).toInt());
    com->setProperty("section", fields.at(4));
    com->setProperty("subjectGroup", fields.at(0).left(3));
    com->setProperty("subjectConfig", fields.at(0).right(1));
    QString newID = fields.at(0) + "_" + fields.at(1) + "_" + fields.at(3) + "_" + fields.at(4);
    com->setProperty("newID", newID);
    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        rec->setProperty("speakerRole", fields.at(2));
        rec->setProperty("speakerID", sampleIDtoSpeakerID(fields.at(0), fields.at(2)));
        rec->setProperty("newComID", newID);
        rec->setProperty("newID", newID + "_" + fields.at(2));
    }
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        annot->setProperty("newID", newID);
        annot->setProperty("speakerID", sampleIDtoSpeakerID(fields.at(0), fields.at(2)));
    }
}

void MyExperiments::renameRecordings(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    if (!corpus || !com) return;
//    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
//        QString newFilename = rec->filename().section("/", 0, 0) + "/" + rec->ID() + ".wav";
//        if (QFile::rename(corpus->baseMediaPath() + "/" + rec->filename(), corpus->baseMediaPath() + "/" + newFilename)) {
//            rec->setFilename(newFilename);
//            rec->setName(rec->ID());
//        }
//    }
}

void MyExperiments::updateTranscriptionMode(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    if (!corpus || !com) return;

    com->setProperty("transcriptionMode", "A");
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
        QScopedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_transcription = tiers->getIntervalTierByName("transcription");
            if (tier_transcription) {
                com->setProperty("transcriptionMode", "M");
            }
        }
    }
}

void MyExperiments::mergePauses(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    if (!corpus || !com) return;

    foreach (QString annotationID, com->annotationIDs()) {
        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;

            QStringList tiernames; tiernames << "phone" << "syll" << "words" << "tok_min" << "tok_mwu" << "transcription";
            foreach (QString tiername, tiernames) {
                IntervalTier *tier = tiers->getIntervalTierByName(tiername);
                if (!tier) continue;
                tier->fillEmptyTextLabelsWith("_");
                tier->mergeIdenticalAnnotations("_");
                corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier);
            }
        }
        qDeleteAll(tiersAll);
    }
}


