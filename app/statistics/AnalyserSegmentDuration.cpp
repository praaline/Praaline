#include <QString>
#include <QHash>
#include <qmath.h>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"

#include "AnalyserSegmentDuration.h"


struct AnalyserSegmentDurationData
{
    AnalyserSegmentDurationData() : levelPhones("phone"), levelSyllables("syll")
    {
        // Vowels, nasal vowels and schwa
        vowels << "i" << "e" << "E" << "a" << "A" << "o" << "O" << "y" << "2" << "9" << "u"
               << "U~" << "e~" << "a~" << "o~" << "9~" << "@";
    }

    QString levelPhones;
    QString levelSyllables;
    QList<QString> vowels;
};

AnalyserSegmentDuration::AnalyserSegmentDuration(QObject *parent)
    : QObject(parent), d(new AnalyserSegmentDurationData)
{

}

AnalyserSegmentDuration::~AnalyserSegmentDuration()
{
    delete d;
}

IntervalTier *AnalyserSegmentDuration::categorizePhones(IntervalTier *tier_phone)
{
    if (!tier_phone) return 0;
    IntervalTier *tier_cv = new IntervalTier(tier_phone, "phone_cv", false);
    foreach (Interval *cvseg, tier_cv->intervals()) {
        cvseg->setText( d->vowels.contains(cvseg->text()) ? "V" : "C" );
    }
    return tier_cv;
}

QList<QString> AnalyserSegmentDuration::measureIDs(const QString &groupingLevel)
{
    Q_UNUSED(groupingLevel)
    return QList<QString>()
            << "TimeSpeech" << "TimeArticulation"
            << "TimeArticulation_Alone" << "TimeArticulation_Overlap" << "TimeArticulation_Overlap_Continue" << "TimeArticulation_Overlap_TurnChange";
}

StatisticalMeasureDefinition AnalyserSegmentDuration::measureDefinition(const QString &groupingLevel, const QString &measureID)
{
    if (measureID == "TimeTotalSample")          return StatisticalMeasureDefinition("TimeTotalSample", "Total sample time", "s");
    if (measureID == "TimeSingleSpeaker")        return StatisticalMeasureDefinition("TimeSingleSpeaker", "Single-speaker time", "s");
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

void AnalyserSegmentDuration::calculateStatistics(const QList<Interval *> &intervals, const QString &name, RealTime threshold)
{
    RealTime sumNonPauseDuration;   // total duration of non-pause intervals
    double rPVI = 0;                // raw Pairwise Variability Index
    double nPVI = 0;                // normalized Pairwise Variability Index

    QList<double> durations, lognDurations;
    foreach (Interval *intv, intervals) {
        if (intv->duration() < threshold) continue;
        if (true) {
            double duration = intv->duration().toDouble();
            double lognDuration = qLn(duration);
            durations << duration;
            lognDurations << lognDuration;
        }
        if (!intv->isPauseSilent())
            sumNonPauseDuration = sumNonPauseDuration + intv->duration();
    }
    StatisticalSummary summaryDuration;
    summaryDuration.calculate(durations);
}






