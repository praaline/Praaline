#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/FileDatastore.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/statistics/Measures.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "AnalyserTemporalItem.h"

struct AnalyserTemporalItemData {
    AnalyserTemporalItemData() : levelSyllables("syll"), levelTokens("tok_min")
    {
        filledPauseTokens << "euh" << "euhm";
    }

    QString levelSyllables;
    QString levelTokens;
    QStringList filledPauseTokens;
    // Data from the analysis of one Communication
    QHash<QString, double> measuresCom;
    QMap<QString, QHash<QString, double> > measuresSpk;
    QHash<QString, QList<double> > vectorsCom;
    QMap<QString, QHash<QString, QList<double> > > vectorsSpk;
};

AnalyserTemporalItem::AnalyserTemporalItem(QObject *parent) :
    QObject(parent), d(new AnalyserTemporalItemData)
{
}

AnalyserTemporalItem::~AnalyserTemporalItem()
{
    delete d;
}

QStringList AnalyserTemporalItem::measureIDsForCommunication()
{
    return QStringList()
            << "TimeTotalSample" << "TimeSingleSpeaker" << "TimeOverlap" << "TimeGap"
            << "RatioSingleSpeaker" << "RatioOverlap" << "RatioGap"
            << "GapDurations_Median" << "GapDurations_Q1" << "GapDurations_Q3"
            << "TurnChangesCount" << "TurnChangesCount_Gap" << "TurnChangesCount_Overlap"
            << "TurnChangesRate" << "TurnChangesRate_Gap" << "TurnChangesRate_Overlap";
}

QStringList AnalyserTemporalItem::measureIDsForSpeaker()
{
    return QStringList()
            << "TimeSpeech" << "TimeArticulation"
            << "TimeArticulation_Alone" << "TimeArticulation_Overlap" << "TimeArticulation_Overlap_Continue" << "TimeArticulation_Overlap_TurnChange"
            << "TimeSilentPause" << "TimeFilledPause"
            << "RatioArticulation" << "RatioArticulation_Alone"
            << "RatioArticulation_Overlap" << "RatioArticulation_Overlap_Continue" << "RatioArticulation_Overlap_TurnChange"
            << "RatioSilentPause" << "RatioFilledPause"
            << "NumTokens" << "NumSyllables" << "NumSilentPauses" << "NumFilledPauses"
            << "SpeechRate" << "SilentPauseRate" << "FilledPauseRate" << "ArticulationRate"
            << "PauseDur_SIL_Median" << "PauseDur_SIL_Q1" << "PauseDur_SIL_Q3" << "PauseDur_FIL_Median" << "PauseDur_FIL_Q1" << "PauseDur_FIL_Q3"
            << "TurnDuration_Time_Mean" << "TurnDuration_Syll_Mean" << "TurnDuration_Token_Mean";
}

QStringList AnalyserTemporalItem::vectorMeasureIDsForCommunication()
{
    return QStringList() << "OverlapDurations" << "GapDurations";
}

QStringList AnalyserTemporalItem::vectorMeasureIDsForSpeaker()
{
    return QStringList() << "Pause_SIL_Durations" << "Pause_FIL_Durations" << "TurnDurations";
}

StatisticalMeasureDefinition AnalyserTemporalItem::measureDefinition(const QString &measureID)
{
    // Measures per Communication
    if (measureID == "TimeTotalSample")          return StatisticalMeasureDefinition("TimeTotalSample", "Total sample time", "s");
    if (measureID == "TimeSingleSpeaker")        return StatisticalMeasureDefinition("TimeSingleSpeaker", "Single-speaker time", "s");
    if (measureID == "TimeOverlap")              return StatisticalMeasureDefinition("TimeOverlap", "Overlap time", "s");
    if (measureID == "TimeGap")                  return StatisticalMeasureDefinition("TimeGap", "Gap time", "s");
    if (measureID == "RatioSingleSpeaker")       return StatisticalMeasureDefinition("RatioSingleSpeaker", "Single-Speaker time ratio", "%", "Signle-speaker time / Total sample time");
    if (measureID == "RatioOverlap")             return StatisticalMeasureDefinition("RatioOverlap", "Overlap time ratio", "%", "Overlap time / Total sample time");
    if (measureID == "RatioGap")                 return StatisticalMeasureDefinition("RatioGap", "Gap time ratio", "%", "Gap time / Total sample time");
    if (measureID == "GapDurations_Median")      return StatisticalMeasureDefinition("GapDurations_Median", "Gap duration (median)", "s");
    if (measureID == "GapDurations_Q1")          return StatisticalMeasureDefinition("GapDurations_Q1", "Gap duration (Q1)", "s");
    if (measureID == "GapDurations_Q3")          return StatisticalMeasureDefinition("GapDurations_Q3", "Gap duration (Q3)", "s");
    if (measureID == "TurnChangesCount")         return StatisticalMeasureDefinition("TurnChangesCount", "Number of turn changes", "");
    if (measureID == "TurnChangesCount_Gap")     return StatisticalMeasureDefinition("TurnChangesCount_Gap", "Number of turn changes, without overlap", "");
    if (measureID == "TurnChangesCount_Overlap") return StatisticalMeasureDefinition("TurnChangesCount_Overlap", "Number of turn changes, with overlap", "");
    if (measureID == "TurnChangesRate")          return StatisticalMeasureDefinition("TurnChangesRate", "Turn changes per minute", "1/min");
    if (measureID == "TurnChangesRate_Gap")      return StatisticalMeasureDefinition("TurnChangesRate_Gap", "Turn changes per minute, without overlap", "1/min");
    if (measureID == "TurnChangesRate_Overlap")  return StatisticalMeasureDefinition("TurnChangesRate_Overlap", "Turn changes per minute, with overlap", "1/min");
    // Vectors per Communication
    if (measureID == "OverlapDurations")         return StatisticalMeasureDefinition("OverlapDurations", "Overlap durations", "s", "Durations of intervals of overlapping speech", QVariant::Double, true);
    if (measureID == "GapDurations")             return StatisticalMeasureDefinition("GapDurations", "Gap durations", "s", "Durations of gaps between speakers", QVariant::Double, true);
    // Measures per Speaker
    if (measureID == "TimeSpeech")                return StatisticalMeasureDefinition("TimeSpeech", "Speech time", "s", "Articulation Time + Silent Pause Time + Filled Pause Time");
    if (measureID == "TimeArticulation")          return StatisticalMeasureDefinition("TimeArticulation", "Articulation time", "s");
    if (measureID == "TimeArticulation_Alone")    return StatisticalMeasureDefinition("TimeArticulation_Alone", "Articulation, speaking alone time", "s");
    if (measureID == "TimeArticulation_Overlap")  return StatisticalMeasureDefinition("TimeArticulation_Overlap", "Articulation, overlap time", "s");
    if (measureID == "TimeArticulation_Overlap_Continue")   return StatisticalMeasureDefinition("TimeArticulation_Overlap_Continue", "Articulation, overlap without turn change, time", "s");
    if (measureID == "TimeArticulation_Overlap_TurnChange") return StatisticalMeasureDefinition("TimeArticulation_Overlap_TurnChange", "Articulation, overlap with turn change, time", "s");
    if (measureID == "TimeSilentPause") return StatisticalMeasureDefinition("TimeSilentPause", "Silent pause time", "s");
    if (measureID == "TimeFilledPause") return StatisticalMeasureDefinition("TimeFilledPause", "Filled pause time", "s");
    if (measureID == "RatioArticulation")         return StatisticalMeasureDefinition("RatioArticulation", "Articulation ratio", "%");
    if (measureID == "RatioArticulation_Alone")   return StatisticalMeasureDefinition("RatioArticulation_Alone", "Articulation, speaking alone ratio", "%");
    if (measureID == "RatioArticulation_Overlap") return StatisticalMeasureDefinition("RatioArticulation_Overlap", "Articulation, overlap ratio, ", "%");
    if (measureID == "RatioArticulation_Overlap_Continue")   return StatisticalMeasureDefinition("RatioArticulation_Overlap_Continue", "Articulation, overlap without turn change, ratio", "%");
    if (measureID == "RatioArticulation_Overlap_TurnChange") return StatisticalMeasureDefinition("RatioArticulation_Overlap_TurnChange", "Articulation, overlap with turn change, ratio", "%");
    if (measureID == "RatioSilentPause") return StatisticalMeasureDefinition("RatioSilentPause", "Silent pause ratio", "%");
    if (measureID == "RatioFilledPause") return StatisticalMeasureDefinition("RatioFilledPause", "Filled pause ratio", "%");
    if (measureID == "NumTokens")        return StatisticalMeasureDefinition("NumTokens", "Number of tokens", "");
    if (measureID == "NumSyllables")     return StatisticalMeasureDefinition("NumSyllables", "Number of syllables (articulated)", "");
    if (measureID == "NumSilentPauses")  return StatisticalMeasureDefinition("NumSilentPauses", "Number of silent pauses", "");
    if (measureID == "NumFilledPauses")  return StatisticalMeasureDefinition("NumFilledPauses", "Number of filled pauses", "");
    if (measureID == "SpeechRate")       return StatisticalMeasureDefinition("SpeechRate", "Speech rate", "syll/s", "All articulated syllables (excluding SIL and FIL) / Speech time");
    if (measureID == "SilentPauseRate")  return StatisticalMeasureDefinition("SilentPauseRate", "Silent pause rate", "SIL/s", "Number of silent pauses / Speech time");
    if (measureID == "FilledPauseRate")  return StatisticalMeasureDefinition("FilledPauseRate", "Filled pause rate", "FIL/s", "Number of filled pauses / Speech time");
    if (measureID == "ArticulationRate") return StatisticalMeasureDefinition("ArticulationRate", "Articulation rate", "syll/s", "All articulated syllables (excluding SIL and FIL) / Articulation time");
    if (measureID == "PauseDur_SIL_Median") return StatisticalMeasureDefinition("PauseDur_SIL_Median", "Silent pause duration (median)", "s");
    if (measureID == "PauseDur_SIL_Q1")     return StatisticalMeasureDefinition("PauseDur_SIL_Q1", "Silent pause duration (Q1)", "s");
    if (measureID == "PauseDur_SIL_Q3")     return StatisticalMeasureDefinition("PauseDur_SIL_Q3", "Silent pause duration (Q3)", "s");
    if (measureID == "PauseDur_FIL_Median") return StatisticalMeasureDefinition("PauseDur_FIL_Median", "Filled pause duration (median)", "s");
    if (measureID == "PauseDur_FIL_Q1")     return StatisticalMeasureDefinition("PauseDur_FIL_Q1", "Filled pause duration (Q1)", "s");
    if (measureID == "PauseDur_FIL_Q3")     return StatisticalMeasureDefinition("PauseDur_FIL_Q3", "Filled pause duration (Q3)", "s");
    if (measureID == "TurnDuration_Time_Mean")  return StatisticalMeasureDefinition("TurnDuration_Time_Mean", "Turn duration (mean)", "s");
    if (measureID == "TurnDuration_Syll_Mean")  return StatisticalMeasureDefinition("TurnDuration_Syll_Mean", "Number of syllables in turn (mean)", "syll");
    if (measureID == "TurnDuration_Token_Mean") return StatisticalMeasureDefinition("TurnDuration_Token_Mean", "Number of tokens in turn (mean)", "tokens");
    // Note: ArtRatio + SilRatio + FilRatio = 100%. Percentages calculated over the Speech time of current speaker)
    // Vectors per Speaker
    if (measureID == "Pause_SIL_Durations") return StatisticalMeasureDefinition("Pause_SIL_Durations", "Silent pause durations", "s", "Silent pause durations", QVariant::Double, true);
    if (measureID == "Pause_FIL_Durations") return StatisticalMeasureDefinition("Pause_SIL_Durations", "Filled pause durations", "s", "Filled pause durations", QVariant::Double, true);
    if (measureID == "TurnDurations")       return StatisticalMeasureDefinition("TurnDurations", "Turn durations", "s", "Durations of turns", QVariant::Double, true);
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

double AnalyserTemporalItem::measureCom(const QString &measureID) const
{
    return d->measuresCom.value(measureID, qQNaN());
}

double AnalyserTemporalItem::measureSpk(const QString &speakerID, const QString &measureID) const
{
    if (!d->measuresSpk.contains(speakerID)) return qQNaN();
    return d->measuresSpk.value(speakerID).value(measureID, qQNaN());
}

QList<double> AnalyserTemporalItem::vectorMeasureCom(const QString &measureID) const
{
    return d->vectorsCom.value(measureID);
}

QList<double> AnalyserTemporalItem::vectorMeasureSpk(const QString &speakerID, const QString &measureID) const
{
    if (!d->vectorsSpk.contains(speakerID)) return QList<double>();
    return d->vectorsSpk.value(speakerID).value(measureID);
}

QStringList AnalyserTemporalItem::speakerIDs() const
{
    return d->measuresSpk.keys();
}

QPair<IntervalTier *, IntervalTier *> getSpeakerTimeline(AnalyserTemporalItemData *d, QPointer<CorpusCommunication> com)
{
    QPair<IntervalTier *, IntervalTier *> ret(0, 0);
    if (!com) return ret;
    if (!com->repository()) return ret;

    // A timeline on the syllable level with detailed = true means there will be an interval for each syllable
    // and possibly segmented syllables when two speakers overlap.
    IntervalTier *tier_timelineSyll = com->repository()->annotations()->getSpeakerTimeline(com->ID(), "", d->levelSyllables, true);
    if (!tier_timelineSyll) return ret;

    // Exclude silence at the begining and end of recording
    int i = 0;
    while (i < tier_timelineSyll->count() && tier_timelineSyll->interval(i)->isPauseSilent()) {
        tier_timelineSyll->interval(i)->setAttribute("temporal", "X");
        i++;
    }
    i = tier_timelineSyll->count() - 1;
    while (i >= 0 && tier_timelineSyll->interval(i)->isPauseSilent()) {
        tier_timelineSyll->interval(i)->setAttribute("temporal", "X");
        i--;
    }

    // Create a timeline clone where regions of identical speakers are merged together
    IntervalTier *tier_timelineSpk = new IntervalTier(tier_timelineSyll);
    tier_timelineSpk->mergeIdenticalAnnotations();

    // Categorise intervals using an additional attribute on the timeline tier
    for (i = 0; i < tier_timelineSpk->count(); ++i) {
        Interval *intv = tier_timelineSpk->interval(i);
        if (intv->attribute("temporal").toString() == "X")
            continue; // excluded
        if (intv->isPauseSilent()) {
            QString speakerBeforePause, speakerAfterPause;
            if (i > 0) speakerBeforePause = tier_timelineSpk->interval(i - 1)->text();
            if (i < tier_timelineSpk->count() - 2) speakerAfterPause = tier_timelineSpk->interval(i + 1)->text();
            if (speakerAfterPause.contains(speakerBeforePause) && !speakerBeforePause.isEmpty() && !speakerAfterPause.isEmpty()) {
                intv->setText(speakerBeforePause); // silent pause belonging to the current speaker
                intv->setAttribute("temporal", "P");
            }
            else if (speakerBeforePause.isEmpty() || speakerAfterPause.isEmpty()) {
                intv->setAttribute("temporal", "X");
            } else {
                intv->setAttribute("temporal", "G");
            }
        }
        else {
            // One or many speakers speaking?
            if (intv->text().contains("+")) {
                // Detect overlaps with or without change of turn
                QString speakerBefore, speakerAfter;
                if (i > 0) speakerBefore = tier_timelineSpk->interval(i - 1)->text();
                if (i < tier_timelineSpk->count() - 2) speakerAfter = tier_timelineSpk->interval(i + 1)->text();
                if ((speakerBefore == speakerAfter) && (!speakerBefore.contains("+")) && (!speakerAfter.contains("+")))
                    intv->setAttribute("temporal", "OVC");
                else
                    intv->setAttribute("temporal", "OVT");
            } else {
                intv->setAttribute("temporal", "S");
            }
        }
        foreach (Interval *syllseg, tier_timelineSyll->getIntervalsContainedIn(intv)) {
            syllseg->setText(intv->text());
            syllseg->setAttribute("temporal", intv->attribute("temporal"));
        }
    }
    ret.first = tier_timelineSyll;
    ret.second = tier_timelineSpk;
    return ret;
}

void debugCreateTimelineTextgrid(AnalyserTemporalItemData *d, QPointer<CorpusCommunication> com)
{
    if (!com) return;
    if (!com->repository()) return;

    QPair<IntervalTier *, IntervalTier *> timelines = getSpeakerTimeline(d, com);
    if (!timelines.first || !timelines.second) return;
    IntervalTier *tier_timelineSyll(timelines.first);
    IntervalTier *tier_timelineSpk(timelines.second);

    QString path = com->repository()->files()->basePath();
    QScopedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            IntervalTier *tier_tokmin = tiers->getIntervalTierByName("tok_min");
            txg->addTier(new IntervalTier(tier_syll));
            txg->addTier(new IntervalTier(tier_tokmin));
            // Turn
            IntervalTier *tier_turns = new IntervalTier(tier_timelineSpk);
            foreach (Interval *intv, tier_turns->intervals()) {
                if (intv->text().contains(speakerID)) intv->setText(speakerID); else intv->setText("");
            }
            tier_turns->mergeIdenticalAnnotations();
            txg->addTier(tier_turns);
        }
        qDeleteAll(tiersAll);
    }
    txg->addTier(tier_timelineSyll);
    IntervalTier *tier_timelineSyllT = new IntervalTier(tier_timelineSyll, "timelineSyllT");
    foreach (Interval *intv, tier_timelineSyllT->intervals())
        intv->setText(intv->attribute("temporal").toString());
    txg->addTier(tier_timelineSyllT);
    txg->addTier(tier_timelineSpk);
    IntervalTier *tier_timelineSpkT = new IntervalTier(tier_timelineSpk, "timelineSpkT");
    foreach (Interval *intv, tier_timelineSpkT->intervals())
        intv->setText(intv->attribute("temporal").toString());
    txg->addTier(tier_timelineSpkT);

    PraatTextGrid::save(path + "/" + com->ID() + ".TextGrid", txg.data());
}

void AnalyserTemporalItem::analyse(QPointer<CorpusCommunication> com)
{
    if (!com) return;
    if (!com->repository()) return;

    d->measuresCom.clear();
    d->measuresSpk.clear();
    d->vectorsCom.clear();
    d->vectorsSpk.clear();

    // debugCreateTimelineTextgrid(d, com);

    QPair<IntervalTier *, IntervalTier *> timelines = getSpeakerTimeline(d, com);
    if (!timelines.first || !timelines.second) return;
    QScopedPointer<IntervalTier> tier_timelineSyll(timelines.first);
    QScopedPointer<IntervalTier> tier_timelineSpk(timelines.second);

    // Measures that can be calculated from the timeline, on the Communication level
    RealTime timeSingleSpeaker, timeOverlap, timeGap;
    int turnChangesWithGap(0), turnChangesWithOverlap(0);
    QList<double> gapDurations, overlapDurations;
    foreach (Interval *intv, tier_timelineSpk->intervals()) {
        QString temporal = intv->attribute("temporal").toString();
        if (temporal == "X") continue;
        if (temporal == "S" || temporal == "P") {
            timeSingleSpeaker = timeSingleSpeaker + intv->duration();
        }
        else if (temporal == "G") {
            gapDurations << intv->duration().toDouble();
            timeGap = timeGap + intv->duration();
            turnChangesWithGap++;
        }
        else {
            overlapDurations << intv->duration().toDouble();
            timeOverlap = timeOverlap + intv->duration();
            if (temporal == "OVT") turnChangesWithOverlap++;
        }
    }
    RealTime timeTotalSample = timeSingleSpeaker + timeOverlap + timeGap;
    if (timeTotalSample == RealTime(0, 0)) return;
    d->measuresCom.insert("TimeTotalSample", timeTotalSample.toDouble());
    d->measuresCom.insert("TimeSingleSpeaker", timeSingleSpeaker.toDouble());
    d->measuresCom.insert("TimeOverlap", timeOverlap.toDouble());
    d->measuresCom.insert("TimeGap", timeGap.toDouble());
    d->measuresCom.insert("RatioSingleSpeaker", timeSingleSpeaker.toDouble() / timeTotalSample.toDouble());
    d->measuresCom.insert("RatioOverlap", timeOverlap.toDouble() / timeTotalSample.toDouble());
    d->measuresCom.insert("RatioGap", timeGap.toDouble() / timeTotalSample.toDouble());
    StatisticalSummary summaryGapDurations;
    summaryGapDurations.calculate(gapDurations);
    d->measuresCom.insert("GapDurations_Median", summaryGapDurations.median());
    d->measuresCom.insert("GapDurations_Q1", summaryGapDurations.firstQuartile());
    d->measuresCom.insert("GapDurations_Q3", summaryGapDurations.thirdQuartile());
    d->measuresCom.insert("TurnChangesCount", (turnChangesWithGap + turnChangesWithOverlap));
    d->measuresCom.insert("TurnChangesCount_Gap", turnChangesWithGap);
    d->measuresCom.insert("TurnChangesCount_Overlap", turnChangesWithOverlap);
    // * 60.0 ==> convert to turns per minute
    d->measuresCom.insert("TurnChangesRate", ((double)(turnChangesWithGap + turnChangesWithOverlap)) * 60.0 / timeTotalSample.toDouble());
    d->measuresCom.insert("TurnChangesRate_Gap", ((double)turnChangesWithGap) * 60.0 / timeTotalSample.toDouble());
    d->measuresCom.insert("TurnChangesRate_Overlap", ((double)turnChangesWithOverlap) * 60.0 / timeTotalSample.toDouble());
    d->vectorsCom.insert("OverlapDurations", overlapDurations);
    d->vectorsCom.insert("GapDurations", gapDurations);

    foreach (QString annotationID, com->annotationIDs()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll =
                com->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList() << d->levelSyllables << d->levelTokens);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllables);
            IntervalTier *tier_tokmin = tiers->getIntervalTierByName(d->levelTokens);
            if (!tier_syll || !tier_tokmin) continue;
            // Per speaker measures
            RealTime timeSpeech, timeSilentPause, timeFilledPause;
            RealTime timeArticulationAlone, timeArticulationOverlapContinue, timeArticulationOverlapTurnChange;
            int numSilentPauses(0), numFilledPauses(0), numSyllablesArticulated(0), numTokens(0);
            QList<double> durationsPauseSIL, durationsPauseFIL;
            QList<double> durationsPauseSIL_rel1, durationsPauseSIL_rel2, durationsPauseSIL_rel3, durationsPauseSIL_rel4, durationsPauseSIL_rel5;
            QList<double> turnDurations, turnTokenCounts, turnArtSyllCounts;
            // Turn level
            QScopedPointer<IntervalTier> tier_turns(new IntervalTier(tier_timelineSpk.data()));
            foreach (Interval *intv, tier_turns->intervals()) {
                if (intv->text().contains(speakerID)) intv->setText(speakerID); else intv->setText("");
            }
            tier_turns->mergeIdenticalAnnotations();
            foreach (Interval *turn, tier_turns->intervals()) {
                if (!turn->text().contains(speakerID)) continue;
                turnDurations << turn->duration().toDouble();
                timeSpeech = timeSpeech + turn->duration();
                // Count non-pause, non-filled pause tokens in the turn
                int currentTurnTokenCount(0), currentTurnArtSyllCount(0);
                foreach (Interval *token, tier_tokmin->getIntervalsContainedIn(turn)) {
                    if ((!token->isPauseSilent()) && (!d->filledPauseTokens.contains(token->text()))) {
                        numTokens++;
                        currentTurnTokenCount++;
                    }
                }
                turnTokenCounts << ((double)currentTurnTokenCount);
                // The basic units of time measurement are the speaker's syllables
                int syllIndex(0);
                foreach (Interval *syll, tier_syll->getIntervalsContainedIn(turn)) {
                    QString syllCategory = ""; // SIL, FIL or ART?
                    QList<Interval *> tokens = tier_tokmin->getIntervalsOverlappingWith(syll);
                    // foreach (Interval *token, tokens) qDebug() << token->text();
                    if (tokens.count() == 1 && d->filledPauseTokens.contains(tokens.first()->text())) {
                        syllCategory = "FIL"; // This syllable is a filled pause
                    }
                    // Process sub-syllabic segments on the timeline
                    foreach (Interval *intv, tier_timelineSyll->getIntervalsContainedIn(syll)) {
                        QString temporal = intv->attribute("temporal").toString();
                        if (intv->text().contains(speakerID)) {
                            if (syllCategory == "FIL") { // Filled pause
                                timeFilledPause = timeFilledPause + intv->duration();
                                durationsPauseFIL << intv->duration().toDouble();
                            }
                            else {
                                if (temporal == "P") {
                                    if (syllCategory != "ART") syllCategory = "SIL";
                                    timeSilentPause = timeSilentPause + intv->duration();
                                    durationsPauseSIL << intv->duration().toDouble();
                                    // duration of pause relative to (-1, 1)...(-5, 5) segments - pause does not block window
                                    durationsPauseSIL_rel1 << Measures::relative(tier_syll, "duration", syllIndex, 1, 1, false);
                                    durationsPauseSIL_rel2 << Measures::relative(tier_syll, "duration", syllIndex, 2, 2, false);
                                    durationsPauseSIL_rel3 << Measures::relative(tier_syll, "duration", syllIndex, 3, 3, false);
                                    durationsPauseSIL_rel4 << Measures::relative(tier_syll, "duration", syllIndex, 4, 4, false);
                                    durationsPauseSIL_rel5 << Measures::relative(tier_syll, "duration", syllIndex, 5, 5, false);
                                }
                                else {
                                    syllCategory = "ART";
                                    if      (temporal == "S")    timeArticulationAlone = timeArticulationAlone + intv->duration();
                                    else if (temporal == "OVC")  timeArticulationOverlapContinue = timeArticulationOverlapContinue + intv->duration();
                                    else if (temporal == "OVT")  timeArticulationOverlapTurnChange = timeArticulationOverlapTurnChange + intv->duration();
                                }
                            }
                        }
                    }
                    if      (syllCategory == "SIL") { numSilentPauses++; }
                    else if (syllCategory == "FIL") { numFilledPauses++; }
                    else if (syllCategory == "ART") { numSyllablesArticulated++; currentTurnArtSyllCount++; }
                    syllIndex++;
                } // end foreach syll
                turnArtSyllCounts << ((double)currentTurnArtSyllCount);
            } // end foreach turn
            // Processing of this speaker finished
            if (timeSpeech == RealTime(0,0)) continue;
            // Add statistical measures for current speaker
            RealTime timeArticulation = timeArticulationAlone + timeArticulationOverlapContinue + timeArticulationOverlapTurnChange;
            RealTime timeOverlap = timeArticulationOverlapContinue + timeArticulationOverlapTurnChange;
            QHash<QString, double> measures;
            measures.insert("TimeSpeech", timeSpeech.toDouble());
            measures.insert("TimeArticulation", timeArticulation.toDouble());
            measures.insert("TimeArticulation_Alone", timeArticulationAlone.toDouble());
            measures.insert("TimeArticulation_Overlap", timeOverlap.toDouble());
            measures.insert("TimeArticulation_Overlap_Continue", timeArticulationOverlapContinue.toDouble());
            measures.insert("TimeArticulation_Overlap_TurnChange", timeArticulationOverlapTurnChange.toDouble());
            measures.insert("TimeSilentPause", timeSilentPause.toDouble());
            measures.insert("TimeFilledPause", timeFilledPause.toDouble());
            measures.insert("RatioArticulation", timeArticulation.toDouble() / timeSpeech.toDouble());
            measures.insert("RatioArticulation_Alone", timeArticulationAlone.toDouble() / timeSpeech.toDouble());
            measures.insert("RatioArticulation_Overlap", timeOverlap.toDouble() / timeSpeech.toDouble());
            measures.insert("RatioArticulation_Overlap_Continue", timeArticulationOverlapContinue.toDouble() / timeSpeech.toDouble());
            measures.insert("RatioArticulation_Overlap_TurnChange", timeArticulationOverlapTurnChange.toDouble() / timeSpeech.toDouble());
            measures.insert("RatioSilentPause", timeSilentPause.toDouble() / timeSpeech.toDouble());
            measures.insert("RatioFilledPause", timeFilledPause.toDouble() / timeSpeech.toDouble());
            measures.insert("NumTokens", numTokens);
            measures.insert("NumSyllables", numSyllablesArticulated);
            measures.insert("NumSilentPauses", numSilentPauses);
            measures.insert("NumFilledPauses", numFilledPauses);
            measures.insert("SpeechRate", numSyllablesArticulated / timeSpeech.toDouble());
            measures.insert("SilentPauseRate", numSilentPauses / timeSpeech.toDouble());
            measures.insert("FilledPauseRate", numFilledPauses / timeSpeech.toDouble());
            measures.insert("ArticulationRate", numSyllablesArticulated / timeArticulation.toDouble());
            StatisticalSummary summaryDurationsPauseSIL, summaryDurationsPauseFIL;
            StatisticalSummary summaryTurnDurations, summaryTurnTokenCounts, summaryTurnArtSyllCounts;
            summaryDurationsPauseSIL.calculate(durationsPauseSIL);
            summaryDurationsPauseFIL.calculate(durationsPauseFIL);
            summaryTurnDurations.calculate(turnDurations);
            summaryTurnTokenCounts.calculate(turnTokenCounts);
            summaryTurnArtSyllCounts.calculate(turnArtSyllCounts);
            measures.insert("PauseDur_SIL_Median", summaryDurationsPauseSIL.median());
            measures.insert("PauseDur_SIL_Q1", summaryDurationsPauseSIL.firstQuartile());
            measures.insert("PauseDur_SIL_Q3", summaryDurationsPauseSIL.thirdQuartile());
            measures.insert("PauseDur_FIL_Median", summaryDurationsPauseFIL.median());
            measures.insert("PauseDur_FIL_Q1", summaryDurationsPauseFIL.firstQuartile());
            measures.insert("PauseDur_FIL_Q3", summaryDurationsPauseFIL.thirdQuartile());
            measures.insert("TurnDuration_Time_Mean", summaryTurnDurations.mean());
            measures.insert("TurnDuration_Token_Mean", summaryTurnTokenCounts.mean());
            measures.insert("TurnDuration_Syll_Mean", summaryTurnArtSyllCounts.mean());
            d->measuresSpk.insert(speakerID, measures);
            // Vector measures for current speaker
            QHash<QString, QList<double> > vectors;
            vectors.insert("Pause_SIL_Durations", durationsPauseSIL);
            vectors.insert("Pause_FIL_Durations", durationsPauseFIL);
            vectors.insert("TurnDurations", turnDurations);
            d->vectorsSpk.insert(speakerID, vectors);
        }
        qDeleteAll(tiersAll);
    }
}

