#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"

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
    QHash<QString, QHash<QString, double> > measuresSpk;
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
            << "TurnDuration_Time_Median" << "TurnDuration_Syll_Median" << "TurnDuration_Token_Median";
}

StatisticalMeasureDefinition AnalyserTemporal::measureDefinitionForCommunication(const QString &measureID)
{
    if (measureID == "TimeTotalSample")     return StatisticalMeasureDefinition("TimeTotalSample", "Total sample time", "s");
    if (measureID == "TimeSingleSpeaker")   return StatisticalMeasureDefinition("TimeSingleSpeaker", "Single-speaker time", "s");
    if (measureID == "TimeOverlap")         return StatisticalMeasureDefinition("TimeOverlap", "Overlap time", "s");
    if (measureID == "TimeGap")             return StatisticalMeasureDefinition("TimeGap", "Gap time", "s");
    if (measureID == "RatioSingleSpeaker")  return StatisticalMeasureDefinition("RatioSingleSpeaker", "Single-Speaker time ratio", "%", "Signle-speaker time / Total sample time");
    if (measureID == "RatioOverlap")        return StatisticalMeasureDefinition("RatioOverlap", "Overlap time ratio", "%", "Overlap time / Total sample time");
    if (measureID == "RatioGap")            return StatisticalMeasureDefinition("RatioGap", "Gap time ratio", "%", "Gap time / Total sample time");
    if (measureID == "GapDurations_Median") return StatisticalMeasureDefinition("GapDurations_Median", "Gap duration (median)", "s");
    if (measureID == "GapDurations_Q1")     return StatisticalMeasureDefinition("GapDurations_Q1", "Gap duration (Q1)", "s");
    if (measureID == "GapDurations_Q3")     return StatisticalMeasureDefinition("GapDurations_Q3", "Gap duration (Q3)", "s");
    if (measureID == "TurnChangesRate")     return StatisticalMeasureDefinition("TurnChangesRate", "Turn changes per second", "1/s");
    if (measureID == "TurnChangesRate_Gap") return StatisticalMeasureDefinition("TurnChangesRate_Gap", "Turn changes per second, without overlap", "1/s");
    if (measureID == "TurnChangesRate_Overlap") return StatisticalMeasureDefinition("TurnChangesRate_Overlap", "Turn changes per second, with overlap", "1/s");
    return StatisticalMeasureDefinition(measureID, measureID, "");
    // Note: total sample time is less than the recording (media file) time, by excluding leading and trailing silence
}

StatisticalMeasureDefinition AnalyserTemporal::measureDefinitionForSpeaker(const QString &measureID)
{
    if (measureID == "TimeSpeech") return StatisticalMeasureDefinition("TimeSpeech", "Speech time", "s", "Articulation Time + Silent Pause Time + Filled Pause Time");
    if (measureID == "TimeArticulation") return StatisticalMeasureDefinition("TimeArticulation", "Articulation time", "s");
    if (measureID == "TimeArticulation_Alone") return StatisticalMeasureDefinition("TimeArticulation_Alone", "Articulation, speaking alone time", "s");
    if (measureID == "TimeArticulation_Overlap") return StatisticalMeasureDefinition("TimeArticulation_Overlap", "Articulation, overlap time", "s");
    if (measureID == "TimeArticulation_Overlap_Continue") return StatisticalMeasureDefinition("TimeArticulation_Overlap_Continue", "Articulation, overlap without turn change, time", "s");
    if (measureID == "TimeArticulation_Overlap_TurnChange") return StatisticalMeasureDefinition("TimeArticulation_Overlap_TurnChange", "Articulation, overlap with turn change, time", "s");
    if (measureID == "TimeSilentPause") return StatisticalMeasureDefinition("TimeSilentPause", "Silent pause time", "s");
    if (measureID == "TimeFilledPause") return StatisticalMeasureDefinition("TimeFilledPause", "Filled pause time", "s");
    if (measureID == "RatioArticulation") return StatisticalMeasureDefinition("RatioArticulation", "Articulation ratio", "%");
    if (measureID == "RatioArticulation_Alone") return StatisticalMeasureDefinition("RatioArticulation_Alone", "Articulation, speaking alone ratio", "%");
    if (measureID == "RatioArticulation_Overlap") return StatisticalMeasureDefinition("RatioArticulation_Overlap", "Articulation, overlap ratio, ", "%");
    if (measureID == "RatioArticulation_Overlap_Continue") return StatisticalMeasureDefinition("RatioArticulation_Overlap_Continue", "Articulation, overlap without turn change, ratio", "%");
    if (measureID == "RatioArticulation_Overlap_TurnChange") return StatisticalMeasureDefinition("RatioArticulation_Overlap_TurnChange", "Articulation, overlap with turn change, ratio", "%");
    if (measureID == "RatioSilentPause") return StatisticalMeasureDefinition("RatioSilentPause", "Silent pause ratio", "%");
    if (measureID == "RatioFilledPause") return StatisticalMeasureDefinition("RatioFilledPause", "Filled pause ratio", "%");
    if (measureID == "NumTokens") return StatisticalMeasureDefinition("NumTokens", "Number of tokens", "");
    if (measureID == "NumSyllables") return StatisticalMeasureDefinition("NumSyllables", "Number of syllables (articulated)", "");
    if (measureID == "NumSilentPauses") return StatisticalMeasureDefinition("NumSilentPauses", "Number of silent pauses", "");
    if (measureID == "NumFilledPauses") return StatisticalMeasureDefinition("NumFilledPauses", "Number of filled pauses", "");
    if (measureID == "SpeechRate") return StatisticalMeasureDefinition("SpeechRate", "Speech rate", "syll/s", "All articulated syllables (excluding SIL and FIL) / Speech time");
    if (measureID == "SilentPauseRate") return StatisticalMeasureDefinition("SilentPauseRate", "Silent pause rate", "SIL/s", "Number of silent pauses / Speech time");
    if (measureID == "FilledPauseRate") return StatisticalMeasureDefinition("FilledPauseRate", "Filled pause rate", "FIL/s", "Number of filled pauses / Speech time");
    if (measureID == "ArticulationRate") return StatisticalMeasureDefinition("ArticulationRate", "Articulation rate", "(syll/s)", "All articulated syllables (excluding SIL and FIL) / Articulation time");
    if (measureID == "PauseDur_SIL_Median") return StatisticalMeasureDefinition("PauseDur_SIL_Median", "Silent pause duration (median)", "s");
    if (measureID == "PauseDur_SIL_Q1") return StatisticalMeasureDefinition("PauseDur_SIL_Q1", "Silent pause duration (Q1)", "s");
    if (measureID == "PauseDur_SIL_Q3") return StatisticalMeasureDefinition("PauseDur_SIL_Q3", "Silent pause duration (Q3)", "s");
    if (measureID == "PauseDur_FIL_Median") return StatisticalMeasureDefinition("PauseDur_FIL_Median", "Filled pause duration (median)", "s");
    if (measureID == "PauseDur_FIL_Q1") return StatisticalMeasureDefinition("PauseDur_FIL_Q1", "Filled pause duration (Q1)", "s");
    if (measureID == "PauseDur_FIL_Q3") return StatisticalMeasureDefinition("PauseDur_FIL_Q3", "Filled pause duration (Q3)", "s");
    if (measureID == "TurnDuration_Time_Median") return StatisticalMeasureDefinition("TurnDuration_Time_Median", "Turn duration (median)", "s");
    if (measureID == "TurnDuration_Syll_Median") return StatisticalMeasureDefinition("TurnDuration_Syll_Median", "Number of syllables in turn (median)", "syll");
    if (measureID == "TurnDuration_Token_Median") return StatisticalMeasureDefinition("TurnDuration_Token_Median", "Number of tokens in turn (median)", "tokens");
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

IntervalTier *AnalyserTemporal::getSpeakerTimeline(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    if (!corpus || !com) return 0;

    IntervalTier *tier_timeline = corpus->datastoreAnnotations()->getSpeakerTimeline(com->ID(), "", d->levelSyllables);
    if (!tier_timeline) return 0;

    // Exclude silence at the begining and end of recording
    int i = 0;
    while (i < tier_timeline->count() && tier_timeline->interval(i)->isPauseSilent()) {
        tier_timeline->interval(i)->setAttribute("temporal", "X");
        i++;
    }
    i = tier_timeline->count() - 1;
    while (i >= 0 && tier_timeline->interval(i)->isPauseSilent()) {
        tier_timeline->interval(i)->setAttribute("temporal", "X");
        i--;
    }
    // Categorise intervals using an additional attribute on the timeline tier
    for (i = 0; i < tier_timeline->count(); ++i) {
        Interval *intv = tier_timeline->interval(i);
        if (intv->attribute("temporal").toString() == "X")
            continue; // excluded
        if (intv->isPauseSilent()) {
            QString speakerBefore, speakerAfter;
            if (i > 0) speakerBefore = tier_timeline->interval(i - 1)->text();
            if (i < tier_timeline->count() - 2) speakerAfter = tier_timeline->interval(i + 1)->text();
            if (speakerAfter.contains(speakerBefore) || (speakerBefore.isEmpty()) || (speakerAfter.isEmpty())) {
                intv->setText(speakerBefore); // silent pause belonging to the current speaker
                intv->setAttribute("temporal", "P");
            } else {
                intv->setAttribute("temporal", "G");
            }
        }
        else {
            // One or many speakers speaking?
            if (intv->text().contains("+")) {
                // Detect overlaps with or without change of turn
                QString speakerBefore, speakerAfter;
                if (i > 0) speakerBefore = tier_timeline->interval(i - 1)->text();
                if (i < tier_timeline->count() - 2) speakerAfter = tier_timeline->interval(i + 1)->text();
                if ((speakerBefore == speakerAfter) && (!speakerBefore.contains("+")) && (!speakerAfter.contains("+")))
                    intv->setAttribute("temporal", "OVC");
                else
                    intv->setAttribute("temporal", "OVT");
            } else {
                intv->setAttribute("temporal", "S");
            }
        }
    }
    return tier_timeline;
}

void AnalyserTemporal::calculate(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    if (!corpus || !com) return;
    IntervalTier *tier_timeline = getSpeakerTimeline(corpus, com);

    // Measures that can be calculated from the timeline, on the Communication level
    double timeSingleSpeaker(0), timeOverlap(0), timeGap(0);
    QList<double> gapDurations;
    int turnChangesWithGap(0), turnChangesWithOverlap(0);
    foreach (Interval *intv, tier_timeline->intervals()) {
        QString temporal = intv->attribute("temporal").toString();
        if (temporal == "X") continue;
        if (temporal == "S" || temporal == "P")
            timeSingleSpeaker += intv->duration().toDouble();
        else if (temporal == "G") {
            gapDurations << intv->duration().toDouble();
            turnChangesWithGap++;
            timeGap += intv->duration().toDouble();
        }
        else {
            timeOverlap += intv->duration().toDouble();
            if (temporal == "OVT") turnChangesWithOverlap++;
        }
    }
    double timeTotalSample = timeSingleSpeaker + timeOverlap + timeGap;
    if (timeTotalSample == 0) return;
    d->measuresCom.insert("TimeTotalSample", timeTotalSample);
    d->measuresCom.insert("TimeSingleSpeaker", timeSingleSpeaker);
    d->measuresCom.insert("TimeOverlap", timeOverlap );
    d->measuresCom.insert("TimeGap", timeGap);
    d->measuresCom.insert("RatioSingleSpeaker", timeSingleSpeaker / timeTotalSample);
    d->measuresCom.insert("RatioOverlap", timeOverlap / timeTotalSample);
    d->measuresCom.insert("RatioGap", timeGap / timeTotalSample);
    StatisticalSummary summaryGapDurations;
    summaryGapDurations.calculate(gapDurations);
    d->measuresCom.insert("GapDurations_Median", summaryGapDurations.median());
    d->measuresCom.insert("GapDurations_Q1", summaryGapDurations.firstQuartile());
    d->measuresCom.insert("GapDurations_Q3", summaryGapDurations.thirdQuartile());
    d->measuresCom.insert("TurnChangesRate", ((double)(turnChangesWithGap + turnChangesWithOverlap)) / timeTotalSample);
    d->measuresCom.insert("TurnChangesRate_Gap", ((double)turnChangesWithGap) / timeTotalSample);
    d->measuresCom.insert("TurnChangesRate_Overlap", ((double)turnChangesWithOverlap) / timeTotalSample);


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
            double timeSpeech(0), timeSilentPause(0), timeFilledPause(0);
            double timeArticulationAlone(0), timeArticulationOverlapContinue(0), timeArticulationOverlapTurnChange(0);
            int numSilentPauses(0), numFilledPauses(0), numSyllablesArticulated(0), numTokens(0);
            QList<double> durationsPauseSIL, durationsPauseFIL;
            QList<double> turnDurations, turnTokenCounts, turnSyllableCounts;
            double currentTurnDuration(0), currentTurnTokenCount(0), currentTurnSyllableCount(0);
            // Process timeline, focusing only on intervals for current speaker
            foreach (Interval *intv, tier_timeline->intervals()) {
                QString temporal = intv->attribute("temporal").toString();
                if (intv->text().contains(speakerID)) {
                    timeSpeech += intv->duration().toDouble();
                    currentTurnDuration += intv->duration().toDouble();
                    if (temporal == "P") {
                        // Silent pause belonging to current speaker
                        timeSilentPause += intv->duration().toDouble();
                        durationsPauseSIL << intv->duration().toDouble();
                        numSilentPauses++;
                    }
                    else {
                        // Speech. Separate filled pauses from articulated syllables.
                        QList<Interval *> tokens = tier_tokmin->getIntervalsContainedIn(intv);
                        foreach (Interval *token, tokens) {
                            if (d->filledPauseTokens.contains(token->text())) {
                                // Filled pause
                                timeFilledPause += token->duration().toDouble();
                                durationsPauseFIL << token->duration().toDouble();
                                numFilledPauses++;
                            }
                            else {
                                // Articulation
                                if (temporal == "S")    timeArticulationAlone += token->duration().toDouble();
                                if (temporal == "OVC")  timeArticulationOverlapContinue += token->duration().toDouble();
                                if (temporal == "OVT")  timeArticulationOverlapTurnChange += token->duration().toDouble();
                                QPair<int, int> syllIndices = tier_syll->getIntervalIndexesContainedIn(token);
                                int nsyll = (syllIndices.second - syllIndices.first + 1);
                                numSyllablesArticulated += nsyll;
                                numTokens++;
                                currentTurnSyllableCount += (double)nsyll;
                                currentTurnTokenCount += 1.0;
                            }
                        }
                        // End of Speech.
                    }
                }
                else {
                    // Turn just changed.
                    if (currentTurnDuration > 0) {
                        turnDurations << currentTurnDuration;
                        turnTokenCounts << currentTurnTokenCount;
                        turnSyllableCounts << currentTurnSyllableCount;
                    }
                    currentTurnDuration = 0;
                    currentTurnTokenCount = 0;
                    currentTurnSyllableCount = 0;
                }
            }
            // Any left-overs?
            if (currentTurnDuration > 0) {
                turnDurations << currentTurnDuration;
                turnTokenCounts << currentTurnTokenCount;
                turnSyllableCounts << currentTurnSyllableCount;
            }
            if (timeSpeech == 0) continue;
            // Add statistical measures for current speaker
            double timeArticulation = timeArticulationAlone + timeArticulationOverlapContinue + timeArticulationOverlapTurnChange;
            double timeOverlap = timeArticulationOverlapContinue + timeArticulationOverlapTurnChange;
            QHash<QString, double> measures;
            measures.insert("TimeSpeech", timeSpeech);
            measures.insert("TimeArticulation", timeArticulation);
            measures.insert("TimeArticulation_Alone", timeArticulationAlone);
            measures.insert("TimeArticulation_Overlap", timeOverlap);
            measures.insert("TimeArticulation_Overlap_Continue", timeArticulationOverlapContinue);
            measures.insert("TimeArticulation_Overlap_TurnChange", timeArticulationOverlapTurnChange);
            measures.insert("TimeSilentPause", timeSilentPause);
            measures.insert("TimeFilledPause", timeFilledPause);
            measures.insert("RatioArticulation", timeArticulation / timeSpeech);
            measures.insert("RatioArticulation_Alone", timeArticulationAlone / timeSpeech);
            measures.insert("RatioArticulation_Overlap", timeOverlap / timeSpeech);
            measures.insert("RatioArticulation_Overlap_Continue", timeArticulationOverlapContinue / timeSpeech);
            measures.insert("RatioArticulation_Overlap_TurnChange", timeArticulationOverlapTurnChange / timeSpeech);
            measures.insert("RatioSilentPause", timeSilentPause / timeSpeech);
            measures.insert("RatioFilledPause", timeFilledPause / timeSpeech);
            measures.insert("NumTokens", numTokens);
            measures.insert("NumSyllables", numSyllablesArticulated);
            measures.insert("NumSilentPauses", numSilentPauses);
            measures.insert("NumFilledPauses", numFilledPauses);
            measures.insert("SpeechRate", numSyllablesArticulated / timeSpeech);
            measures.insert("SilentPauseRate", numSilentPauses / timeSpeech);
            measures.insert("FilledPauseRate", numFilledPauses / timeSpeech);
            measures.insert("ArticulationRate", numSyllablesArticulated / timeArticulation);
            StatisticalSummary summaryDurationsPauseSIL, summaryDurationsPauseFIL;
            StatisticalSummary summaryTurnDurations, summaryTurnTokenCounts, summaryTurnSyllableCounts;
            summaryDurationsPauseSIL.calculate(durationsPauseSIL);
            summaryDurationsPauseFIL.calculate(durationsPauseFIL);
            summaryTurnDurations.calculate(turnDurations);
            summaryTurnTokenCounts.calculate(turnTokenCounts);
            summaryTurnSyllableCounts.calculate(turnSyllableCounts);
            measures.insert("PauseDur_SIL_Median", summaryDurationsPauseSIL.median());
            measures.insert("PauseDur_SIL_Q1", summaryDurationsPauseSIL.firstQuartile());
            measures.insert("PauseDur_SIL_Q3", summaryDurationsPauseSIL.thirdQuartile());
            measures.insert("PauseDur_FIL_Median", summaryDurationsPauseFIL.median());
            measures.insert("PauseDur_FIL_Q1", summaryDurationsPauseFIL.firstQuartile());
            measures.insert("PauseDur_FIL_Q3", summaryDurationsPauseFIL.thirdQuartile());
            measures.insert("TurnDuration_Time_Median", summaryTurnDurations.median());
            measures.insert("TurnDuration_Token_Median", summaryTurnTokenCounts.median());
            measures.insert("TurnDuration_Syll_Median", summaryTurnSyllableCounts.median());
            d->measuresSpk.insert(speakerID, measures);
        }
        qDeleteAll(tiersAll);
    }
}

