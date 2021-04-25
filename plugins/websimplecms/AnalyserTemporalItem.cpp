#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QMutex>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Statistics/Measures.h"
#include "PraalineCore/Statistics/StatisticalMeasureDefinition.h"
#include "PraalineCore/Statistics/StatisticalSummary.h"
#include "PraalineCore/Statistics/SpeakerTimeline.h"
using namespace Praaline::Core;

#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

#include "AnalyserTemporalItem.h"

struct AnalyserTemporalItemData {
    AnalyserTemporalItemData() : levelSyllables("syll"), levelTokens("tok_min")
    {
        filledPauseTokens << "euh" << "euhm" << "hum";
    }

    QString levelSyllables;
    QString levelTokens;
    QStringList filledPauseTokens;
    // Data from the analysis of one Communication
    QHash<QString, double> measuresCom;
    QMap<QString, QHash<QString, double> > measuresSpk;
    QHash<QString, QList<double> > vectorsCom;
    QMap<QString, QHash<QString, QList<double> > > vectorsSpk;
    QPointer<SpeakerTimeline> timeline;
};

AnalyserTemporalItem::AnalyserTemporalItem(QObject *parent) :
    QObject(parent), d(new AnalyserTemporalItemData)
{
}

AnalyserTemporalItem::~AnalyserTemporalItem()
{
    delete d;
}

// static
QStringList AnalyserTemporalItem::measureIDsForCommunication()
{
    return QStringList()
            << "TimeTotalSample" << "TimeSingleSpeaker" << "TimeOverlap" << "TimeGap"
            << "RatioSingleSpeaker" << "RatioOverlap" << "RatioGap"
            << "GapDurations_Median" << "GapDurations_Q1" << "GapDurations_Q3"
            << "TurnChangesCount" << "TurnChangesCount_Gap" << "TurnChangesCount_Overlap"
            << "TurnChangesRate" << "TurnChangesRate_Gap" << "TurnChangesRate_Overlap";
}

// static
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
            << "TurnDuration_Time_Mean" << "TurnDuration_Syll_Mean" << "TurnDuration_Token_Mean"
            << "IntersyllabicInterval_Mean" << "IntersyllabicInterval_StDev";
}

// static
QStringList AnalyserTemporalItem::vectorMeasureIDsForCommunication()
{
    return QStringList() << "OverlapDurations" << "GapDurations";
}

// static
QStringList AnalyserTemporalItem::vectorMeasureIDsForSpeaker()
{
    return QStringList() << "Pause_SIL_Durations" << "Pause_FIL_Durations" << "TurnDurations"
                         << "IntersyllabicIntervals";
}

// static
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
    if (measureID == "IntersyllabicInterval_Mean") return StatisticalMeasureDefinition("IntersyllabicInterval_Mean", "Intersyllabic Interval (mean)", "sec");
    if (measureID == "IntersyllabicInterval_StDev") return StatisticalMeasureDefinition("IntersyllabicInterval_StDev", "Intersyllabic Interval (std dev)", "sec");
    // Note: ArtRatio + SilRatio + FilRatio = 100%. Percentages calculated over the Speech time of current speaker)
    // Vectors per Speaker
    if (measureID == "Pause_SIL_Durations") return StatisticalMeasureDefinition("Pause_SIL_Durations", "Silent pause durations", "s", "Silent pause durations", QVariant::Double, true);
    if (measureID == "Pause_FIL_Durations") return StatisticalMeasureDefinition("Pause_SIL_Durations", "Filled pause durations", "s", "Filled pause durations", QVariant::Double, true);
    if (measureID == "TurnDurations")       return StatisticalMeasureDefinition("TurnDurations", "Turn durations", "s", "Durations of turns", QVariant::Double, true);
    if (measureID == "IntersyllabicIntervals")       return StatisticalMeasureDefinition("IntersyllabicIntervals", "Intersyllabic intervals", "s", "Durations between syllable centres", QVariant::Double, true);
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

QString AnalyserTemporalItem::levelIDSyllables() const
{
    return d->levelSyllables;
}

void AnalyserTemporalItem::setLevelIDSyllables(const QString &levelID)
{
    d->levelSyllables = levelID;
}

QString AnalyserTemporalItem::levelIDTokens() const
{
    return d->levelTokens;
}

void AnalyserTemporalItem::setLevelIDTokens(const QString &levelID)
{
    d->levelTokens = levelID;
}

QStringList AnalyserTemporalItem::filledPauseTokens() const
{
    return d->filledPauseTokens;
}

void AnalyserTemporalItem::setFilledPauseTokens(const QStringList &filledPauseTokens)
{
    d->filledPauseTokens = filledPauseTokens;
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

QPointer<IntervalTier> AnalyserTemporalItem::timelineSyll() const
{
    if (d->timeline) return d->timeline->timelineDetailed();
    return nullptr;
}

QPointer<IntervalTier> AnalyserTemporalItem::timelineSpeaker() const
{
    if (d->timeline) return d->timeline->timelineCoarse();
    return nullptr;
}


void AnalyserTemporalItem::analyse(CorpusCommunication *com)
{
    static QMutex mutex;
    if (!com) return;
    if (!com->repository()) return;

    qDebug() << "Analysing temporal variables for " << com->ID();

    d->measuresCom.clear();
    d->measuresSpk.clear();
    d->vectorsCom.clear();
    d->vectorsSpk.clear();

    // Lock mutex because SpeakerTimeline::calculate will be accessing the database
    mutex.lock();
    if (d->timeline) delete d->timeline;
    d->timeline = new SpeakerTimeline(this);
    d->timeline->calculate(com, d->levelSyllables);
    mutex.unlock();
    QPointer<IntervalTier> tier_timelineSyll(d->timeline->timelineDetailed());
    QPointer<IntervalTier> tier_timelineSpk(d->timeline->timelineCoarse());
    if ((!tier_timelineSyll) || (!tier_timelineSpk)) return;

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
    d->measuresCom.insert("TurnChangesRate", static_cast<double>(turnChangesWithGap + turnChangesWithOverlap) * 60.0 / timeTotalSample.toDouble());
    d->measuresCom.insert("TurnChangesRate_Gap", static_cast<double>(turnChangesWithGap) * 60.0 / timeTotalSample.toDouble());
    d->measuresCom.insert("TurnChangesRate_Overlap", static_cast<double>(turnChangesWithOverlap) * 60.0 / timeTotalSample.toDouble());
    d->vectorsCom.insert("OverlapDurations", overlapDurations);
    d->vectorsCom.insert("GapDurations", gapDurations);

    // Inter-syllabic interval
    QStringList excludedSyllTextForISI;
    excludedSyllTextForISI << "_" << "" << "N" << "D";

    foreach (QString annotationID, com->annotationIDs()) {
        mutex.lock();
        SpeakerAnnotationTierGroupMap tiersAll =
                com->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList() << d->levelSyllables << d->levelTokens);
        mutex.unlock();
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
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
            QList<double> intersyllabicIntervals;
            // Turn level
            QScopedPointer<IntervalTier> tier_turns(tier_timelineSpk->clone());
            foreach (Interval *intv, tier_turns->intervals()) {
                if (intv->text().contains(speakerID)) intv->setText(speakerID); else intv->setText("");
            }
            tier_turns->mergeIdenticalAnnotations();
            foreach (Interval *turn, tier_turns->intervals()) {
                if (!turn->text().contains(speakerID)) continue;
                // qDebug() << com->ID() << speakerID;
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
                turnTokenCounts << static_cast<double>(currentTurnTokenCount);
                // The basic units of time measurement are the speaker's syllables
                int syllIndex(0);
                foreach (Interval *syll, tier_syll->getIntervalsContainedIn(turn)) {
                    QString syllCategory = ""; // SIL, FIL or ART?
                    QList<Interval *> tokens = tier_tokmin->getIntervalsOverlappingWith(syll);
                    // foreach (Interval *token, tokens) qDebug() << token->text();
                    if (tokens.count() == 1 && d->filledPauseTokens.contains(tokens.first()->text())) {
                        syllCategory = "FIL"; // This syllable is a filled pause
                    }
                    // Inter-Syllabic Interval (ISI)
                    QString syllText = tier_syll->at(syllIndex)->text();
                    QString syllTextPrev = (syllIndex > 0) ? tier_syll->at(syllIndex - 1)->text() : "";
                    if ((syllIndex > 0) && (!tier_syll->at(syllIndex - 1)->isPauseSilent()) &&
                        (!excludedSyllTextForISI.contains(syllText)) && (!excludedSyllTextForISI.contains(syllTextPrev))) {
                        double dur = (syll->tCenter() - tier_syll->at(syllIndex - 1)->tCenter()).toDouble();
                        intersyllabicIntervals << dur;
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
                turnArtSyllCounts << static_cast<double>(currentTurnArtSyllCount);
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
            StatisticalSummary summaryIntersyllabicIntervals;
            summaryDurationsPauseSIL.calculate(durationsPauseSIL);
            summaryDurationsPauseFIL.calculate(durationsPauseFIL);
            summaryTurnDurations.calculate(turnDurations);
            summaryTurnTokenCounts.calculate(turnTokenCounts);
            summaryTurnArtSyllCounts.calculate(turnArtSyllCounts);
            summaryIntersyllabicIntervals.calculate(intersyllabicIntervals);
            measures.insert("PauseDur_SIL_Median", summaryDurationsPauseSIL.median());
            measures.insert("PauseDur_SIL_Q1", summaryDurationsPauseSIL.firstQuartile());
            measures.insert("PauseDur_SIL_Q3", summaryDurationsPauseSIL.thirdQuartile());
            measures.insert("PauseDur_FIL_Median", summaryDurationsPauseFIL.median());
            measures.insert("PauseDur_FIL_Q1", summaryDurationsPauseFIL.firstQuartile());
            measures.insert("PauseDur_FIL_Q3", summaryDurationsPauseFIL.thirdQuartile());
            measures.insert("TurnDuration_Time_Mean", summaryTurnDurations.mean());
            measures.insert("TurnDuration_Token_Mean", summaryTurnTokenCounts.mean());
            measures.insert("TurnDuration_Syll_Mean", summaryTurnArtSyllCounts.mean());
            measures.insert("IntersyllabicInterval_Mean", summaryIntersyllabicIntervals.mean());
            measures.insert("IntersyllabicInterval_StDev", summaryIntersyllabicIntervals.stDev());
            d->measuresSpk.insert(speakerID, measures);
            // Vector measures for current speaker
            QHash<QString, QList<double> > vectors;
            vectors.insert("Pause_SIL_Durations", durationsPauseSIL);
            vectors.insert("Pause_FIL_Durations", durationsPauseFIL);
            vectors.insert("TurnDurations", turnDurations);
            vectors.insert("IntersyllabicIntervals", intersyllabicIntervals);

            d->vectorsSpk.insert(speakerID, vectors);
        }
        qDeleteAll(tiersAll);
    }
}

