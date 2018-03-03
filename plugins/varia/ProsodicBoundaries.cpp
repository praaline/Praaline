#include "math.h"
#include <QPointer>
#include <QDebug>
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/base/RealValueList.h"
#include "pncore/statistics/Measures.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "ProsodicBoundaries.h"

struct ProsodicBoundariesData {
    QString attributePOS;
    QStringList POS_CLI;
    QStringList POS_INT;
    QStringList POS_LEX;
    QStringList additionalAttributes;

    ProsodicBoundariesData() {
        POS_CLI = QStringList( { "PRP:det", "DET:def", "DET:dem", "DET:int", "DET:par", "DET:pos",
                                 "PFX", "PRO:per:objd", "PRO:per:obji", "PRO:per:sjt", "PRO:ref", "PRP" } );
        POS_INT = QStringList( { "ADV:deg", "ADV:int", "ADV:neg", "CON:coo", "CON:sub", "DET:ind", "ITJ",
                                 "NUM:crd:det", "PRO:dem", "PRO:ind", "PRO:int", "PRO:rel",
                                 "VER:cond:aux", "VER:fut:aux", "VER:impe:aux", "VER:impf:aux", "VER:inf:aux",
                                 "VER:ppas:aux", "VER:ppre:aux", "VER:pres:aux", "VER:pres:entatif",
                                 "VER:pres:pred:aux",  "VER:simp:aux", "VER:subp:aux"} );
        POS_LEX = QStringList( { "ADJ", "ADV", "ADV:comp", "FRG", "INTROD",
                                 "NOM:acr", "NOM:com", "NOM:nom", "NOM:pro",
                                 "NUM:crd", "NUM:crd:nom", "NUM:ord:adj",
                                 "PRO:per:ton", "PRO:pos",
                                 "VER:cond", "VER:fut", "VER:impe", "VER:impf", "VER:inf", "VER:ppas",
                                 "VER:ppre", "VER:pres", "VER:simp", "VER:subi", "VER:subp" } );
        attributePOS = "pos_mwu";
    }
};

ProsodicBoundaries::ProsodicBoundaries() :
    d(new ProsodicBoundariesData())
{
}

ProsodicBoundaries::~ProsodicBoundaries()
{
    delete d;
}

QStringList ProsodicBoundaries::additionalAttributeIDs() const
{
    return d->additionalAttributes;
}

void ProsodicBoundaries::setAdditionalAttributeIDs(const QStringList &attributeIDs)
{
    d->additionalAttributes = attributeIDs;
}

bool ProsodicBoundaries::isLexical(Interval *token) const
{
    // LEX:
    // ADJ, ADV, NUM, NOM, FRG, ITJ, PFX, PRO:pos, VER except...
    // Non-LEX:
    // CON:sub, DET, PRO except PRO:pos, PRP, VER:pres:aux
    // Exceptions:
    // "-t-il", "-t-elle"
    // *** A VOIR *** certains adverbes, prepositions longues, mots grammaticaux post-poses
    QString currentToken = token->text();
    QString currentPOS = token->attribute(d->attributePOS).toString();
    if (currentPOS == "CON:sub") return false;
    if (currentPOS.left(3) == "DET") return false;
    if (currentPOS.left(3) == "PRO" && currentPOS != "PRO:pos") return false;
    if (currentPOS.left(3) == "PRP") return false;
    if (currentPOS.left(3) == "VER:pres:aux") return false;
    if (currentToken == "-t-il") return false;
    if (currentToken == "-t-elle") return false;
    return true;
}

QString ProsodicBoundaries::categorise_CLI_INT_LEX(Interval *token) const
{
    if (!token) return "0";
    if (token->isPauseSilent()) return "_";
    QString currentPOS = token->attribute(d->attributePOS).toString();
    if (currentPOS == "0") return "FST"; // false starts
    if (d->POS_CLI.contains(currentPOS)) return "CLI";
    if (d->POS_INT.contains(currentPOS)) return "INT";
    if (d->POS_LEX.contains(currentPOS)) return "LEX";
    return "LEX";
}

QStringList
ProsodicBoundaries::analyseBoundaryListToStrings(Corpus *corpus, const QString &annotationID, QList<int> syllIndices)
{
    QStringList results;
    if (!corpus) return results;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->repository()->annotations()->getTiersAllSpeakers(annotationID);
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        if (!tier_syll) continue;
        IntervalTier *tier_tokmwu = tiers->getIntervalTierByName("tok_mwu");
        RealValueList pause_durations;

        // Prepare syllables (calculcate log duration and a list of pause durations)
        // Distinguish between intra-speaker and inter-speaker pauses
        // Note: for getSpeakerTimeline, in the experiment corpus communicationID = annotationID
        IntervalTier *timeline = corpus->repository()->annotations()->getSpeakerTimeline(annotationID, annotationID, "syll");
        foreach (Interval *syll, tier_syll->intervals()) {
            syll->setAttribute("duration_log", log(syll->attribute("duration").toDouble()));
            if (syll->text() == "_") {
                int pause_index = timeline->intervalIndexAtTime(syll->tCenter());
                if (pause_index == 0 || pause_index == timeline->count() - 1) continue;
                QString speakerBefore = timeline->interval(pause_index - 1)->text();
                QString speakerAfter = timeline->interval(pause_index + 1)->text();
                if (speakerBefore == speakerAfter && speakerBefore == speakerID)
                    pause_durations << syll->attribute("duration_log").toDouble();
            }
        }

        // Analyse selected syllables
        foreach (int isyll, syllIndices) {
            if ((isyll < 0) || (isyll >= tier_syll->count())) continue;
            Interval *syll = tier_syll->interval(isyll);

            // Prosodic features
            // --------------------------------------------------------------------------------------------------------
            double durNextPause = 0.0, logdurNextPause = 0.0, logdurNextPauseZ = 0.0;
            if (isyll < tier_syll->count() - 1) {
                if (tier_syll->interval(isyll + 1)->text() == "_") {
                    int pause_index = timeline->intervalIndexAtTime(tier_syll->interval(isyll + 1)->tCenter());
                    if (pause_index != 0 && pause_index != timeline->count() - 1) {
                        QString speakerBefore = timeline->interval(pause_index - 1)->text();
                        QString speakerAfter = timeline->interval(pause_index + 1)->text();
                        if (speakerBefore == speakerAfter && speakerBefore == speakerID) {
                            durNextPause = tier_syll->interval(isyll + 1)->duration().toDouble();
                            logdurNextPause = tier_syll->interval(isyll + 1)->attribute("duration_log").toDouble();
                            logdurNextPauseZ = pause_durations.zscore_calculate(logdurNextPause);
                        }
                    }
                }
            }
            double durSyllRel20 = 0.0, durSyllRel30 = 0.0, durSyllRel40 = 0.0, durSyllRel50 = 0.0;
            durSyllRel20 = Measures::relative(tier_syll, "duration", isyll, 2, 0, true, "", false);
            durSyllRel30 = Measures::relative(tier_syll, "duration", isyll, 3, 0, true, "", false);
            durSyllRel40 = Measures::relative(tier_syll, "duration", isyll, 4, 0, true, "", false);
            durSyllRel50 = Measures::relative(tier_syll, "duration", isyll, 5, 0, true, "", false);

            double logdurSyllRel20 = 0.0, logdurSyllRel30 = 0.0, logdurSyllRel40 = 0.0, logdurSyllRel50 = 0.0;
            logdurSyllRel20 = Measures::relative(tier_syll, "duration_log", isyll, 2, 0, true, "", false);
            logdurSyllRel30 = Measures::relative(tier_syll, "duration_log", isyll, 3, 0, true, "", false);
            logdurSyllRel40 = Measures::relative(tier_syll, "duration_log", isyll, 4, 0, true, "", false);
            logdurSyllRel50 = Measures::relative(tier_syll, "duration_log", isyll, 5, 0, true, "", false);

            double f0meanSyllRel20 = 0.0, f0meanSyllRel30 = 0.0, f0meanSyllRel40 = 0.0, f0meanSyllRel50 = 0.0;
            f0meanSyllRel20 = Measures::relative(tier_syll, "f0_mean", isyll, 2, 0, true, "f0_min", true);
            f0meanSyllRel30 = Measures::relative(tier_syll, "f0_mean", isyll, 3, 0, true, "f0_min", true);
            f0meanSyllRel40 = Measures::relative(tier_syll, "f0_mean", isyll, 4, 0, true, "f0_min", true);
            f0meanSyllRel50 = Measures::relative(tier_syll, "f0_mean", isyll, 5, 0, true, "f0_min", true);

            double intrasyllab_up = tier_syll->interval(isyll)->attribute("intrasyllabup").toDouble();
            double intrasyllab_down = tier_syll->interval(isyll)->attribute("intrasyllabdown").toDouble();
            double trajectory = tier_syll->interval(isyll)->attribute("trajectory").toDouble();

            // Syntax
            // --------------------------------------------------------------------------------------------------------
            Interval *tok_mwu = (tier_tokmwu) ? tier_tokmwu->intervalAtTime(syll->tCenter()) : 0;
            QString pos_mwu = (tok_mwu) ? tok_mwu->attribute(d->attributePOS).toString() : "unk";

            // Context
            int tokenIndex = tier_tokmwu->intervalIndexAtTime(syll->tCenter());
            QString context = (tokenIndex > 0) ? tier_tokmwu->getContextSymmetricFormated(tokenIndex, 10) : "";

            // --------------------------------------------------------------------------------------------------------
            // Write results to a string list
            // --------------------------------------------------------------------------------------------------------
            QString resultLine;
            QString sep = "\t", decimal = ",";
            resultLine.append(annotationID).append(sep).append(annotationID.right(1)).append(sep).append(speakerID);
            resultLine.append(sep).append(QString::number(isyll)).append(sep);
            resultLine.append(QString::number(tier_syll->interval(isyll)->tMin().toDouble()).replace(".", decimal)).append(sep);
            resultLine.append(QString(syll->text()).remove("\t").remove("\n")).append(sep);
            // Expert
            resultLine.append(syll->attribute("boundaryType").toString()).append(sep);
            resultLine.append(syll->attribute("contour").toString()).append(sep);
            resultLine.append(syll->attribute("boundary").toString()).append(syll->attribute("contour").toString()).append(sep);
            // Prosody
            resultLine.append(QString::number(durNextPause).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(logdurNextPause).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(logdurNextPauseZ).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(durSyllRel20).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(durSyllRel30).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(durSyllRel40).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(durSyllRel50).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(logdurSyllRel20).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(logdurSyllRel30).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(logdurSyllRel40).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(logdurSyllRel50).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(f0meanSyllRel20).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(f0meanSyllRel30).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(f0meanSyllRel40).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(f0meanSyllRel50).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(intrasyllab_up).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(intrasyllab_down).replace(".", decimal)).append(sep);
            resultLine.append(QString::number(trajectory).replace(".", decimal)).append(sep);
            // Syntax
            resultLine.append(pos_mwu).append(sep).append(pos_mwu.left(3)).append(sep);
            resultLine.append(categorise_CLI_INT_LEX(tok_mwu)).append(sep);
            resultLine.append(context);
            // Other requested attributes
            QString rest;
            foreach (QString attributeID, d->additionalAttributes) {
                rest.append(syll->attribute(attributeID).toString().replace(".", decimal)).append(sep);
            }
            if (rest.endsWith(sep)) rest.chop(sep.length());
            if (!rest.isEmpty()) resultLine.append(sep).append(rest);
            results << resultLine;
        }
    }
    return results;
}

void ProsodicBoundaries::analyseBoundaryListToStream(QTextStream &out, Corpus *corpus, const QString &annotationID,
                                                     QList<int> syllIndices)
{
    QStringList results = analyseBoundaryListToStrings(corpus, annotationID, syllIndices);
    foreach (QString line, results) {
        out << line << "\n";
    }
}

QStringList ProsodicBoundaries::analyseAnnotationToStrings(Corpus *corpus, const QString &annotationID)
{
    QStringList results;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->repository()->annotations()->getTiersAllSpeakers(annotationID);
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        // Create a list of all syllables excluding pauses
        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        if (!tier_syll) continue;
        QList<int> syllables;
        for (int i = 0; i < tier_syll->count(); ++i) {
            if (tier_syll->at(i)->isPauseSilent()) continue;
            syllables << i;
        }
        results << analyseBoundaryListToStrings(corpus, annotationID, syllables);
    }
    qDeleteAll(tiersAll);
    return results;
}

void ProsodicBoundaries::analyseAnnotationToStream(QTextStream &out, Corpus *corpus, const QString &annotationID)
{
    QStringList results = analyseAnnotationToStrings(corpus, annotationID);
    foreach (QString line, results) {
        out << line << "\n";
    }
}

QStringList ProsodicBoundaries::analyseLastSyllOfSequenceToStrings(Corpus *corpus, const QString &annotationID,
                                                                   const QString &sequenceLevelID)
{
    QStringList results;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->repository()->annotations()->getTiersAllSpeakers(annotationID);
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        // Create a list of all syllables at the end of sequences, excluding pauses
        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        if (!tier_syll) continue;
        IntervalTier *tier_group = tiers->getIntervalTierByName(sequenceLevelID);
        if (!tier_group) continue;
        QList<int> syllables;
        foreach (Interval *group, tier_group->intervals()) {
            if (group->text() == "_") continue;
            RealTime t = group->tMax() - RealTime::fromNanoseconds(10);
            int i_lastsyll = tier_syll->intervalIndexAtTime(t);
            if (i_lastsyll == -1) continue;
            syllables << i_lastsyll;
        }
        results << analyseBoundaryListToStrings(corpus, annotationID, syllables);
    }
    qDeleteAll(tiersAll);
    return results;
}

void ProsodicBoundaries::analyseLastSyllOfSequenceToStream(QTextStream &out, Corpus *corpus, const QString &annotationID,
                                                           const QString &sequenceLevelID)
{
    QStringList results = analyseLastSyllOfSequenceToStrings(corpus, annotationID, sequenceLevelID);
    foreach (QString line, results) {
        out << line << "\n";
    }
}

