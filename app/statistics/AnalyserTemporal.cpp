#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"

#include "pncore/interfaces/praat/praattextgrid.h"

#include "AnalyserTemporal.h"

struct AnalyserTemporalData {
    AnalyserTemporalData() : levelSyllables("syll"), levelTokens("tok_min")
    {
        filledPauseTokens << "euh" << "euhm";
    }

    QString levelSyllables;
    QString levelTokens;
    QStringList filledPauseTokens;

    QHash<QString, double> measuresCom;
    QMap<QString, QHash<QString, double> > measuresSpk; // speakers are sorted
};

AnalyserTemporal::AnalyserTemporal(QObject *parent) :
    QObject(parent), d(new AnalyserTemporalData)
{
}

AnalyserTemporal::~AnalyserTemporal()
{
    delete d;
}

QList<QString> AnalyserTemporal::measureIDsForCommunication()
{
    return QList<QString>()
            << "TimeTotalSample" << "TimeSingleSpeaker" << "TimeOverlap" << "TimeGap"
            << "RatioSingleSpeaker" << "RatioOverlap" << "RatioGap"
            << "GapDurations_Median" << "GapDurations_Q1" << "GapDurations_Q3"
            << "TurnChangesCount" << "TurnChangesCount_Gap" << "TurnChangesCount_Overlap"
            << "TurnChangesRate" << "TurnChangesRate_Gap" << "TurnChangesRate_Overlap";
}

QList<QString> AnalyserTemporal::measureIDsForSpeaker()
{
    return QList<QString>()
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

StatisticalMeasureDefinition AnalyserTemporal::measureDefinitionForCommunication(const QString &measureID)
{
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
    if (measureID == "TurnChangesCount_Gap")     return StatisticalMeasureDefinition("TurnChangesCount_Gap", "Number of turn changes, withour overlap", "");
    if (measureID == "TurnChangesCount_Overlap") return StatisticalMeasureDefinition("TurnChangesCount_Overlap", "Number of turn changes, with overlap", "");
    if (measureID == "TurnChangesRate")          return StatisticalMeasureDefinition("TurnChangesRate", "Turn changes per minute", "1/min");
    if (measureID == "TurnChangesRate_Gap")      return StatisticalMeasureDefinition("TurnChangesRate_Gap", "Turn changes per minute, without overlap", "1/min");
    if (measureID == "TurnChangesRate_Overlap")  return StatisticalMeasureDefinition("TurnChangesRate_Overlap", "Turn changes per minute, with overlap", "1/min");
    return StatisticalMeasureDefinition(measureID, measureID, "");
    // Note: total sample time is less than the recording (media file) time, by excluding leading and trailing silence
}

StatisticalMeasureDefinition AnalyserTemporal::measureDefinitionForSpeaker(const QString &measureID)
{
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
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

double AnalyserTemporal::measureCom(const QString &measureID) const
{
    return d->measuresCom.value(measureID, qQNaN());
}

double AnalyserTemporal::measureSpk(const QString &speakerID, const QString &measureID) const
{
    if (!d->measuresSpk.contains(speakerID)) return qQNaN();
    return d->measuresSpk.value(speakerID).value(measureID, qQNaN());
}

QList<QString> AnalyserTemporal::speakerIDs() const
{
    return d->measuresSpk.keys();
}

QPair<IntervalTier *, IntervalTier *> getSpeakerTimeline(AnalyserTemporalData *d, QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPair<IntervalTier *, IntervalTier *> ret(0, 0);
    if (!corpus || !com) return ret;

    // A timeline on the syllable level with detailed = true means there will be an interval for each syllable
    // and possibly segmented syllables when two speakers overlap.
    IntervalTier *tier_timelineSyll = corpus->datastoreAnnotations()->getSpeakerTimeline(com->ID(), "", d->levelSyllables, true);
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

void debugCreateTimelineTextgrid(AnalyserTemporalData *d, QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    QPair<IntervalTier *, IntervalTier *> timelines = getSpeakerTimeline(d, corpus, com);
    if (!timelines.first || !timelines.second) return;
    IntervalTier *tier_timelineSyll(timelines.first);
    IntervalTier *tier_timelineSpk(timelines.second);

    QString path = corpus->basePath();
    QScopedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
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

void AnalyserTemporal::calculate(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                                 QTextStream &pauseListSIL, QTextStream &pauseListFIL)
{
    if (!corpus || !com) return;

    d->measuresCom.clear();
    d->measuresSpk.clear();

    // debugCreateTimelineTextgrid(d, corpus, com);

    QPair<IntervalTier *, IntervalTier *> timelines = getSpeakerTimeline(d, corpus, com);
    if (!timelines.first || !timelines.second) return;
    QScopedPointer<IntervalTier> tier_timelineSyll(timelines.first);
    QScopedPointer<IntervalTier> tier_timelineSpk(timelines.second);

    // Measures that can be calculated from the timeline, on the Communication level
    RealTime timeSingleSpeaker, timeOverlap, timeGap;
    QList<double> gapDurations;
    int turnChangesWithGap(0), turnChangesWithOverlap(0);
    foreach (Interval *intv, tier_timelineSpk->intervals()) {
        QString temporal = intv->attribute("temporal").toString();
        if (temporal == "X") continue;
        if (temporal == "S" || temporal == "P") {
            timeSingleSpeaker = timeSingleSpeaker + intv->duration();
        }
        else if (temporal == "G") {
            gapDurations << intv->duration().toDouble();
            turnChangesWithGap++;
            timeGap = timeGap + intv->duration();
        }
        else {
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

    foreach (QString annotationID, com->annotationIDs()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID, QStringList() <<
                                                                                                                     d->levelSyllables << d->levelTokens);
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

            // Pause lists
            foreach (double dur, durationsPauseSIL) {
                pauseListSIL << com->ID() << "\t" << speakerID << "\t";
                foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
                    pauseListSIL << com->property(attr->ID()).toString() << "\t";
                }
                pauseListSIL << QString::number(dur) << "\n";
            }
            foreach (double dur, durationsPauseFIL) {
                pauseListFIL << com->ID() << "\t" << speakerID << "\t";
                foreach (QPointer<MetadataStructureAttribute> attr, corpus->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
                    pauseListFIL << com->property(attr->ID()).toString() << "\t";
                }
                pauseListFIL << QString::number(dur) << "\n";
            }
        }
        qDeleteAll(tiersAll);
    }
}

