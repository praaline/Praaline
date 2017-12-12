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
#include "pncore/statistics/Measures.h"
#include "pncore/statistics/StatisticalSummary.h"
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
        QStringList tierNames; tierNames << "phone" << "syll" << "tok_min" << "prosodic_unit" << "sequence";
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

QString ExperimentUtterances::createUnitTier(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    // Import sequence trigrams
    QHash<QString, QStringList> trigrams;
    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/03_prosodie-relations-de-discours/Production";
    QString line;
    QFile file(path + "/sequences_trigrams.txt");
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return "Error reading trigrams file";
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    do {
        line = stream.readLine();
        if (line.startsWith("#")) continue;
        QStringList fields = line.split("\t");
        trigrams.insert(fields.at(0), fields);
    } while (!stream.atEnd());
    file.close();

    if (!com) return "Error";
    QPointer<Corpus> corpus = com->corpus();
    if (!corpus) return "Error";
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        QPointer<CorpusRecording> rec = com->recordings().first();
        if (!rec) continue;
        QString annotationID = rec->ID();
        QString speakerID = com->property("SubjectID").toString();
        IntervalTier *tier_tokens = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
        if (!tier_tokens) return "No tier tokens";
        IntervalTier *tier_units = new IntervalTier(tier_tokens, "sequence", false);
        QString stimulusID = com->property("StimulusID").toString();
        QString tokenBefore = trigrams[stimulusID].at(1);
        QString tokenTarget = trigrams[stimulusID].at(2);
        QString tokenAfter  = trigrams[stimulusID].at(3);
        bool found(false);
        for (int i = 1; i < tier_tokens->count() - 1; ++i) {
            QString target = tier_tokens->at(i)->text();
            if (target != tokenTarget) continue;
            int j = i - 1;
            while ((j > 0) && (tier_tokens->at(j)->isPauseSilent())) j--;
            QString before = tier_tokens->at(j)->text();
            if (before != tokenBefore) continue;
            int k = i + 1;
            while ((k < tier_tokens->count()) && (tier_tokens->at(k)->isPauseSilent())) k++;
            QString after = tier_tokens->at(k)->text();
            if (after != tokenAfter) continue;
            // found it
            tier_tokens->at(i)->setAttribute("target", "*");
            tier_units->merge(k, tier_tokens->count() - 2)->setText("S2");
            tier_units->at(i)->setText("MD");
            tier_units->merge(1, j)->setText("S1");
            found = true;
            break;
        }
        com->repository()->annotations()->saveTier(annotationID, speakerID, tier_units);
        ret.append(com->ID()).append(found ? " OK\n" : " Trigram not found\n");
        delete tier_tokens;
        delete tier_units;
    }
    return ret;
}

struct ProtoToken {
    ProtoToken(const QString &token) : token(token) {}
    QString token;
    QList<double> durations;
    QList<double> meanPitches;
};

QString ExperimentUtterances::averageProsody(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    QMap<QString, QList<ProtoToken> > utterances;
    if (!com) return "Error";
    QPointer<Corpus> corpus = com->corpus();
    if (!corpus) return "Error";
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        QPointer<CorpusRecording> rec = com->recordings().first();
        if (!rec) continue;
        QString annotationID = rec->ID();
        QString speakerID = com->property("SubjectID").toString();
        QString stimulusID = com->property("StimulusID").toString();
        IntervalTier *tier_tokens = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
        if (!tier_tokens) return "No tier tokens";
        IntervalTier *tier_syll = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "syll"));
        if (!tier_syll) return "No tier syll";
        // If this is the first time, construct the prototypical utterance
        if (!utterances.contains(stimulusID)) {
            QList<ProtoToken> prototokens;
            foreach (Interval *token, tier_tokens->intervals()) {
                if (token->isPauseSilent()) continue;
                prototokens << ProtoToken(token->text());
            }
            utterances.insert(stimulusID, prototokens);
        }
        // Interpolate syllable pitch values
        for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
            Interval *syll = tier_syll->interval(isyll);
            // Smoothing for non-stylised syllables
            if (!syll->isPauseSilent() && syll->attribute("f0_min").toInt() == 0) {
                syll->setAttribute("f0_min", Measures::mean(tier_syll, "f0_min", isyll, 4, 4, true, "f0_min"));
                syll->setAttribute("f0_max", Measures::mean(tier_syll, "f0_max", isyll, 4, 4, true, "f0_min"));
                syll->setAttribute("f0_mean", Measures::mean(tier_syll, "f0_mean", isyll, 4, 4, true, "f0_min"));
                syll->setAttribute("int_peak", Measures::mean(tier_syll, "int_peak", isyll, 4, 4, true, "f0_min"));
            }
        }
        // Process
        int i = 0;
        foreach (Interval *token, tier_tokens->intervals()) {
            if (token->isPauseSilent()) continue;
            QList<double> f0_means;
            QPair<int, int> syllIndices = tier_syll->getIntervalIndexesContainedIn(token);
            // smoothing
            while (syllIndices.second - syllIndices.first < 5) {
                if (syllIndices.first > 0) syllIndices.first--;
                if (syllIndices.second < tier_syll->count() - 1) syllIndices.second++;
            }
            for (int i = syllIndices.first; i <= syllIndices.second; ++i) {
                Interval *syll = tier_syll->interval(i);
                if (!syll) continue;
                if (syll->attribute("nucl_t1").toDouble() > 0 && syll->attribute("nucl_t2").toDouble() > 0) {
                    f0_means << syll->attribute("f0_mean").toDouble();
                }
            }
            StatisticalSummary summary_f0_means(f0_means);
            utterances[stimulusID][i].durations << token->duration().toDouble();
            utterances[stimulusID][i].meanPitches << summary_f0_means.mean();
            ++i;
        }
        delete tier_tokens;
        delete tier_syll;
    }
    // Output
    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/03_prosodie-relations-de-discours/Production Analyses";
    QFile file(path + "/averageprosody.txt");
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ) return "Error reading transcriptions file";
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "StimulusID\tDiscourseMarker\tDiscourseRelation\tToken\tTime\tDurationMean\tDurationStdev\tPitchMean\tPitchStdev\n";
    foreach (QString stimulusID, utterances.keys()) {
        QList<ProtoToken> tokens = utterances[stimulusID];
        double time(0.0);
        foreach (ProtoToken t, tokens) {
            StatisticalSummary summary_duration(t.durations);
            StatisticalSummary summary_pitch(t.meanPitches);
            out << stimulusID << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            out << t.token << "\t" << time << "\t";
            out << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            out << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
            time = time + summary_duration.mean();
        }
    }
    file.close();

    return ret;
}


