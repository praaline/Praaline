#include <QDebug>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/FileDatastore.h"
using namespace Praaline::Core;

#include "pnlib/asr/htk/HTKForcedAligner.h"
using namespace  Praaline::ASR;

#include "ExperimentUtterances.h"

ExperimentUtterances::ExperimentUtterances()
{

}

QString ExperimentUtterances::loadTranscriptions(QPointer<CorpusCommunication> com)
{
    QString ret;

    // Import transcriptions (and possibly phonetisation) from file
    QHash<QString, QStringList> importedTranscriptions;
    QString path = "/media/george/Seagate Expansion Drive/TEST";
    QString line;
    QFile file(path + "/transcription.txt");
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return "Error reading transcriptions file";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        line = stream.readLine();
        if (line.startsWith("#")) continue;
        QStringList fields = line.split("\t");
        importedTranscriptions.insert(fields.at(0), fields);
    } while (!stream.atEnd());
    file.close();

    // Add transcription to the children of this communication
    if (!com) return "Error";
    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        if (!rec) continue;
        // Get utterance ID from metadata
        QString utteranceID = com->property("StimulusID").toString();
        QString annotationID = rec->ID();
        QString speakerID = com->property("SubjectID").toString();
        // Check whether a transcription for this utterance is contained in the imported transcriptions
        if (!importedTranscriptions.contains(utteranceID)) continue;
        // Find or create the corresponding annotation
        QPointer<CorpusAnnotation> annot = com->annotation(annotationID);
        if (!annot) {
            annot = new CorpusAnnotation(annotationID);
            com->addAnnotation(annot);
            com->save();
        }
        // Create a tier for importing the transcription
        QSharedPointer<IntervalTier> tier_trancsription(new IntervalTier("transcription", RealTime(0, 0), rec->duration()));
        // Pauses at beginning and end
        tier_trancsription->split(RealTime::fromMilliseconds(100));
        tier_trancsription->split(tier_trancsription->tMax() - RealTime::fromMilliseconds(100));
        if (tier_trancsription->count() < 3) continue;
        tier_trancsription->interval(0)->setText("_");
        tier_trancsription->interval(2)->setText("_");
        // Transcription
        tier_trancsription->interval(1)->setText(importedTranscriptions.value(utteranceID).at(1));
        tier_trancsription->interval(1)->setAttribute("phonetisation", importedTranscriptions.value(utteranceID).at(2));
        // Save tier
        com->repository()->annotations()->saveTier(annotationID, speakerID, tier_trancsription.data());
        ret.append(QString("%1\t%2\t%3").arg(com->ID()).arg(annotationID).arg(speakerID));
    }
    return ret;
}

#include "SyllabifierEasy.h"

QString ExperimentUtterances::align(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return "Error";
    QString annotationID = rec->ID();
    QString speakerID = com->property("SubjectID").toString();
    IntervalTier *tier_tokens = qobject_cast<IntervalTier *>
            (com->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
    if (!tier_tokens) return "No tier tokens";
    QList<Interval *> list_phones;
    QString alignerOutput;
    HTKForcedAligner aligner;
    bool ok = aligner.alignUtterance(rec->filePath(), tier_tokens, list_phones, alignerOutput);
    ret.append(QString("%1\t%2\t%3").arg(annotationID).arg(speakerID).arg((ok) ? "OK" : "Error"));
    if (!ok) {
        ret.append("\n").append(alignerOutput);
    }

    if (ok) {
        IntervalTier *tier_phone = new IntervalTier("phone", list_phones);
        IntervalTier *tier_syll= SyllabifierEasy::syllabify(tier_phone);
        tier_syll->setName("syll");
        com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tokens);
        com->repository()->annotations()->saveTier(annotationID, speakerID, tier_phone);
        com->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
    }

    delete tier_tokens;
    qDeleteAll(list_phones);

    return ret;
}

QString ExperimentUtterances::concatenate(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    QPointer<Corpus> corpus = com->corpus();
    if (!corpus) return "Error";

    // Group communications that will be concatenated
    QMap<QString, QStringList> concatenationGroups;
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        QString groupID = com->property("SubjectID").toString();
        if (!concatenationGroups.contains(groupID))
            concatenationGroups.insert(groupID, QStringList() << com->ID());
        else
            concatenationGroups[groupID] << com->ID();
    }
    foreach (QString groupID, concatenationGroups.keys()) {
        QStringList comIDs = concatenationGroups.value(groupID);
        qSort(comIDs);
        QString commandSoxConcatenate = "sox ";
        QSharedPointer<AnnotationTierGroup> tierGroupConcatenated(new AnnotationTierGroup);
        QList<Interval *> intervals_IDs;
        QStringList tierNames; tierNames << "phone" << "syll" << "tok_min";
        QHash<QString, QList<Interval *> > intervals_annot;
        foreach (QString levelID, tierNames)
            intervals_annot.insert(levelID, QList<Interval *>());
        RealTime offset;
        foreach (QString comID, comIDs) {
            QPointer<CorpusCommunication> com = corpus->communication(comID);
            if (!com) continue;
            foreach (QPointer<CorpusRecording> rec, com->recordings()) {
                commandSoxConcatenate = commandSoxConcatenate.append(rec->filePath()).append(" ");
                // Get annotation for this recording
                RealTime maxDuration = rec->duration();
                QString annotationID = rec->ID();
                foreach (QString levelID, tierNames) {
                    IntervalTier *tier = qobject_cast<IntervalTier *>
                            (corpus->repository()->annotations()->getTier(annotationID, groupID, levelID));
                    tier->timeShift(offset);
                    intervals_annot[levelID] << tier->intervals();
                    if (maxDuration < tier->duration()) maxDuration = tier->duration();
                }
                intervals_IDs << new Interval(offset, offset + maxDuration, rec->ID());
                offset = offset + maxDuration;
            }
        }
        QString basePathForConcatenatedMedia = corpus->repository()->files()->basePath();
        commandSoxConcatenate = commandSoxConcatenate.append(basePathForConcatenatedMedia + "/" + groupID + ".wav");
        // ret.append(commandSoxConcatenate).append("\n");
        // Export textgrid
        foreach (QString levelID, tierNames) {
            IntervalTier *tier = new IntervalTier(levelID, intervals_annot[levelID]);
            tierGroupConcatenated->addTier(tier);
        }
        IntervalTier *tier_IDs = new IntervalTier("ID", intervals_IDs);
        tierGroupConcatenated->addTier(tier_IDs);
        PraatTextGrid::save(basePathForConcatenatedMedia + "/" + groupID + ".TextGrid", tierGroupConcatenated.data());
    }

    return ret;
}


