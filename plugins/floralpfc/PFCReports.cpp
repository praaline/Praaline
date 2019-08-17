#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>
#include <QRegularExpression>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "pncore/statistics/WordAlign.h"
#include "PFCReports.h"

struct PFCReportsData {
    int i;
};

PFCReports::PFCReports() : d(new PFCReportsData)
{
}

PFCReports::~PFCReports()
{
    delete d;
}

QString PFCReports::corpusCoverageStatistics(QPointer<Corpus> corpus)
{
    QString outPerCom, outPivot;
    if (!corpus) return "No corpus";
    QString sep = "\t";
    // Pivot
    QMap<QString, QMap<QString, int> >    pivotCount;
    QMap<QString, QMap<QString, double> > pivotDurationTotal;
    QMap<QString, QMap<QString, double> > pivotDurationSingleSpk;
    // Header
    outPerCom.append("Region").append(sep).append("ID").append(sep).append("Task").append(sep);
    outPerCom.append("CountRecordings").append(sep).append("CountAnnotations").append(sep);
    outPerCom.append("RecordingDuration").append(sep).append("TranscriptionDuration").append(sep);
    outPerCom.append("CountSegmentsSingleSpk").append(sep).append("CountSegmentsOverlap").append(sep);
    outPerCom.append("DurationSegmentsSingleSpk").append(sep).append("PercentSingleSpk").append(sep);
    outPerCom.append("DurationSegmentsOverlap").append(sep).append("PercentOverlap").append(sep);
    outPerCom.append("CountSpeakers").append(sep).append("Speakers").append("\n");
    foreach (CorpusCommunication *com, corpus->communications()) {
        // Communication-Level properties: ID, region, Task
        QString ID = com->ID();
        QString region = ID.left(3);
        QString task;
        if      (ID.right(1) == "m") task = "WORDS";
        else if (ID.right(1) == "t") task = "TEXT";
        else if (ID.right(1) == "g") task = "GUIDED";
        else if (ID.right(1) == "l") task = "LIBRE";
        else                         task = "OTHER";
        com->setProperty("task", task);
        // Total duration of all recordings (normally there is only one associated with each com)
        RealTime totalRecordingDuration;
        int countRecordings = com->recordingIDs().count();
        foreach (CorpusRecording *rec, com->recordings()) {
            totalRecordingDuration = totalRecordingDuration + rec->duration();
        }
        // Number of annotations associated with this com (one or zero)
        int countAnnotations = com->annotationIDs().count();
        // Measures
        RealTime totalTranscriptionDuration;
        QMap<QString, RealTime> durationSingleSpkSegments;
        RealTime durationOverlaps, durationSingleSpk;
        int countSegmentsSingleSpk(0), countSegmentsOverlaps(0);
        // Statistics on interval durations
        foreach (CorpusAnnotation *annot, com->annotations()) {
            AnnotationTierGroup *tiers = com->repository()->annotations()->getTiers(annot->ID(), annot->ID(), QStringList() << "transcription");
            IntervalTier *transcription = tiers->getIntervalTierByName("transcription");
            if (!transcription) continue;
            foreach (Interval *intv, transcription->intervals()) {
                totalTranscriptionDuration = totalTranscriptionDuration + intv->duration();
                QString ortho = intv->text();
                QString schwa = intv->attribute("schwa").toString();
                QString liaison = intv->attribute("liaison").toString();
                int o = ortho.split(":").count();
                int s = schwa.split(":").count();
                int l = liaison.split(":").count();
                // Check for single speaker
                if ( ((o == s) && (o == l) && (o == 2)) || (!ortho.contains(":")) ) {
                    // Single speaker intervals are formatted like that: LOC: text text ...
                    QString spk = ortho.split(":").at(0);
                    if (!ortho.contains(":"))
                        spk = ID.mid(3, 3).toUpper();
                    if (durationSingleSpkSegments.contains(spk)) {
                        durationSingleSpkSegments[spk] = durationSingleSpkSegments[spk] + intv->duration();
                    } else {
                        durationSingleSpkSegments.insert(spk, intv->duration());
                    }
                    durationSingleSpk = durationSingleSpk + intv->duration();
                    countSegmentsSingleSpk++;
                }
                else {
                    durationOverlaps = durationOverlaps + intv->duration();
                    countSegmentsOverlaps++;
                }
            }
            // Clean up
            delete tiers;
            // Pivot
            if (task == "OTHER") continue;
            if (!pivotCount.contains(region)) {
                pivotCount.insert(region, QMap<QString, int>());
                pivotDurationTotal.insert(region, QMap<QString, double>());
                pivotDurationSingleSpk.insert(region, QMap<QString, double>());
            }
            pivotCount[region].insert(task, pivotCount[region].value(task, 0) + 1);
            pivotDurationTotal[region].insert(task, pivotDurationTotal[region].value(task, 0.0) + totalTranscriptionDuration.toDouble());
            pivotDurationSingleSpk[region].insert(task, pivotDurationSingleSpk[region].value(task, 0.0) + durationSingleSpk.toDouble());

        }
        // Write out data
        outPerCom.append(region).append(sep).append(ID).append(sep).append(task).append(sep);
        outPerCom.append(QString::number(countRecordings)).append(sep);
        outPerCom.append(QString::number(countAnnotations)).append(sep);
        outPerCom.append(QString::number(totalRecordingDuration.toDouble())).append(sep);
        outPerCom.append(QString::number(totalTranscriptionDuration.toDouble())).append(sep);
        outPerCom.append(QString::number(countSegmentsSingleSpk)).append(sep);
        outPerCom.append(QString::number(countSegmentsOverlaps)).append(sep);
        outPerCom.append(QString::number(durationSingleSpk.toDouble())).append(sep);
        outPerCom.append(QString::number(durationSingleSpk.toDouble() / totalTranscriptionDuration.toDouble())).append(sep);
        outPerCom.append(QString::number(durationOverlaps.toDouble())).append(sep);
        outPerCom.append(QString::number(durationOverlaps.toDouble() / totalTranscriptionDuration.toDouble())).append(sep);
        outPerCom.append(QString::number(durationSingleSpkSegments.keys().count())).append(sep);
        outPerCom.append(durationSingleSpkSegments.keys().join(", ")).append("\n");
    }
    // Pivot
    outPivot.append("Region").append(sep);
    foreach (QString task, pivotCount.first().keys()) outPivot.append(task).append(sep).append(sep).append(sep);
    outPivot.chop(3); outPivot.append("\n");
    foreach (QString region, pivotCount.keys()) {
        outPivot.append(region).append(sep);
        foreach (QString task, pivotCount[region].keys()) {
            outPivot.append(QString::number(pivotCount[region].value(task, 0))).append(sep);
            outPivot.append(QString::number(pivotDurationTotal[region].value(task, 0.0) / 60.0)).append(sep);
            if (pivotDurationTotal[region].value(task, 0.0) > 0.0)
                outPivot.append(QString::number(pivotDurationSingleSpk[region].value(task, 0.0) / pivotDurationTotal[region].value(task, 0.0))).append(sep);
            else
                outPivot.append(sep);
        }
        outPivot.chop(1);
        outPivot.append("\n");
    }
    return outPivot;
}

QString PFCReports::reportCorrections(Praaline::Core::CorpusCommunication *com)
{
    if (!com) return "No Communication";
    QString ret;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        AnnotationTierGroup *tiers = com->repository()->annotations()->
                getTiers(annot->ID(), annot->ID(), QStringList() << "transcription" << "transcription_original");
        IntervalTier *transcription = tiers->getIntervalTierByName("transcription");
        if (!transcription) continue;
        IntervalTier *transcription_original = tiers->getIntervalTierByName("transcription_original");
        if (!transcription_original) continue;
        if (transcription->count() != transcription_original->count())
            return com->ID() + " Changed number of transcription intervals!" ;
        for (int i = 0; i < transcription->count(); ++i) {
            Interval *original = transcription_original->at(i);
            Interval *modified = transcription->at(i);
            if (original->isPauseSilent() && modified->isPauseSilent()) continue;
            QStringList tokensOriginal = original->text().split(" ", QString::SkipEmptyParts);
            QStringList tokensModified = modified->text().split(" ", QString::SkipEmptyParts);
            QStringList liaisonOriginal = original->attribute("liaison").toString().split(" ", QString::SkipEmptyParts);
            QStringList liaisonModified = modified->attribute("liaison").toString().split(" ", QString::SkipEmptyParts);
            QStringList schwaOriginal = original->attribute("schwa").toString().split(" ", QString::SkipEmptyParts);
            QStringList schwaModified = modified->attribute("schwa").toString().split(" ", QString::SkipEmptyParts);
            WordAlign alignTokens;  alignTokens.align(tokensOriginal, tokensModified);
            WordAlign alignLiaison; alignLiaison.align(liaisonOriginal, liaisonModified);
            WordAlign alignSchwa;   alignSchwa.align(schwaOriginal, schwaModified);
            if (alignTokens.WER() > 0.0) {
                ret.append(com->ID()).append("\t").append(QString::number(i)).append("\t");
                ret.append(alignTokens.alignmentTextOriginal("latex")).append("\n");
                ret.append(alignTokens.alignmentTextModified("latex")).append("\n");
            }
        }
        delete tiers;
    }
    return ret;
}


