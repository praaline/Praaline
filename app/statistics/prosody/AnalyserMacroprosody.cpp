#include <QObject>
#include <QString>
#include <QList>
#include <QStandardItemModel>
#include "math.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "AnalyserMacroprosody.h"

struct AnalyserMacroprosodyData {
    AnalyserMacroprosodyData() : levelPhones("phone"), levelSyllables("syll"), levelTokens("tok_min"), levelMacroUnits("segment"),
        attributeProminence("promise_pos"), model(0)
    {
        filledPauseTokens << "euh" << "euhm";
        prominentLabels << "P";
        getFirstLastSyllableValues = true;
    }

    QString levelPhones;
    QString levelSyllables;
    QString levelTokens;
    QString levelMacroUnits;
    QStringList filledPauseTokens;
    QString attributeProminence;
    QStringList prominentLabels;
    bool getFirstLastSyllableValues;

    QStandardItemModel *model;
    QStringList metadataAttributeIDsCom;
    QStringList metadataAttributeIDsSpk;
    QStringList metadataAttributeIDsAnnot;
};

AnalyserMacroprosody::AnalyserMacroprosody(QObject *parent)
    : QObject(parent), d(new AnalyserMacroprosodyData)
{
    d->model = new QStandardItemModel(this);
}

AnalyserMacroprosody::~AnalyserMacroprosody()
{
    delete d;
}

// static
QList<QString> AnalyserMacroprosody::groupingLevels()
{
    return QList<QString>() << "CommunicationID" << "AnnotationID" << "SpeakerID";
}

// static
QList<QString> AnalyserMacroprosody::measureIDs(const QString &groupingLevel)
{
    Q_UNUSED(groupingLevel)
    return QList<QString>() << "StartTime" << "EndTime" << "Duration"
                            << "StartSyll" << "EndSyll" << "TextTokens" << "TextSyllables"
                            << "TimeArticulation" << "TimeFilledPause" << "TimeSilentPause"
                            << "NumSyllArticulated" << "NumFilledPauses" << "NumSilentPauses"
                            << "RatioArticulation"
                            << "SpeechRate" << "ArticulationRate" << "RateFILandSyll"
                            << "TimeSilentPauseBefore" << "TimeSilentPauseAfter"
                            << "MeanPitch" << "MeanPitchStDev" << "PitchLow" << "PitchHigh" << "PitchRange"
                            << "MeanPitchFirstSyll" << "PitchLowFirstSyll" << "PitchHighFirstSyll" << "PitchStartFirstSyll"
                            << "MeanPitchLastSyll" << "PitchLowLastSyll" << "PitchHighLastSyll" << "PitchEndLastSyll"
                            << "NumRisingSyll" << "NumFallingSyll" << "NumProminentSyll"
                            << "RatioRisingSyll" << "RatioFallingSyll" << "RatioProminentSyll"
                            << "MelodicPath"
                            << "NumDisfluencyLEN" << "NumDisfluencyFST" << "NumDisfluencyREP" << "NumDisfluencyStruct"
                            << "DisfluencyText";
}

// static
StatisticalMeasureDefinition AnalyserMacroprosody::measureDefinition(const QString &groupingLevel, const QString &measureID)
{
    Q_UNUSED(groupingLevel)
    if (measureID == "StartTime")               return StatisticalMeasureDefinition("StartTime", "Start time of unit", "s");
    if (measureID == "EndTime")                 return StatisticalMeasureDefinition("EndTime", "End time of unit", "s");
    if (measureID == "Duration")                return StatisticalMeasureDefinition("Duration", "Duration of unit", "s");
    if (measureID == "StartSyll")               return StatisticalMeasureDefinition("StartSyll", "Syllable index at start of unit", "");
    if (measureID == "EndSyll")                 return StatisticalMeasureDefinition("EndSyll", "Syllable index at end of unit", "");
    if (measureID == "TextTokens")              return StatisticalMeasureDefinition("TextTokens", "Text of tokens", "");
    if (measureID == "TextSyllables")           return StatisticalMeasureDefinition("TextSyllables", "Text of syllables", "");
    if (measureID == "TimeArticulation")        return StatisticalMeasureDefinition("TimeArticulation", "Articulation time", "s");
    if (measureID == "TimeFilledPause")         return StatisticalMeasureDefinition("TimeFilledPause", "Filled pause time", "s");
    if (measureID == "TimeSilentPause")         return StatisticalMeasureDefinition("TimeSilentPause", "Sillent pausetime", "s");
    if (measureID == "NumSyllArticulated")      return StatisticalMeasureDefinition("NumSyllArticulated", "Number of articulated syllables (excluding filled pauses)", "");
    if (measureID == "NumFilledPauses")         return StatisticalMeasureDefinition("NumFilledPauses", "Number of filled pauses", "");
    if (measureID == "NumSilentPauses")         return StatisticalMeasureDefinition("NumSilentPauses", "Number of silent pauses", "");
    if (measureID == "SpeechRate")              return StatisticalMeasureDefinition("SpeechRate", "Speech rate", "syll/s", "All articulated syllables (excluding SIL and FIL) / Speech time");
    if (measureID == "ArticulationRate")        return StatisticalMeasureDefinition("ArticulationRate", "Articulation rate", "syll/s", "All articulated syllables (excluding SIL and FIL) / Articulation time");
    if (measureID == "RateFILandSyll")          return StatisticalMeasureDefinition("RateFILandSyll", "Articulation (incl FIL) rate", "syll/s", "All articulated syllables (including filled pauses) / Articulation + filled pause time");
    if (measureID == "TimeSilentPauseBefore")   return StatisticalMeasureDefinition("TimeSilentPauseBefore", "Silent pause time before unit", "s", "");
    if (measureID == "TimeSilentPauseAfter")    return StatisticalMeasureDefinition("TimeSilentPauseAfter", "Silent pause time after unit", "s", "");
    if (measureID == "MeanPitch")               return StatisticalMeasureDefinition("MeanPitch", "Mean of the mean pitch of all stylised syllables in unit", "ST");
    if (measureID == "MeanPitchStDev")          return StatisticalMeasureDefinition("MeanPitchStDev", "Standard deviation of the mean pitch of all stylised syllables in unit", "ST");
    if (measureID == "PitchLow")                return StatisticalMeasureDefinition("PitchLow", "Minimum of the low pitch of all stylised syllables in unit", "ST");
    if (measureID == "PitchHigh")               return StatisticalMeasureDefinition("PitchHigh", "Maximum of the high pitch of all stylised syllables in unit", "ST");
    if (measureID == "PitchRange")              return StatisticalMeasureDefinition("PitchRange", "Pitch range: pitch high - pitch low", "ST");
    if (measureID == "MeanPitchFirstSyll")      return StatisticalMeasureDefinition("MeanPitchFirstSyll", "Mean pitch of the unit's first syllable", "ST");
    if (measureID == "PitchLowFirstSyll")       return StatisticalMeasureDefinition("PitchLowFirstSyll", "Low pitch of the unit's first syllable", "ST");
    if (measureID == "PitchHighFirstSyll")      return StatisticalMeasureDefinition("PitchHighFirstSyll", "High pitch of the unit's first syllable", "ST");
    if (measureID == "PitchStartFirstSyll")     return StatisticalMeasureDefinition("PitchStartFirstSyll", "Start pitch of the unit's first syllable", "ST");
    if (measureID == "MeanPitchLastSyll")       return StatisticalMeasureDefinition("MeanPitchLastSyll", "Mean pitch of the unit's last syllable", "ST");
    if (measureID == "PitchLowLastSyll")        return StatisticalMeasureDefinition("PitchLowLastSyll", "Low pitch of the unit's last syllable", "ST");
    if (measureID == "PitchHighLastSyll")       return StatisticalMeasureDefinition("PitchHighLastSyll", "High pitch of the unit's last syllable", "ST");
    if (measureID == "PitchEndLastSyll")        return StatisticalMeasureDefinition("PitchEndLastSyll", "End pitch of the unit's last syllable", "ST");
    if (measureID == "NumRisingSyll")           return StatisticalMeasureDefinition("NumRisingSyll", "Number of rising syllables", "");
    if (measureID == "NumFallingSyll")          return StatisticalMeasureDefinition("NumFallingSyll", "Number of falling syllables", "");
    if (measureID == "NumProminentSyll")        return StatisticalMeasureDefinition("NumProminentSyll", "Number of prominent syllables", "");
    if (measureID == "RatioRisingSyll")         return StatisticalMeasureDefinition("RatioRisingSyll", "Percentage of rising syllables over articulated syllables", "%");
    if (measureID == "RatioFallingSyll")        return StatisticalMeasureDefinition("RatioFallingSyll", "Percentage of falling syllables over articulated syllables", "%");
    if (measureID == "RatioProminentSyll")      return StatisticalMeasureDefinition("RatioProminentSyll", "Percentage of prominent syllables over articulated syllables", "%");
    if (measureID == "MelodicPath")             return StatisticalMeasureDefinition("MelodicPath", "Melodic path", "ST/s", "Sum of absolute pitch interval (ST) of tonal segments in nucleus (rises and falls add up) / Sum of stylised nucleus time");
    if (measureID == "NumDisfluencyLEN")        return StatisticalMeasureDefinition("NumDisfluencyLEN", "Number of disfluencies: lengthening", "");
    if (measureID == "NumDisfluencyFST")        return StatisticalMeasureDefinition("NumDisfluencyFST", "Number of disfluencies: false starts", "");
    if (measureID == "NumDisfluencyREP")        return StatisticalMeasureDefinition("NumDisfluencyREP", "Number of disfluencies: repetitions", "");
    if (measureID == "NumDisfluencyStruct")     return StatisticalMeasureDefinition("NumDisfluencyStruct", "Number of disfluencies: structured", "");
    if (measureID == "DisfluencyText")          return StatisticalMeasureDefinition("DisfluencyText", "Disfluency text", "");
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

double AnalyserMacroprosody::measure(const QString &groupingLevel, const QString &key, const QString &measureID) const
{
    Q_UNUSED(groupingLevel)
    Q_UNUSED(key)
    Q_UNUSED(measureID)
    return 0;
}

QPointer<QStandardItemModel> AnalyserMacroprosody::model()
{
    return d->model;
}

double HzToSTre1Hz(double Hz)
{
    return 12.0 * log2(Hz);
}

void AnalyserMacroprosody::setMetadataAttributesCommunication(const QStringList &attributeIDs)
{
    d->metadataAttributeIDsCom = attributeIDs;
}

void AnalyserMacroprosody::setMetadataAttributesSpeaker(const QStringList &attributeIDs)
{
    d->metadataAttributeIDsSpk = attributeIDs;
}

void AnalyserMacroprosody::setMetadataAttributesAnnotation(const QStringList &attributeIDs)
{
    d->metadataAttributeIDsAnnot = attributeIDs;
}

void AnalyserMacroprosody::setMacroUnitsLevel(const QString &levelID)
{
    d->levelMacroUnits = levelID;
}

QString AnalyserMacroprosody::calculate(QPointer<Corpus> corpus, const QString &communicationID, const QString &annotationID, const QString &speakerIDfilter,
                                        const QList<Interval *> &units)
{
    if (!corpus) return tr("Error accessing corpus. No statistical analysis produced.");
    d->model->clear();  
    // Create model headers
    QStringList labels;
    labels << "CommunicationID" << d->metadataAttributeIDsCom;
    labels << "AnnotationID" << d->metadataAttributeIDsAnnot;
    labels << "SpeakerID" << d->metadataAttributeIDsSpk;
    if (!d->levelMacroUnits.isEmpty()) labels << "UnitNo" << d->levelMacroUnits;
    labels << measureIDs("");
    d->model->setHorizontalHeaderLabels(labels);
    // Process data
    SpeakerAnnotationTierGroupMap tiersAll;
    tiersAll = corpus->repository()->annotations()
            ->getTiersAllSpeakers(annotationID, QStringList() << d->levelPhones << d->levelSyllables << d->levelTokens);
    foreach (QString speakerID, tiersAll.keys()) {
        if ((!speakerIDfilter.isEmpty()) && (speakerIDfilter != speakerID)) continue;
        // Get metadata objects
        CorpusCommunication *com = corpus->communication(communicationID);
        QPointer<CorpusSpeaker> spk = corpus->speaker(speakerID);
        CorpusAnnotation *annot = (com) ? com->annotation(annotationID) : 0;
        // Get tiers for this speaker
        AnnotationTierGroup *tiers = tiersAll.value(speakerID);
        if (!tiers) continue;
        IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllables);
        IntervalTier *tier_tokmin = tiers->getIntervalTierByName(d->levelTokens);
        if (!tier_syll || !tier_tokmin) continue;
        // Selected units: either given by the user, or read from the tier
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
            double f0_mean_first(-1), f0_min_first(-1), f0_max_first(-1);
            double f0_mean_last(-1), f0_min_last(-1), f0_max_last(-1);
            double f0_start_first(-1), f0_end_last(-1);
            RealTime timeSilentPauseBefore, timeSilentPauseAfter;

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
            // Check for pauses before and after the unit
            if (syllIndices.first - 1 >= 0) {
                if (tier_syll->at(syllIndices.first - 1)->isPauseSilent())
                    timeSilentPauseBefore = tier_syll->at(syllIndices.first - 1)->duration();
            }
            if (syllIndices.second + 1 < tier_syll->count()) {
                if (tier_syll->at(syllIndices.second + 1)->isPauseSilent())
                    timeSilentPauseAfter = tier_syll->at(syllIndices.second + 1)->duration();
            }
            // For each syllable...
            for (int i = syllIndices.first; i <= syllIndices.second; ++i) {
                Interval *syll = tier_syll->interval(i);
                if (!syll) continue;
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
                    // First and last syllable
                    if (i == syllIndices.first) {
                        f0_mean_first = syll->attribute("f0_mean").toDouble();
                        f0_min_first = HzToSTre1Hz(syll->attribute("f0_min").toDouble());
                        f0_max_first = HzToSTre1Hz(syll->attribute("f0_max").toDouble());
                        f0_max_first = HzToSTre1Hz(syll->attribute("f0_max").toDouble());
                        f0_start_first = HzToSTre1Hz(syll->attribute("f0_start").toDouble());
                    }
                    if (i == syllIndices.second) {
                        f0_mean_last = syll->attribute("f0_mean").toDouble();
                        f0_min_last = HzToSTre1Hz(syll->attribute("f0_min").toDouble());
                        f0_max_last = HzToSTre1Hz(syll->attribute("f0_max").toDouble());
                        f0_end_last = HzToSTre1Hz(syll->attribute("f0_end").toDouble());
                    }
                }
                // Prominence attributes
                QString prom = syll->attribute(d->attributeProminence).toString();
                if (d->prominentLabels.contains(prom)) numProminentSyll++;
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
            // Identifiers and Metadata
            item = new QStandardItem(); item->setData(communicationID, Qt::DisplayRole); items << item;
            foreach (QString attributeID, d->metadataAttributeIDsCom) {
                item = new QStandardItem(); if (com) item->setData(com->property(attributeID), Qt::DisplayRole); items << item;
            }
            item = new QStandardItem(); item->setData(annotationID, Qt::DisplayRole); items << item;
            foreach (QString attributeID, d->metadataAttributeIDsAnnot) {
                item = new QStandardItem(); if (annot) item->setData(annot->property(attributeID), Qt::DisplayRole); items << item;
            }
            item = new QStandardItem(); item->setData(speakerID, Qt::DisplayRole); items << item;
            foreach (QString attributeID, d->metadataAttributeIDsSpk) {
                item = new QStandardItem(); if (spk) item->setData(spk->property(attributeID), Qt::DisplayRole); items << item;
            }
            // Unit data
            item = new QStandardItem(); item->setData(unitNo, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->text().replace("\t", " ").replace("\n", " "), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->tMin().toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->tMax().toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(unit->duration().toDouble(), Qt::DisplayRole); items << item;
            // unit contents in tokens and syllables
            item = new QStandardItem(); item->setData(syllIndices.first, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(syllIndices.second, Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(textTokens.replace("\t", " ").replace("\n", " "), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(textSylls.replace("\t", " ").replace("\n", " "), Qt::DisplayRole); items << item;
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
            // pause before and after
            item = new QStandardItem(); item->setData(timeSilentPauseBefore.toDouble(), Qt::DisplayRole); items << item;
            item = new QStandardItem(); item->setData(timeSilentPauseAfter.toDouble(), Qt::DisplayRole); items << item;
//            item = new QStandardItem(); item->setData(timeStylisedNuclei.toDouble(), Qt::DisplayRole); items << item;
//            item = new QStandardItem(); item->setData(numNuclei, Qt::DisplayRole); items << item;
            // pitch
            StatisticalSummary summary_f0_means(f0_means), summary_f0_minima(f0_minima), summary_f0_maxima(f0_maxima);
            if (numNuclei > 0) {
                item = new QStandardItem(); item->setData(summary_f0_means.mean(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_means.stDev(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_minima.min(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_maxima.max(), Qt::DisplayRole); items << item;
                item = new QStandardItem(); item->setData(summary_f0_maxima.max() - summary_f0_minima.min(), Qt::DisplayRole); items << item;
            }
            else {
                items << new QStandardItem("NA") << new QStandardItem("NA") << new QStandardItem("NA") << new QStandardItem("NA");
            }
            if (d->getFirstLastSyllableValues) {
                // pitch of first and last syllable
                item = new QStandardItem(); if (f0_mean_first  >= 0) item->setData(f0_mean_first,  Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_min_first   >= 0) item->setData(f0_min_first,   Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_max_first   >= 0) item->setData(f0_max_first,   Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_start_first >= 0) item->setData(f0_start_first, Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_mean_last   >= 0) item->setData(f0_mean_last,   Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_min_last    >= 0) item->setData(f0_min_last,    Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_max_last    >= 0) item->setData(f0_max_last,    Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
                item = new QStandardItem(); if (f0_end_last    >= 0) item->setData(f0_end_last,    Qt::DisplayRole); else item->setData("NA", Qt::DisplayRole); items << item;
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

QString AnalyserMacroprosody::calculate(QPointer<Corpus> corpus, CorpusCommunication *com)
{
    if (!corpus || !com) return "Error";
    foreach (QString annotationID, com->annotationIDs()) {
        calculate(corpus, com->ID(), annotationID);
    }
    return QString();
}
