#include <QObject>
#include <QString>
#include <QList>
#include <QStandardItemModel>
#include "math.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "AnalyserPitch.h"

struct AnalyserPitchData {
    AnalyserPitchData() : levelPhones("phone"), levelSyllables("syll"), levelTokens("tok_min"), levelMacroUnits("segment"),
        attributeProminence("promise_pos"), model(0)
    {
        filledPauseTokens << "euh" << "euhm";
        prominentLabels << "P";
    }

    QString levelPhones;
    QString levelSyllables;
    QString levelTokens;
    QString levelMacroUnits;
    QStringList filledPauseTokens;
    QString attributeProminence;
    QStringList prominentLabels;

    QStandardItemModel *model;
};

AnalyserPitch::AnalyserPitch(QObject *parent)
    : QObject(parent), d(new AnalyserPitchData)
{
    d->model = new QStandardItemModel(this);
}

AnalyserPitch::~AnalyserPitch()
{
    delete d;
}

// static
QList<QString> AnalyserPitch::groupingLevels()
{
    return QList<QString>() << "CommunicationID" << "AnnotationID" << "SpeakerID";
}

// static
QList<QString> AnalyserPitch::measureIDs(const QString &groupingLevel)
{
    return QList<QString>() << "StartTime" << "EndTime" << "Duration"
                            << "StartSyll" << "EndSyll" << "TextTokens" << "TextSyllables"
                            << "TimeArticulation" << "TimeFilledPause" << "TimeSilentPause"
                            << "NumSyllArticulated" << "NumFilledPauses" << "NumSilentPauses"
                            << "RatioArticulation"
                            << "SpeechRate" << "ArticulationRate" << "RateFILandSyll"
                            << "MeanPitch" << "PitchLow" << "PitchHigh" << "PitchRange"
                            << "NumRisingSyll" << "NumFallingSyll" << "NumProminentSyll"
                            << "RatioRisingSyll" << "RatioFallingSyll" << "RatioProminentSyll"
                            << "MelodicPath"
                            << "NumDisfluencyLEN" << "NumDisfluencyFST" << "NumDisfluencyREP" << "NumDisfluencyStruct"
                            << "DisfluencyText";
}

// static
StatisticalMeasureDefinition AnalyserPitch::measureDefinition(const QString &groupingLevel, const QString &measureID)
{
    if (measureID == "TimeTotalSample")          return StatisticalMeasureDefinition("TimeTotalSample", "Total sample time", "s");
    if (measureID == "TimeSingleSpeaker")        return StatisticalMeasureDefinition("TimeSingleSpeaker", "Single-speaker time", "s");
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

double AnalyserPitch::measure(const QString &groupingLevel, const QString &key, const QString &measureID) const
{
    Q_UNUSED(groupingLevel)

}

QPointer<QStandardItemModel> AnalyserPitch::model()
{
    return d->model;
}

double HzToSTre1Hz(double Hz)
{
    return 12.0 * log2(Hz);
}

QString AnalyserPitch::calculate(QPointer<Corpus> corpus, const QString &communicationID, const QString &annotationID,
                                 const QList<Interval *> &units)
{
    if (!corpus) return tr("Error accessing corpus. No statistical analysis produced.");
    d->model->clear();
    // Create model headers
    QStringList labels;
    labels << "CommunicationID" << "AnnotationID" << "SpeakerID";
    if (!d->levelMacroUnits.isEmpty()) labels << "Unit No" << d->levelMacroUnits;
    labels << measureIDs("");
    d->model->setHorizontalHeaderLabels(labels);
    // Process data
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()
            ->getTiersAllSpeakers(annotationID, QStringList() << d->levelPhones << d->levelSyllables << d->levelTokens);
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        if (!tiers) continue;
        IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllables);
        IntervalTier *tier_tokmin = tiers->getIntervalTierByName(d->levelTokens);
        if (!tier_syll || !tier_tokmin) continue;
        // Selected units
        QList<Interval *> selection(units);
        if (units.isEmpty() && !d->levelMacroUnits.isEmpty()) {
            IntervalTier *tier_units = tiers->getIntervalTierByName(d->levelMacroUnits);
            if (!tier_units) continue;
            selection = tier_units->intervals();
        }
        // Per unit measures
        int unitNo(1);
        foreach (Interval *u, selection) {
            // QList<Interval *> tok = tier_tokmin->getIntervalsOverlappingWith(u);
            Interval *unit = new Interval(tier_tokmin->getBoundaryClosestTo(u->tMin()),
                                          tier_tokmin->getBoundaryClosestTo(u->tMax()), u->text());

            QString textTokens, textSylls;
            RealTime timeSilentPause, timeFilledPause, timeArticulation, timeStylisedNuclei;
            int numSilentPauses(0), numFilledPauses(0), numSyllablesArticulated(0), numNuclei(0), numTokens(0);
            QList<double> f0_means, f0_minima, f0_maxima;
            int numRisingSyll(0), numFallingSyll(0), numProminentSyll(0);
            double sumTrajectory(0);
            int numDisfluencyLEN(0), numDisfluencyFST(0), numDisfluencyREP(0), numDisfluencyStruct(0);

            // For each token
            foreach (Interval *token, tier_tokmin->getIntervalsContainedIn(unit)) {
                // Count non-pause, non-filled pause tokens in the unit
                if ((!token->isPauseSilent()) && (!d->filledPauseTokens.contains(token->text()))) numTokens++;
                textTokens.append(token->text()).append(" ");
                // Count disfluencies in unit
                if (token->attribute("disfluency").toString().contains("LEN") || token->attribute("dis2_lex").toString().contains("LEN"))
                    ++numDisfluencyLEN;
                if (token->attribute("disfluency").toString().contains("FST") || token->attribute("dis2_lex").toString().contains("FST"))
                    ++numDisfluencyFST;
                if (token->attribute("disfluency").toString().contains("REP*") || token->attribute("dis2_rep").toString().contains("REP*"))
                    ++numDisfluencyREP;
                if (token->attribute("disfluency").toString().contains("INS*") || token->attribute("dis2_struct").toString().contains("INS*") ||
                    token->attribute("disfluency").toString().contains("SUB*") || token->attribute("dis2_struct").toString().contains("SUB*") ||
                    token->attribute("disfluency").toString().contains("DEL*") || token->attribute("dis2_struct").toString().contains("DEL*"))
                    ++numDisfluencyStruct;
            }
            if (!textTokens.isEmpty()) textTokens.chop(1);

            // The basic units of time measurement are the speaker's syllables
            QPair<int, int> syllIndices = tier_syll->getIntervalIndexesContainedIn(unit);
            for (int i = syllIndices.first; i <= syllIndices.second; ++i) {
                Interval *syll = tier_syll->interval(i);
                textSylls.append(syll->text()).append(" ");
                QList<Interval *> tokens = tier_tokmin->getIntervalsOverlappingWith(syll);
                // foreach (Interval *token, tokens) qDebug() << token->text();
                if (tokens.count() == 1 && d->filledPauseTokens.contains(tokens.first()->text())) {
                    timeFilledPause = timeFilledPause + syll->duration();
                    numFilledPauses++;
                }
                else if (syll->isPauseSilent()) {
                    timeSilentPause = timeSilentPause + syll->duration();
                    numSilentPauses++;
                }
                else {
                    timeArticulation = timeArticulation + syll->duration();
                    numSyllablesArticulated++;
                }
                // Pitch-prosodic measures calculated on all stylised syllables, including FIL
                if (syll->attribute("nucl_t1").toDouble() > 0 && syll->attribute("nucl_t2").toDouble() > 0) {
                    RealTime tMinNucl = RealTime::fromSeconds(syll->attribute("nucl_t1").toDouble());
                    RealTime tMaxNucl = RealTime::fromSeconds(syll->attribute("nucl_t2").toDouble());
                    timeStylisedNuclei = timeStylisedNuclei + (tMaxNucl - tMinNucl);
                    numNuclei++;
                    f0_means << syll->attribute("f0_mean").toDouble();
                    f0_minima << HzToSTre1Hz(syll->attribute("f0_min").toDouble());
                    f0_maxima << HzToSTre1Hz(syll->attribute("f0_max").toDouble());
                    int dynamic = syll->attribute("dynamic").toInt();
                    if      (dynamic < 0) numFallingSyll++;
                    else if (dynamic > 0) numRisingSyll++;
                    sumTrajectory = sumTrajectory + syll->attribute("trajectory").toDouble();
                    QString prom = syll->attribute(d->attributeProminence).toString();
                    if (d->prominentLabels.contains(prom)) numProminentSyll++;
                }
            } // end foreach syll
            // Calculate measures for this unit
            RealTime timeSpeech = timeArticulation + timeFilledPause + timeSilentPause;
            double ratioArticulation = timeArticulation.toDouble() / timeSpeech.toDouble();
            double rateSpeech = ((double)(numSyllablesArticulated + numFilledPauses)) / timeSpeech.toDouble();
            double rateArticulation = ((double)numSyllablesArticulated) / timeArticulation.toDouble();
            double rateArtSyllAndFIL = ((double)numSyllablesArticulated + numFilledPauses) / (timeArticulation.toDouble() + timeFilledPause.toDouble());
            double ratioRisingSyll = ((double)numRisingSyll) / ((double)numSyllablesArticulated);
            double ratioFallingSyll = ((double)numFallingSyll) / ((double)numSyllablesArticulated);
            double ratioProminentSyll = ((double)numProminentSyll) / ((double)numSyllablesArticulated);
            double melodicPath = sumTrajectory / timeStylisedNuclei.toDouble();

            QList<QStandardItem *> items;
            QStandardItem *item;
            item = new QStandardItem(); item->setData(communicationID, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(annotationID, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(speakerID, Qt::DisplayRole); items << item;
            // unit data
            item = new QStandardItem(); item->setData(unitNo, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->text(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->tMin().toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->tMax().toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->duration().toDouble(), Qt::DisplayRole); items << item;
            // unit contents in tokens and syllables
            item = new QStandardItem(); item->setData(syllIndices.first, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(syllIndices.second, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(textTokens, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(textSylls, Qt::DisplayRole); items << item;
            // times
            item = new QStandardItem(); item->setData(timeArticulation.toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(timeFilledPause.toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(timeSilentPause.toDouble(), Qt::DisplayRole); items << item;
            // number
            item = new QStandardItem(); item->setData(numSyllablesArticulated, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numFilledPauses, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numSilentPauses, Qt::DisplayRole); items << item;
            // ratios
            item = new QStandardItem(); item->setData(ratioArticulation, Qt::DisplayRole); items << item;
            // rates
            item = new QStandardItem(); item->setData(rateSpeech, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(rateArticulation, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(rateArtSyllAndFIL, Qt::DisplayRole); items << item;
//            item = new QStandardItem(); item->setData(timeStylisedNuclei.toDouble(), Qt::DisplayRole); items << item;
//            item = new QStandardItem(); item->setData(numNuclei, Qt::DisplayRole); items << item;
            // pitch
            StatisticalSummary summary_f0_means(f0_means), summary_f0_minima(f0_minima), summary_f0_maxima(f0_maxima);
            if (numNuclei > 0) {
                item = new QStandardItem(); item->setData(summary_f0_means.mean(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_minima.min(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_maxima.max(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_maxima.max() - summary_f0_minima.min(), Qt::DisplayRole); items << item;
            }
            else {
                items << new QStandardItem("NA") << new QStandardItem("NA") << new QStandardItem("NA") << new QStandardItem("NA");
            }
            // rises, falls, prominences
            item = new QStandardItem(); item->setData(numRisingSyll, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numFallingSyll, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numProminentSyll, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(ratioRisingSyll, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(ratioFallingSyll, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(ratioProminentSyll, Qt::DisplayRole); items << item;
            // melodic path
            item = new QStandardItem(); item->setData(melodicPath, Qt::DisplayRole); items << item;
            // disfluencies
            item = new QStandardItem(); item->setData(numDisfluencyLEN, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numDisfluencyFST, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numDisfluencyREP, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(numDisfluencyStruct, Qt::DisplayRole); items << item;
            d->model->appendRow(items);
            unitNo++;
        } // end foreach unit
    } // end foreach speaker
    return QString();
}

QString AnalyserPitch::calculate(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    if (!corpus || !com) return "Error";
    foreach (QString annotationID, com->annotationIDs()) {
        calculate(corpus, com->ID(), annotationID);
    }
}
