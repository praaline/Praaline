#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/SequenceTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Statistics/Measures.h"
#include "PraalineCore/Statistics/StatisticalMeasureDefinition.h"
#include "PraalineCore/Statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "AnalyserDisfluenciesItem.h"

// ====================================================================================================================
// Definition of measures
// ====================================================================================================================

QStringList AnalyserDisfluenciesDefinitions::measureIDsForCommunication()
{
    return QStringList();
}

QStringList AnalyserDisfluenciesDefinitions::measureIDsForSpeaker()
{
    return QStringList()
            << "Count_AllDisfluencies" << "Count_FIL" << "Count_FST" << "Count_LEN" << "Count_WDP" << "Count_MSP" << "Count_SingleToken"
            << "Count_REP" << "Count_RED" << "Count_Repetitions" << "Count_DEL" << "Count_INS" << "Count_SUB" << "Count_Structured" << "Count_Complex"
            << "Count_AllStuttered" << "Count_BLOC" << "Count_BLEN" << "Count_BREP" << "Count_BRED" << "Count_BCOM"

            << "CountTokens_AllDisfluencies" << "CountTokens_REP" << "CountTokens_RED" << "CountTokens_Repetitions"
            << "CountTokens_DEL" << "CountTokens_INS" << "CountTokens_SUB" << "CountTokens_Structured" << "CountTokens_Complex"
            << "CountTokens_AllStuttered" << "CountTokens_BREP" << "CountTokens_BRED" << "CountTokens_BCOM"

            << "CountDisflTok_AllDisfluencies" << "CountDisflTok_REP" << "CountDisflTok_RED" << "CountDisflTok_Repetitions"
            << "CountDisflTok_DEL" << "CountDisflTok_INS" << "CountDisflTok_SUB" << "CountDisflTok_Structured" << "CountDisflTok_Complex"
            << "CountDisflTok_AllStuttered" << "CountDisflTok_BREP" << "CountDisflTok_BRED" << "CountDisflTok_BCOM"

            << "Ratio_AllDisfluencies" << "Ratio_FIL" << "Ratio_FST" << "Ratio_LEN" << "Ratio_WDP" << "Ratio_MSP" << "Ratio_SingleToken"
            << "Ratio_REP" << "Ratio_RED" << "Ratio_Repetitions" << "Ratio_DEL" << "Ratio_INS" << "Ratio_SUB" << "Ratio_Structured" << "Ratio_Complex"
            << "Ratio_AllStuttered" << "Ratio_BLOC" << "Ratio_BLEN" << "Ratio_BREP" << "Ratio_BRED" << "Ratio_BCOM"

            << "RatioTokens_AllDisfluencies" << "RatioTokens_REP" << "RatioTokens_RED" << "RatioTokens_Repetitions"
            << "RatioTokens_DEL" << "RatioTokens_INS" << "RatioTokens_SUB" << "RatioTokens_Structured" << "RatioTokens_Complex"
            << "RatioTokens_AllStuttered" << "RatioTokens_BREP" << "RatioTokens_BRED" << "RatioTokens_BCOM"

            << "RatioDisflTok_AllDisfluencies" << "RatioDisflTok_REP" << "RatioDisflTok_RED" << "RatioDisflTok_Repetitions"
            << "RatioDisflTok_DEL" << "RatioDisflTok_INS" << "RatioDisflTok_SUB" << "RatioDisflTok_Structured" << "RatioDisflTok_Complex"
            << "RatioDisflTok_AllStuttered" << "RatioDisflTok_BREP" << "RatioDisflTok_BRED" << "RatioDisflTok_BCOM"

            << "TokensBetweenDisfluencies_Median" << "TokensBetweenDisfluencies_Q1" << "TokensBetweenDisfluencies_Q3";
}

QStringList AnalyserDisfluenciesDefinitions::vectorMeasureIDsForCommunication()
{
    return QStringList();
}

QStringList AnalyserDisfluenciesDefinitions::vectorMeasureIDsForSpeaker()
{
    return QStringList() << "TokensBetweenDisfluencies";
}

Praaline::Core::StatisticalMeasureDefinition AnalyserDisfluenciesDefinitions::measureDefinition(const QString &measureID)
{
    // Measures per Speaker
    if (measureID == "Count_AllDisfluencies")   return StatisticalMeasureDefinition("Count_AllDisfluencies", "Number of disfluencies (total)", "", "");
    if (measureID == "Count_FIL")               return StatisticalMeasureDefinition("Count_FIL", "Number of filled pauses", "");
    if (measureID == "Count_FST")               return StatisticalMeasureDefinition("Count_FST", "Number of false starts", "");
    if (measureID == "Count_LEN")               return StatisticalMeasureDefinition("Count_LEN", "Number of lengthenings", "");
    if (measureID == "Count_WDP")               return StatisticalMeasureDefinition("Count_WDP", "Number of pauses-within-words", "");
    if (measureID == "Count_MSP")               return StatisticalMeasureDefinition("Count_MSP", "Number of mispronunciations", "");
    if (measureID == "Count_SingleToken")       return StatisticalMeasureDefinition("Count_SingleToken", "Total single-token disfluencies", "");
    if (measureID == "Count_REP")               return StatisticalMeasureDefinition("Count_REP", "Number of repetitions of full words", "");
    if (measureID == "Count_RED")               return StatisticalMeasureDefinition("Count_RED", "Number of repetitions with fragments", "");
    if (measureID == "Count_Repetitions")       return StatisticalMeasureDefinition("Count_Repetitions", "Total repetitions", "");
    if (measureID == "Count_DEL")               return StatisticalMeasureDefinition("Count_DEL", "Number of deletions", "");
    if (measureID == "Count_INS")               return StatisticalMeasureDefinition("Count_INS", "Number of insertions", "");
    if (measureID == "Count_SUB")               return StatisticalMeasureDefinition("Count_SUB", "Number of substitutions", "");
    if (measureID == "Count_Structured")        return StatisticalMeasureDefinition("Count_Structured", "Total structured disfluencies", "");
    if (measureID == "Count_Complex")           return StatisticalMeasureDefinition("Count_Complex", "Number of complex editing disfluencies", "");
    if (measureID == "Count_AllStuttered")      return StatisticalMeasureDefinition("Count_AllStuttered", "Number of stuttered disfluencies (total)", "", "");
    if (measureID == "Count_BLOC")              return StatisticalMeasureDefinition("Count_BLOC", "Number of stuttered blocks", "");
    if (measureID == "Count_BLEN")              return StatisticalMeasureDefinition("Count_BLEN", "Number of stuttered prolongations", "");
    if (measureID == "Count_BREP")              return StatisticalMeasureDefinition("Count_BREP", "Number of stuttered repetitions of full words", "");
    if (measureID == "Count_BRED")              return StatisticalMeasureDefinition("Count_BRED", "Number of stuttered repetitions with fragments", "");
    if (measureID == "Count_BCOM")              return StatisticalMeasureDefinition("Count_BCOM", "Number of stuttered complex editing disfluencies", "");

    if (measureID == "CountTokens_AllDisfluencies")   return StatisticalMeasureDefinition("CountTokens_AllDisfluencies", "Number of tokens in disfluencies (total)", "", "");
    if (measureID == "CountTokens_REP")               return StatisticalMeasureDefinition("CountTokens_REP", "Number of tokens in repetitions of full words", "");
    if (measureID == "CountTokens_RED")               return StatisticalMeasureDefinition("CountTokens_RED", "Number of tokens in repetitions with false starts", "");
    if (measureID == "CountTokens_Repetitions")       return StatisticalMeasureDefinition("CountTokens_Repetitions", "Total tokens in repetitions", "");
    if (measureID == "CountTokens_DEL")               return StatisticalMeasureDefinition("CountTokens_DEL", "Number of tokens in deletions", "");
    if (measureID == "CountTokens_INS")               return StatisticalMeasureDefinition("CountTokens_INS", "Number of tokens in insertions", "");
    if (measureID == "CountTokens_SUB")               return StatisticalMeasureDefinition("CountTokens_SUB", "Number of tokens in substitutions", "");
    if (measureID == "CountTokens_Structured")        return StatisticalMeasureDefinition("CountTokens_Structured", "Total tokens in structured disfluencies", "");
    if (measureID == "CountTokens_Complex")           return StatisticalMeasureDefinition("CountTokens_Complex", "Number of tokens in complex editing disfluencies", "");
    if (measureID == "CountTokens_AllStuttered")      return StatisticalMeasureDefinition("CountTokens_AllStuttered", "Number of tokens in stuttered disfluencies (total)", "", "");
    if (measureID == "CountTokens_BREP")              return StatisticalMeasureDefinition("CountTokens_BREP", "Number of tokens in stuttered repetitions of full words", "");
    if (measureID == "CountTokens_BRED")              return StatisticalMeasureDefinition("CountTokens_BRED", "Number of tokens in stuttered repetitions with fragments", "");
    if (measureID == "CountTokens_BCOM")              return StatisticalMeasureDefinition("CountTokens_BCOM", "Number of tokens in stuttered complex editing disfluencies", "");

    if (measureID == "CountDisflTok_AllDisfluencies")   return StatisticalMeasureDefinition("CountDisflTok_AllDisfluencies", "Number of disfluent tokens in disfluencies (total)", "", "");
    if (measureID == "CountDisflTok_REP")               return StatisticalMeasureDefinition("CountDisflTok_REP", "Number of disfluent tokens in repetitions of full words", "");
    if (measureID == "CountDisflTok_RED")               return StatisticalMeasureDefinition("CountDisflTok_RED", "Number of disfluent tokens in repetitions with false starts", "");
    if (measureID == "CountDisflTok_Repetitions")       return StatisticalMeasureDefinition("CountDisflTok_Repetitions", "Total disfluent tokens in repetitions", "");
    if (measureID == "CountDisflTok_DEL")               return StatisticalMeasureDefinition("CountDisflTok_DEL", "Number of disfluent tokens in deletions", "");
    if (measureID == "CountDisflTok_INS")               return StatisticalMeasureDefinition("CountDisflTok_INS", "Number of disfluent tokens in insertions", "");
    if (measureID == "CountDisflTok_SUB")               return StatisticalMeasureDefinition("CountDisflTok_SUB", "Number of disfluent tokens in substitutions", "");
    if (measureID == "CountDisflTok_Structured")        return StatisticalMeasureDefinition("CountDisflTok_Structured", "Total disfluent tokens in structured disfluencies", "");
    if (measureID == "CountDisflTok_Complex")           return StatisticalMeasureDefinition("CountDisflTok_Complex", "Number of disfluent tokens in complex editing disfluencies", "");
    if (measureID == "CountDisflTok_AllStuttered")      return StatisticalMeasureDefinition("CountDisflTok_AllStuttered", "Number of disfluent tokens in stuttered disfluencies (total)", "", "");
    if (measureID == "CountDisflTok_BREP")              return StatisticalMeasureDefinition("CountDisflTok_BREP", "Number of disfluent tokens in stuttered repetitions of full words", "");
    if (measureID == "CountDisflTok_BRED")              return StatisticalMeasureDefinition("CountDisflTok_BRED", "Number of disfluent tokens in stuttered repetitions with fragments", "");
    if (measureID == "CountDisflTok_BCOM")              return StatisticalMeasureDefinition("CountDisflTok_BCOM", "Number of disfluent tokens in stuttered complex editing disfluencies", "");

    if (measureID == "Ratio_AllDisfluencies")   return StatisticalMeasureDefinition("Ratio_AllDisfluencies", "Ratio of disfluencies (total)", "", "");
    if (measureID == "Ratio_FIL")               return StatisticalMeasureDefinition("Ratio_FIL", "Ratio of filled pauses", "");
    if (measureID == "Ratio_FST")               return StatisticalMeasureDefinition("Ratio_FST", "Ratio of false starts", "");
    if (measureID == "Ratio_LEN")               return StatisticalMeasureDefinition("Ratio_LEN", "Ratio of lengthenings", "");
    if (measureID == "Ratio_WDP")               return StatisticalMeasureDefinition("Ratio_WDP", "Ratio of pauses-within-words", "");
    if (measureID == "Ratio_MSP")               return StatisticalMeasureDefinition("Ratio_MSP", "Ratio of mispronunciations", "");
    if (measureID == "Ratio_SingleToken")       return StatisticalMeasureDefinition("Ratio_SingleToken", "Ratio of single-token disfluencies", "");
    if (measureID == "Ratio_REP")               return StatisticalMeasureDefinition("Ratio_REP", "Ratio of repetitions of full words", "");
    if (measureID == "Ratio_RED")               return StatisticalMeasureDefinition("Ratio_RED", "Ratio of repetitions with false starts", "");
    if (measureID == "Ratio_Repetitions")       return StatisticalMeasureDefinition("Ratio_Repetitions", "Ratio of repetitions", "");
    if (measureID == "Ratio_DEL")               return StatisticalMeasureDefinition("Ratio_DEL", "Ratio of deletions", "");
    if (measureID == "Ratio_INS")               return StatisticalMeasureDefinition("Ratio_INS", "Ratio of insertions", "");
    if (measureID == "Ratio_SUB")               return StatisticalMeasureDefinition("Ratio_SUB", "Ratio of substitutions", "");
    if (measureID == "Ratio_Structured")        return StatisticalMeasureDefinition("Ratio_Structured", "Ratio of structured disfluencies", "");
    if (measureID == "Ratio_Complex")           return StatisticalMeasureDefinition("Ratio_Complex", "Ratio of complex editing disfluencies", "");
    if (measureID == "Ratio_AllStuttered")      return StatisticalMeasureDefinition("Ratio_AllStuttered", "Ratio of stuttered disfluencies (total)", "", "");
    if (measureID == "Ratio_BLOC")              return StatisticalMeasureDefinition("Ratio_BLOC", "Ratio of stuttered blocks", "");
    if (measureID == "Ratio_BLEN")              return StatisticalMeasureDefinition("Ratio_BLEN", "Ratio of stuttered prolongations", "");
    if (measureID == "Ratio_BREP")              return StatisticalMeasureDefinition("Ratio_BREP", "Ratio of stuttered repetitions of full words", "");
    if (measureID == "Ratio_BRED")              return StatisticalMeasureDefinition("Ratio_BRED", "Ratio of stuttered repetitions with fragments", "");
    if (measureID == "Ratio_BCOM")              return StatisticalMeasureDefinition("Ratio_BCOM", "Ratio of stuttered complex editing disfluencies", "");

    if (measureID == "RatioTokens_AllDisfluencies")   return StatisticalMeasureDefinition("RatioTokens_AllDisfluencies", "Ratio of tokens in disfluencies (total)", "", "Total number of disfluencies (all types)");
    if (measureID == "RatioTokens_REP")               return StatisticalMeasureDefinition("RatioTokens_REP", "Ratio of tokens in repetitions of full words", "");
    if (measureID == "RatioTokens_RED")               return StatisticalMeasureDefinition("RatioTokens_RED", "Ratio of tokens in repetitions with false starts", "");
    if (measureID == "RatioTokens_Repetitions")       return StatisticalMeasureDefinition("RatioTokens_Repetitions", "Ratio of tokens in repetitions", "");
    if (measureID == "RatioTokens_DEL")               return StatisticalMeasureDefinition("RatioTokens_DEL", "Ratio of tokens in deletions", "");
    if (measureID == "RatioTokens_INS")               return StatisticalMeasureDefinition("RatioTokens_INS", "Ratio of tokens in insertions", "");
    if (measureID == "RatioTokens_SUB")               return StatisticalMeasureDefinition("RatioTokens_SUB", "Ratio of tokens in substitutions", "");
    if (measureID == "RatioTokens_Structured")        return StatisticalMeasureDefinition("RatioTokens_Structured", "Ratio of tokens in structured disfluencies", "");
    if (measureID == "RatioTokens_Complex")           return StatisticalMeasureDefinition("RatioTokens_Complex", "Ratio of tokens in complex editing disfluencies", "");
    if (measureID == "RatioTokens_AllStuttered")      return StatisticalMeasureDefinition("RatioTokens_AllStuttered", "Ratio of tokens in stuttered disfluencies (total)", "", "Total number of disfluencies (all types)");
    if (measureID == "RatioTokens_BREP")              return StatisticalMeasureDefinition("RatioTokens_BREP", "Ratio of tokens in stuttered repetitions of full words", "");
    if (measureID == "RatioTokens_BRED")              return StatisticalMeasureDefinition("RatioTokens_BRED", "Ratio of tokens in stuttered repetitions with fragments", "");
    if (measureID == "RatioTokens_BCOM")              return StatisticalMeasureDefinition("RatioTokens_BCOM", "Ratio of tokens in stuttered complex editing disfluencies", "");

    if (measureID == "RatioDisflTok_AllDisfluencies")   return StatisticalMeasureDefinition("RatioDisflTok_AllDisfluencies", "Ratio of disfluent tokens in disfluencies (total)", "", "Total number of disfluencies (all types)");
    if (measureID == "RatioDisflTok_REP")               return StatisticalMeasureDefinition("RatioDisflTok_REP", "Ratio of disfluent tokens in repetitions of full words", "");
    if (measureID == "RatioDisflTok_RED")               return StatisticalMeasureDefinition("RatioDisflTok_RED", "Ratio of disfluent tokens in repetitions with false starts", "");
    if (measureID == "RatioDisflTok_Repetitions")       return StatisticalMeasureDefinition("RatioDisflTok_Repetitions", "Ratio of disfluent tokens in repetitions", "");
    if (measureID == "RatioDisflTok_DEL")               return StatisticalMeasureDefinition("RatioDisflTok_DEL", "Ratio of disfluent tokens in deletions", "");
    if (measureID == "RatioDisflTok_INS")               return StatisticalMeasureDefinition("RatioDisflTok_INS", "Ratio of disfluent tokens in insertions", "");
    if (measureID == "RatioDisflTok_SUB")               return StatisticalMeasureDefinition("RatioDisflTok_SUB", "Ratio of disfluent tokens in substitutions", "");
    if (measureID == "RatioDisflTok_Structured")        return StatisticalMeasureDefinition("RatioDisflTok_Structured", "Ratio of disfluent tokens in structured disfluencies", "");
    if (measureID == "RatioDisflTok_Complex")           return StatisticalMeasureDefinition("RatioDisflTok_Complex", "Ratio of disfluent tokens in complex editing disfluencies", "");
    if (measureID == "RatioDisflTok_AllStuttered")      return StatisticalMeasureDefinition("RatioDisflTok_AllStuttered", "Ratio of disfluent tokens in stuttered disfluencies (total)", "", "Total number of disfluencies (all types)");
    if (measureID == "RatioDisflTok_BREP")              return StatisticalMeasureDefinition("RatioDisflTok_BREP", "Ratio of disfluent tokens in stuttered repetitions of full words", "");
    if (measureID == "RatioDisflTok_BRED")              return StatisticalMeasureDefinition("RatioDisflTok_BRED", "Ratio of disfluent tokens in stuttered repetitions with fragments", "");
    if (measureID == "RatioDisflTok_BCOM")              return StatisticalMeasureDefinition("RatioDisflTok_BCOM", "Ratio of disfluent tokens in stuttered complex editing disfluencies", "");

    if (measureID == "TokensBetweenDisfluencies_Median") return StatisticalMeasureDefinition("TokensBetweenDisfl_Median", "Number of tokens between disfluencies (median)", "");
    if (measureID == "TokensBetweenDisfluencies_Q1")     return StatisticalMeasureDefinition("TokensBetweenDisfl_Q1", "Number of tokens between disfluencies (Q1)", "");
    if (measureID == "TokensBetweenDisfluencies_Q3")     return StatisticalMeasureDefinition("TokensBetweenDisfl_Q3", "Number of tokens between disfluencies (Q3)", "");

    // Vectors per Speaker
    if (measureID == "TokensBetweenDisfluencies")   return StatisticalMeasureDefinition("TokensBetweenDisfluencies", "Number of tokens between disfluencies", "", "", QVariant::Double, true);
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

// ====================================================================================================================
// Analyser item: disfluencies
// ====================================================================================================================

struct AnalyserDisfluenciesItemData {
    AnalyserDisfluenciesItemData() : levelSyllables("syll"), levelTokens("tok_min"), levelDisfluencySequences("disfluencies")
    {}

    QString levelSyllables;
    QString levelTokens;
    QString levelDisfluencySequences;
    // Data from the analysis of one Communication
    QHash<QString, double> measuresCom;
    QMap<QString, QHash<QString, double> > measuresSpk;
    QHash<QString, QList<double> > vectorsCom;
    QMap<QString, QHash<QString, QList<double> > > vectorsSpk;
};

AnalyserDisfluenciesItem::AnalyserDisfluenciesItem(QObject *parent) :
    AnalyserItemBase(parent), d(new AnalyserDisfluenciesItemData())
{
}

AnalyserDisfluenciesItem::~AnalyserDisfluenciesItem()
{
    delete d;
}

QString AnalyserDisfluenciesItem::levelSyllables() const
{
    return d->levelSyllables;
}

void AnalyserDisfluenciesItem::setLevelSyllables(const QString &levelID)
{
    d->levelSyllables = levelID;
}

QString AnalyserDisfluenciesItem::levelTokens() const
{
    return d->levelTokens;
}

void AnalyserDisfluenciesItem::setLevelTokens(const QString &levelID)
{
    d->levelTokens = levelID;
}

QString AnalyserDisfluenciesItem::levelDisfluencySequences() const
{
    return d->levelDisfluencySequences;
}

void AnalyserDisfluenciesItem::setLevelDisfluencySequences(const QString &levelID)
{
    d->levelDisfluencySequences = levelID;
}

QStringList AnalyserDisfluenciesItem::speakerIDs() const
{
    return d->measuresSpk.keys();
}

double AnalyserDisfluenciesItem::measureCom(const QString &measureID) const
{
    return d->measuresCom.value(measureID, qQNaN());
}

double AnalyserDisfluenciesItem::measureSpk(const QString &speakerID, const QString &measureID) const
{
    if (!d->measuresSpk.contains(speakerID)) return qQNaN();
    return d->measuresSpk.value(speakerID).value(measureID, qQNaN());
}

QList<double> AnalyserDisfluenciesItem::vectorMeasureCom(const QString &measureID) const
{
    return d->vectorsCom.value(measureID);
}

QList<double> AnalyserDisfluenciesItem::vectorMeasureSpk(const QString &speakerID, const QString &measureID) const
{
    if (!d->vectorsSpk.contains(speakerID)) return QList<double>();
    return d->vectorsSpk.value(speakerID).value(measureID);
}

void AnalyserDisfluenciesItem::analyse(Praaline::Core::CorpusCommunication *com)
{
    if (!com) return;
    if (!com->repository()) return;

    d->measuresCom.clear();
    d->measuresSpk.clear();
    d->vectorsCom.clear();
    d->vectorsSpk.clear();

    foreach (QString annotationID, com->annotationIDs()) {
        SpeakerAnnotationTierGroupMap tiersAll =
                com->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList() << d->levelSyllables << d->levelTokens << d->levelDisfluencySequences);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            // IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllables);
            IntervalTier *tier_tokmin = tiers->getIntervalTierByName(d->levelTokens);
            SequenceTier *tier_disseq = tiers->getSequenceTierByName(d->levelDisfluencySequences);
            if (!tier_tokmin || !tier_disseq) continue;
            // Per speaker measures
            int Count_FIL(0), Count_FST(0), Count_LEN(0), Count_WDP(0), Count_MSP(0), Count_REP(0), Count_RED(0);
            int Count_DEL(0), Count_INS(0), Count_SUB(0), Count_COM(0);
            int Count_BLOC(0), Count_BLEN(0), Count_BREP(0), Count_BRED(0), Count_BCOM(0);
            int CountTokens_Single(0), CountTokens_REP(0), CountTokens_RED(0), CountTokens_DEL(0), CountTokens_INS(0), CountTokens_SUB(0), CountTokens_COM(0);
            int CountTokens_StutteredSingle(0), CountTokens_BREP(0), CountTokens_BRED(0), CountTokens_BCOM(0);
            int CountDisflTok_Single(0), CountDisflTok_REP(0), CountDisflTok_RED(0), CountDisflTok_DEL(0), CountDisflTok_INS(0), CountDisflTok_SUB(0), CountDisflTok_COM(0);
            int CountDisflTok_StutteredSingle(0), CountDisflTok_BREP(0), CountDisflTok_BRED(0), CountDisflTok_BCOM(0);
            int totalTokens(0);
            QList<double> tokensBetweenDisfluencies;
            // Analysis
            foreach (Sequence *dis, tier_disseq->sequences()) {
                int length = dis->indexTo() - dis->indexFrom() + 1;
                int indexReparans = dis->attribute("indexReparans").toInt();
                int lengthReparans = (indexReparans > 0) ? dis->indexTo() - indexReparans + 1 : 0;
                int dislen = length - lengthReparans;
                if      (dis->text() == "FIL")  { Count_FIL++; CountTokens_Single++; CountDisflTok_Single++; }
                else if (dis->text() == "FST")  { Count_FST++; CountTokens_Single++; CountDisflTok_Single++; }
                else if (dis->text() == "LEN")  { Count_LEN++; CountTokens_Single++; CountDisflTok_Single++; }
                else if (dis->text() == "WDP")  { Count_WDP++; CountTokens_Single++; CountDisflTok_Single++; }
                else if (dis->text() == "MSP")  { Count_MSP++; CountTokens_Single++; CountDisflTok_Single++; }
                else if (dis->text() == "REP")  { Count_REP++; CountTokens_REP += length; CountDisflTok_REP += dislen; }
                else if (dis->text() == "RED")  { Count_RED++; CountTokens_RED += length; CountDisflTok_RED += dislen; }
                else if (dis->text() == "DEL")  { Count_DEL++; CountTokens_DEL += length; CountDisflTok_DEL += dislen; }
                else if (dis->text() == "INS")  { Count_INS++; CountTokens_INS += length; CountDisflTok_INS += dislen; }
                else if (dis->text() == "SUB")  { Count_SUB++; CountTokens_SUB += length; CountDisflTok_SUB += dislen; }
                else if (dis->text() == "COM")  { Count_COM++; CountTokens_COM += length; CountDisflTok_COM += dislen; }
                else if (dis->text() == "BLOC") { Count_BLOC++; CountTokens_StutteredSingle++; CountDisflTok_StutteredSingle++; }
                else if (dis->text() == "BLEN") { Count_BLEN++; CountTokens_StutteredSingle++; CountDisflTok_StutteredSingle++; }
                else if (dis->text() == "BREP") { Count_BREP++; CountTokens_BREP += length; CountDisflTok_BREP += dislen; }
                else if (dis->text() == "BRED") { Count_BRED++; CountTokens_BRED += length; CountDisflTok_BRED += dislen; }
                else if (dis->text() == "BCOM") { Count_BCOM++; CountTokens_BCOM += length; CountDisflTok_BCOM += dislen; }
            }
            int runlength(0);
            for (int i = 0; i < tier_tokmin->count(); ++i) {
                QString dis = tier_tokmin->at(i)->attribute("disfluency").toString();
                dis = dis.replace("SIL:b", "SIL").replace("SIL:l", "SIL");
                if (dis == "SIL" || dis == "PARA") continue;
                totalTokens++;
                if (dis.isEmpty())
                    runlength++;
                else {
                    if (runlength > 0) {
                        tokensBetweenDisfluencies << runlength;
                        runlength = 0;
                    }
                }
            }
            // Totals
            int Count_SingleToken = Count_FIL + Count_FST + Count_LEN + Count_WDP + Count_MSP;
            int Count_Repetitions = Count_REP + Count_RED;
            int Count_Structured  = Count_DEL + Count_INS + Count_SUB;
            int Count_All = Count_SingleToken + Count_Repetitions + Count_Structured + Count_COM;
            int Count_AllStuttered  = Count_BLOC + Count_BLEN + Count_BREP + Count_BRED + Count_BCOM;
            int CountTokens_Repetitions = CountTokens_REP + CountTokens_RED;
            int CountTokens_Structured = CountTokens_DEL + CountTokens_INS + CountTokens_SUB;
            int CountTokens_All = CountTokens_Single + CountTokens_Repetitions + CountTokens_Structured + CountTokens_COM;
            int CountTokens_AllStuttered = CountTokens_StutteredSingle + CountTokens_BREP + CountTokens_BRED + CountTokens_BCOM;
            int CountDisflTok_Repetitions = CountDisflTok_REP + CountDisflTok_RED;
            int CountDisflTok_Structured = CountDisflTok_DEL + CountDisflTok_INS + CountDisflTok_SUB;
            int CountDisflTok_All = CountDisflTok_Single + CountDisflTok_Repetitions + CountDisflTok_Structured + CountDisflTok_COM;
            int CountDisflTok_AllStuttered = CountDisflTok_StutteredSingle + CountDisflTok_BREP + CountDisflTok_BRED + CountDisflTok_BCOM;

            // Add statistical measures for current speaker
            QHash<QString, double> measures;
            measures.insert("Count_FIL", Count_FIL);
            measures.insert("Count_FST", Count_FST);
            measures.insert("Count_LEN", Count_LEN);
            measures.insert("Count_WDP", Count_WDP);
            measures.insert("Count_MSP", Count_MSP);
            measures.insert("Count_SingleToken", Count_SingleToken);
            measures.insert("Count_REP", Count_REP);                    measures.insert("CountTokens_REP", CountTokens_REP);                    measures.insert("CountDisflTok_REP", CountDisflTok_REP);
            measures.insert("Count_RED", Count_RED);                    measures.insert("CountTokens_RED", CountTokens_RED);                    measures.insert("CountDisflTok_RED", CountDisflTok_RED);
            measures.insert("Count_Repetitions", Count_Repetitions);    measures.insert("CountTokens_Repetitions", CountTokens_Repetitions);    measures.insert("CountDisflTok_Repetitions", CountDisflTok_Repetitions);
            measures.insert("Count_DEL", Count_DEL);                    measures.insert("CountTokens_DEL", CountTokens_DEL);                    measures.insert("CountDisflTok_DEL", CountDisflTok_DEL);
            measures.insert("Count_INS", Count_INS);                    measures.insert("CountTokens_INS", CountTokens_INS);                    measures.insert("CountDisflTok_INS", CountDisflTok_INS);
            measures.insert("Count_SUB", Count_SUB);                    measures.insert("CountTokens_SUB", CountTokens_SUB);                    measures.insert("CountDisflTok_SUB", CountDisflTok_SUB);
            measures.insert("Count_Structured", Count_Structured);      measures.insert("CountTokens_Structured", CountTokens_Structured);      measures.insert("CountDisflTok_Structured", CountDisflTok_Structured);
            measures.insert("Count_Complex", Count_COM);                measures.insert("CountTokens_Complex", CountTokens_COM);                measures.insert("CountDisflTok_Complex", CountDisflTok_COM);
            measures.insert("Count_AllDisfluencies", Count_All);        measures.insert("CountTokens_AllDisfluencies", CountTokens_All);        measures.insert("CountDisflTok_AllDisfluencies", CountDisflTok_All);

            measures.insert("Count_BLOC", Count_BLOC);
            measures.insert("Count_BLEN", Count_BLEN);
            measures.insert("Count_BREP", Count_BREP);                  measures.insert("CountTokens_BREP", CountTokens_BREP);                  measures.insert("CountDisflTok_BREP", CountDisflTok_BREP);
            measures.insert("Count_BRED", Count_BRED);                  measures.insert("CountTokens_BRED", CountTokens_BRED);                  measures.insert("CountDisflTok_BRED", CountDisflTok_BRED);
            measures.insert("Count_BCOM", Count_BCOM);                  measures.insert("CountTokens_BCOM", CountTokens_BCOM);                  measures.insert("CountDisflTok_BCOM", CountDisflTok_BCOM);
            measures.insert("Count_AllStuttered", Count_AllStuttered);  measures.insert("CountTokens_AllStuttered", CountTokens_AllStuttered);  measures.insert("CountDisflTok_AllStuttered", CountDisflTok_AllStuttered);

            QStringList normalise;
            normalise << "_AllDisfluencies" << "_FIL" << "_FST" << "_LEN" << "_WDP" << "_MSP" << "_SingleToken"
                      << "_REP" << "_RED" << "_Repetitions" << "_DEL" << "_INS" << "_SUB" << "_Structured" << "_Complex"
                      << "_AllStuttered" << "_BLOC" << "_BLEN" << "_BREP" << "_BRED" << "_BCOM"
                      << "Tokens_AllDisfluencies" << "Tokens_REP" << "Tokens_RED" << "Tokens_Repetitions"
                      << "Tokens_DEL" << "Tokens_INS" << "Tokens_SUB" << "Tokens_Structured" << "Tokens_Complex"
                      << "Tokens_AllStuttered" << "Tokens_BREP" << "Tokens_BRED" << "Tokens_BCOM"
                      << "DisflTok_AllDisfluencies" << "DisflTok_REP" << "DisflTok_RED" << "DisflTok_Repetitions"
                      << "DisflTok_DEL" << "DisflTok_INS" << "DisflTok_SUB" << "DisflTok_Structured" << "DisflTok_Complex"
                      << "DisflTok_AllStuttered" << "DisflTok_BREP" << "DisflTok_BRED" << "DisflTok_BCOM";
            foreach (QString id, normalise) {
                double x = measures.value(QString("Count%1").arg(id), 0.0);
                measures.insert(QString("Ratio%1").arg(id), (x / ((double) totalTokens)));
            }

            StatisticalSummary summaryTokensBetweenDisfl;
            summaryTokensBetweenDisfl.calculate(tokensBetweenDisfluencies);
            measures.insert("TokensBetweenDisfluencies_Median", summaryTokensBetweenDisfl.median());
            measures.insert("TokensBetweenDisfluencies_Q1", summaryTokensBetweenDisfl.firstQuartile());
            measures.insert("TokensBetweenDisfluencies_Q3", summaryTokensBetweenDisfl.thirdQuartile());

            d->measuresSpk.insert(speakerID, measures);
            QHash<QString, QList<double> > vectors;
            vectors.insert("TokensBetweenDisfluencies", tokensBetweenDisfluencies);

            d->vectorsSpk.insert(speakerID, vectors);

        }
        qDeleteAll(tiersAll);
    }
}

