#include "math.h"
#include <QPointer>
#include <QDebug>
#include "pncore/annotation/annotationtiergroup.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/base/RealValueList.h"
#include "prosodicboundaries.h"

QStringList ProsodicBoundaries::POS_CLI = QStringList( { "PRP:det", "DET:def", "DET:dem", "DET:int", "DET:par", "DET:pos",
                                                         "PFX", "PRO:per:objd", "PRO:per:obji", "PRO:per:sjt", "PRO:ref", "PRP" } );
QStringList ProsodicBoundaries::POS_INT = QStringList( { "ADV:deg", "ADV:int", "ADV:neg", "CON:coo", "CON:sub", "DET:ind", "ITJ",
                                                         "NUM:crd:det", "PRO:dem", "PRO:ind", "PRO:int", "PRO:rel",
                                                         "VER:cond:aux", "VER:fut:aux", "VER:impe:aux", "VER:impf:aux", "VER:inf:aux",
                                                         "VER:ppas:aux", "VER:ppre:aux", "VER:pres:aux", "VER:pres:entatif",
                                                         "VER:pres:pred:aux",  "VER:simp:aux", "VER:subp:aux"} );
QStringList ProsodicBoundaries::POS_LEX = QStringList( { "ADJ", "ADV", "ADV:comp", "FRG", "INTROD",
                                                         "NOM:acr", "NOM:com", "NOM:nom", "NOM:pro",
                                                         "NUM:crd", "NUM:crd:nom", "NUM:ord:adj",
                                                         "PRO:per:ton", "PRO:pos",
                                                         "VER:cond", "VER:fut", "VER:impe", "VER:impf", "VER:inf", "VER:ppas",
                                                         "VER:ppre", "VER:pres", "VER:simp", "VER:subi", "VER:subp" } );


QPair<int, int> ProsodicBoundaries::windowNoPause(IntervalTier *tier_syll, int i, int windowLeft, int windowRight)
{
    QPair<int, int> ret;
    ret.first = i; ret.second = i;
    // Checks
    if (!tier_syll) return ret;
    if (i < 0 || i >= tier_syll->countItems()) return ret;
    if (tier_syll->interval(i)->isPauseSilent()) return ret;
    // Calculation
    ret.first = i - windowLeft;
    if (ret.first < 0) ret.first = 0;
    while (tier_syll->interval(ret.first)->isPauseSilent() && ret.first < i) ret.first++;
    ret.second = i + windowRight;
    if (ret.second >= tier_syll->countItems()) ret.second = tier_syll->countItems() - 1;
    while (tier_syll->interval(ret.second)->isPauseSilent() && ret.second > i) ret.second--;
    return ret;
}

bool ProsodicBoundaries::mean(double &mean, IntervalTier *tier_syll, QString attributeName, int i, int windowLeft, int windowRight,
                              bool checkStylized)
{
    if (!tier_syll) return 0.0;
    QPair<int, int> window = windowNoPause(tier_syll, i, windowLeft, windowRight);
    double sum = 0.0;
    int count = 0;
    for (int j = window.first; j <= window.second; j++) {
        Interval *syll = tier_syll->interval(j);
        if (checkStylized) {
            if (syll->attribute("f0_min").toInt() == 0) continue; // check if stylised
        }
        double x = syll->attribute(attributeName).toDouble();
        sum = sum + x;
        count++;
    }
    if (count == 0) return false;
    mean = sum / ((double)count);
    return true;
}

double ProsodicBoundaries::relative(IntervalTier *tier_syll, QString attributeName, int i, int windowLeft, int windowRight,
                                    bool checkStylized, bool logarithmic)
{
    // When it is impossible to calculate a relative value, return 1 for ratios or 0=log(1) for logarithmic attributes
    if (!tier_syll) return (logarithmic) ? 0.0 : 1.0;
    Interval *syll = tier_syll->interval(i);
    if (!syll) return (logarithmic) ? 0.0 : 1.0;
    // Check if attribute has to be stylised and if not, try to interpolate
    double value = syll->attribute(attributeName).toDouble();
    if (checkStylized && (syll->attribute("f0_min").toInt() == 0)) {
        if (!mean(value, tier_syll, attributeName, i, 1, 1, true))
            return (logarithmic) ? 0.0 : 1.0; // no luck
    }
    // Get mean in window
    double windowMean = 0.0;
    if (!mean(windowMean, tier_syll, attributeName, i, windowLeft, windowRight, checkStylized))
        return (logarithmic) ? 0.0 : 1.0; // no luck
    // Calculate relative value
    if (logarithmic) {
        return value - windowMean;
    }
    // else linear
    return value / windowMean;
}

bool ProsodicBoundaries::isLexical(Interval *token)
{
    // LEX:
    // ADJ, ADV, NUM, NOM, FRG, ITJ, PFX, PRO:pos, VER except...
    // Non-LEX:
    // CON:sub, DET, PRO except PRO:pos, PRP, VER:pres:aux
    // Exceptions:
    // "-t-il", "-t-elle"
    // *** A VOIR *** certains adverbes, prepositions longues, mots grammaticaux post-poses
    QString currentToken = token->text();
    QString currentPOS = token->attribute("pos_mwu").toString();
    if (currentPOS == "CON:sub") return false;
    if (currentPOS.left(3) == "DET") return false;
    if (currentPOS.left(3) == "PRO" && currentPOS != "PRO:pos") return false;
    if (currentPOS.left(3) == "PRP") return false;
    if (currentPOS.left(3) == "VER:pres:aux") return false;
    if (currentToken == "-t-il") return false;
    if (currentToken == "-t-elle") return false;
    return true;
}

QString ProsodicBoundaries::categorise_CLI_INT_LEX(Interval *token)
{
    if (!token) return "0";
    QString currentPOS = token->attribute("pos_mwu").toString();
    if (currentPOS == "0") return "FST"; // false starts
    if (POS_CLI.contains(currentPOS)) return "CLI";
    if (POS_INT.contains(currentPOS)) return "INT";
    if (POS_LEX.contains(currentPOS)) return "LEX";
    return "LEX";
}

QList<QString>
ProsodicBoundaries::analyseBoundaryListToStrings(Corpus *corpus, const QString &annotID,
                                                 QList<int> syllIndices, QStringList additionalAttributeIDs)
{
    QList<QString> results;
    if (!corpus) return results;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotID);
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        if (!tier_syll) continue;
        IntervalTier *tier_tokmwu = tiers->getIntervalTierByName("tok_mwu");
        IntervalTier *tier_sequence = tiers->getIntervalTierByName("sequence");
        IntervalTier *tier_rection = tiers->getIntervalTierByName("rection");
        RealValueList pause_durations;

        // Prepare syllables (calculcate log duration and a list of pause durations)
        // Distinguish between intra-speaker and inter-speaker pauses
        IntervalTier *timeline = corpus->datastoreAnnotations()->getSpeakerTimeline(annotID, "syll");
        foreach (Interval *syll, tier_syll->intervals()) {
            syll->setAttribute("duration_log", log(syll->attribute("duration").toDouble()));
            if (syll->text() == "_") {
                int pause_index = timeline->intervalIndexAtTime(syll->tCenter());
                if (pause_index == 0 || pause_index == timeline->countItems() - 1) continue;
                QString speakerBefore = timeline->interval(pause_index - 1)->text();
                QString speakerAfter = timeline->interval(pause_index + 1)->text();
                if (speakerBefore == speakerAfter && speakerBefore == speakerID)
                    pause_durations << syll->attribute("duration_log").toDouble();
            }
        }

        // Analyse selected syllables
        foreach (int isyll, syllIndices) {
            if ((isyll < 0) || (isyll >= tier_syll->countItems())) continue;
            Interval *syll = tier_syll->interval(isyll);

            // Expert annotation
            // --------------------------------------------------------------------------------------------------------
            QString expertBoundary = syll->attribute("boundary").toString();
            QString expertBoundaryType = "0";
            if      (expertBoundary.contains("///"))   expertBoundaryType = "///";
            else if (expertBoundary.contains("//"))    expertBoundaryType = "//";
            else if (expertBoundary.contains("hesi"))  expertBoundaryType = "hesi";
            else if (expertBoundary.contains("#"))     expertBoundaryType = "#";
            QString expertContour = syll->attribute("contour").toString();

            // Prosodic features
            // --------------------------------------------------------------------------------------------------------
            double durNextPause = 0.0, logdurNextPause = 0.0, logdurNextPauseZ = 0.0;
            if (isyll < tier_syll->countItems() - 1) {
                if (tier_syll->interval(isyll + 1)->text() == "_") {
                    int pause_index = timeline->intervalIndexAtTime(tier_syll->interval(isyll + 1)->tCenter());
                    if (pause_index != 0 && pause_index != timeline->countItems() - 1) {
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
            durSyllRel20 = relative(tier_syll, "duration", isyll, 2, 0, false, false);
            durSyllRel30 = relative(tier_syll, "duration", isyll, 3, 0, false, false);
            durSyllRel40 = relative(tier_syll, "duration", isyll, 4, 0, false, false);
            durSyllRel50 = relative(tier_syll, "duration", isyll, 5, 0, false, false);

            double logdurSyllRel20 = 0.0, logdurSyllRel30 = 0.0, logdurSyllRel40 = 0.0, logdurSyllRel50 = 0.0;
            logdurSyllRel20 = relative(tier_syll, "duration_log", isyll, 2, 0, false, false);
            logdurSyllRel30 = relative(tier_syll, "duration_log", isyll, 3, 0, false, false);
            logdurSyllRel40 = relative(tier_syll, "duration_log", isyll, 4, 0, false, false);
            logdurSyllRel50 = relative(tier_syll, "duration_log", isyll, 5, 0, false, false);

            double f0meanSyllRel20 = 0.0, f0meanSyllRel30 = 0.0, f0meanSyllRel40 = 0.0, f0meanSyllRel50 = 0.0;
            f0meanSyllRel20 = relative(tier_syll, "f0_mean", isyll, 2, 0, true, true);
            f0meanSyllRel30 = relative(tier_syll, "f0_mean", isyll, 3, 0, true, true);
            f0meanSyllRel40 = relative(tier_syll, "f0_mean", isyll, 4, 0, true, true);
            f0meanSyllRel50 = relative(tier_syll, "f0_mean", isyll, 5, 0, true, true);

            double intrasyllab_up = tier_syll->interval(isyll)->attribute("intrasyllabup").toDouble();
            double intrasyllab_down = tier_syll->interval(isyll)->attribute("intrasyllabdown").toDouble();
            double trajectory = tier_syll->interval(isyll)->attribute("trajectory").toDouble();

            // Syntax
            // --------------------------------------------------------------------------------------------------------
            Interval *tok_mwu = (tier_tokmwu) ? tier_tokmwu->intervalAtTime(syll->tCenter()) : 0;
            Interval *sequence = (tier_sequence) ? tier_sequence->intervalAtTime(syll->tCenter()) : 0;
            Interval *rection = (tier_rection) ? tier_rection->intervalAtTime(syll->tCenter()) : 0;

            RealTime t10ns = RealTime::fromNanoseconds(10);
            Interval *tok_mwu_next = (tier_tokmwu) ? tier_tokmwu->intervalAtTime(syll->tMax() + t10ns) : 0;
            Interval *sequence_next = (tier_sequence) ? tier_sequence->intervalAtTime(syll->tMax() + t10ns) : 0;
            Interval *rection_next = (tier_rection) ? tier_rection->intervalAtTime(syll->tMax() + t10ns) : 0;

            QString pos_mwu = (tok_mwu) ? tok_mwu->attribute("pos_mwu").toString() : "unk";

            QString tok_mwu_text = (tok_mwu) ? tok_mwu->text() : "";
            if (tok_mwu_next) tok_mwu_text = tok_mwu_text.append("|").append(tok_mwu_next->text());
            QString sequence_text = (sequence) ? sequence->text() : "";
            if (sequence_next) sequence_text = sequence_text.append("|").append(sequence_next->text());
            QString rection_text = (rection) ? rection->text() : "";
            if (rection_next) rection_text = rection_text.append("|").append(rection_next->text());

            QString syntacticBoundaryType = "0";
            if ((tok_mwu) && (tok_mwu != tok_mwu_next)) syntacticBoundaryType = "MWU";
            if ((sequence) && (sequence != sequence_next)) syntacticBoundaryType = "SEQ";
            if ((rection) && (rection != rection_next)) {
                syntacticBoundaryType = "REC";
                if (rection_text.contains("md|")) syntacticBoundaryType = "MD";
            }

            // --------------------------------------------------------------------------------------------------------
            // Write results to a string list
            // --------------------------------------------------------------------------------------------------------
            QString resultLine;
            QString sep = "\t", decimal = ",";
            resultLine.append(annotID).append(sep).append(annotID.right(1)).append(sep).append(speakerID);
            resultLine.append(sep).append(QString::number(isyll)).append(sep);
            resultLine.append(QString::number(tier_syll->interval(isyll)->tMin().toDouble()).replace(".", decimal)).append(sep);
            resultLine.append(QString(syll->text()).remove("\t").remove("\n")).append(sep);
            // Expert
            resultLine.append(expertBoundaryType).append(sep).append(expertContour).append(sep).append(expertBoundary).append(expertContour).append(sep);
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
            resultLine.append(tok_mwu_text).append(sep).append(sequence_text).append(sep).append(rection_text).append(sep);
            resultLine.append(syntacticBoundaryType).append(sep);
            resultLine.append(pos_mwu).append(sep).append(pos_mwu.left(3)).append(sep).append(categorise_CLI_INT_LEX(tok_mwu));
            // Other requested attributes
            QString rest;
            foreach (QString attributeID, additionalAttributeIDs) {
                rest.append(syll->attribute(attributeID).toString().replace(".", decimal)).append(sep);
            }
            if (rest.endsWith(sep)) rest.chop(sep.length());
            if (!rest.isEmpty()) resultLine.append(sep).append(rest);
            results << resultLine;
        }
    }
    return results;
}


void ProsodicBoundaries::analyseBoundaryList(QTextStream &out, Corpus *corpus, const QString &annotID,
                                             QList<int> syllIndices, QStringList additionalAttributeIDs)
{
    QList<QString> results = analyseBoundaryListToStrings(corpus, annotID, syllIndices, additionalAttributeIDs);
    foreach (QString line, results) {
        out << line << "\n";
    }
}


QList<QString> ProsodicBoundaries::analyseCorpusSampleToStrings(Corpus *corpus, const QString &annotID)
{
    QList<QString> results;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotID);
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);

        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        if (!tier_syll) continue;
        IntervalTier *tier_group = tiers->getIntervalTierByName("sequence");
        if (!tier_group) continue;

        QList<int> syllables;
        QStringList additionalAttributeIDs;
        foreach (Interval *group, tier_group->intervals()) {
            if (group->text() == "_") continue;
            RealTime t = group->tMax() - RealTime::fromNanoseconds(10);
            int i_lastsyll = tier_syll->intervalIndexAtTime(t);
            if (i_lastsyll == -1) continue;
            syllables << i_lastsyll;
        }

        results << analyseBoundaryListToStrings(corpus, annotID, syllables, additionalAttributeIDs);
    }
    return results;
}

void ProsodicBoundaries::analyseCorpusSample(QTextStream &out, Corpus *corpus, const QString &annotID)
{
    QList<QString> results = analyseCorpusSampleToStrings(corpus, annotID);
    foreach (QString line, results) {
        out << line << "\n";
    }
}




