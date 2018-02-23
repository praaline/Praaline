#include <QDebug>
#include <QPointer>
#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QFile>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/statistics/Measures.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "AggregateProsody.h"

AggregateProsody::AggregateProsody()
{

}

struct AggregateToken {
    AggregateToken(const QString &token) : token(token) {}
    QString token;
    QList<double> durations;
    QList<double> meanPitches;
};

QString AggregateProsody::markTargetSyllables(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    ret = com->ID();
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return "Error";
    QString annotationID = rec->ID();
    QString speakerID = com->property("SubjectID").toString();
    // Get tiers
    IntervalTier *tier_tokens = qobject_cast<IntervalTier *>
            (com->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
    if (!tier_tokens) return ret + " No tier tokens";
    IntervalTier *tier_syll = qobject_cast<IntervalTier *>
            (com->repository()->annotations()->getTier(annotationID, speakerID, "syll"));
    if (!tier_syll) return ret = " No tier syll";
    // Find target token and corresponding syllables
    int targetTokenIndex(-1);
    for (int i = 0; i < tier_tokens->count(); ++i) {
        if (tier_tokens->at(i)->attribute("target").toString() == "*") {
            targetTokenIndex = i;
            break;
        }
    }
    if (targetTokenIndex < 0) { return ret + " Target token not found"; }
    QPair<int, int> targetSyllIndices = tier_syll->getIntervalIndexesOverlappingWith(tier_tokens->at(targetTokenIndex));
    if ((targetSyllIndices.first < 0) || (targetSyllIndices.second < 0)) {
        return ret + " Syllables not found";
    }
    // Select context
    int contextLeft(5), contextRight(5);
    // Left context
    int syllIndex(0); int contextSyllCount(0);
    syllIndex = targetSyllIndices.first - 1;
    while ((syllIndex >= 0) && (contextSyllCount < contextLeft)) {
        Interval *syll = tier_syll->at(syllIndex);
        if (!syll->isPauseSilent()) {
            syll->setAttribute("target", QString("L%1").arg(-contextSyllCount-1));
            contextSyllCount++;
        }
        syllIndex--;
    }
    // Target
    contextSyllCount = 0;
    for (syllIndex = targetSyllIndices.first; syllIndex <= targetSyllIndices.second; ++syllIndex) {
        if ((syllIndex < 0) || (syllIndex >= tier_syll->count())) break;
        Interval * syll = tier_syll->at(syllIndex);
        syll->setAttribute("target", QString("T%1").arg(contextSyllCount + 1));
        contextSyllCount++;
    }
    // Right context
    syllIndex = targetSyllIndices.second + 1; contextSyllCount = 0;
    while ((syllIndex < tier_syll->count()) && (contextSyllCount < contextRight)) {
        Interval *syll = tier_syll->at(syllIndex);
        if (!syll->isPauseSilent()) {
            syll->setAttribute("target", QString("R%1").arg(contextSyllCount + 1));
            contextSyllCount++;
        }
        syllIndex++;
    }
    // Save syllable tier
    com->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
    return ret;
}

QString AggregateProsody::averageOnTokens(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    QMap<QString, QList<AggregateToken> > utterances;
    if (!com) return "Error";
    QPointer<Corpus> corpus = com->corpus();
    if (!corpus) return "Error";
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        QPointer<CorpusRecording> rec = com->recordings().first();
        if (!rec) continue;
        QString annotationID = rec->ID();
        QString speakerID = com->property("SubjectID").toString();
        QString stimulusID = com->property("StimulusID").toString();
        IntervalTier *tier_tokens = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
        if (!tier_tokens) return "No tier tokens";
        IntervalTier *tier_syll = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "syll"));
        if (!tier_syll) return "No tier syll";
        // If this is the first time, construct the prototypical utterance
        if (!utterances.contains(stimulusID)) {
            QList<AggregateToken> prototokens;
            foreach (Interval *token, tier_tokens->intervals()) {
                if (token->isPauseSilent()) continue;
                prototokens << AggregateToken(token->text());
            }
            utterances.insert(stimulusID, prototokens);
        }
        // Interpolate syllable pitch values
        for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
            Interval *syll = tier_syll->interval(isyll);
            // Smoothing for non-stylised syllables
            if (!syll->isPauseSilent() && syll->attribute("f0_min").toInt() == 0) {
                syll->setAttribute("f0_min", Measures::mean(tier_syll, "f0_min", isyll, 4, 4, true, "f0_min"));
                syll->setAttribute("f0_max", Measures::mean(tier_syll, "f0_max", isyll, 4, 4, true, "f0_min"));
                syll->setAttribute("f0_mean", Measures::mean(tier_syll, "f0_mean", isyll, 4, 4, true, "f0_min"));
                syll->setAttribute("int_peak", Measures::mean(tier_syll, "int_peak", isyll, 4, 4, true, "f0_min"));
            }
        }
        // Process
        int i = 0;
        foreach (Interval *token, tier_tokens->intervals()) {
            if (token->isPauseSilent()) continue;
            QList<double> f0_means;
            QPair<int, int> syllIndices = tier_syll->getIntervalIndexesContainedIn(token);
            // smoothing
            while (syllIndices.second - syllIndices.first < 5) {
                if (syllIndices.first > 0) syllIndices.first--;
                if (syllIndices.second < tier_syll->count() - 1) syllIndices.second++;
            }
            for (int i = syllIndices.first; i <= syllIndices.second; ++i) {
                Interval *syll = tier_syll->interval(i);
                if (!syll) continue;
                if (syll->attribute("nucl_t1").toDouble() > 0 && syll->attribute("nucl_t2").toDouble() > 0) {
                    f0_means << syll->attribute("f0_mean").toDouble();
                }
            }
            StatisticalSummary summary_f0_means(f0_means);
            utterances[stimulusID][i].durations << token->duration().toDouble();
            utterances[stimulusID][i].meanPitches << summary_f0_means.mean();
            ++i;
        }
        delete tier_tokens;
        delete tier_syll;
    }
    // Output
    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/03_prosodie-relations-de-discours/Production Analyses";
    QFile file(path + "/averageprosody.txt");
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ) return "Error reading transcriptions file";
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "StimulusID\tDiscourseMarker\tDiscourseRelation\tToken\tTime\tDurationMean\tDurationStdev\tPitchMean\tPitchStdev\n";
    foreach (QString stimulusID, utterances.keys()) {
        QList<AggregateToken> tokens = utterances[stimulusID];
        double time(0.0);
        foreach (AggregateToken t, tokens) {
            StatisticalSummary summary_duration(t.durations);
            StatisticalSummary summary_pitch(t.meanPitches);
            out << stimulusID << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            out << t.token << "\t" << time << "\t";
            out << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            out << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
            time = time + summary_duration.mean();
        }
    }
    file.close();
    return ret;
}

struct AggregateSyllable {
    AggregateSyllable(const QString &syllable) : syllable(syllable) {}
    QString syllable;
    QList<double> durations;
    QList<double> f0_means;
};

QString AggregateProsody::averageContours(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    QMap<QString, QList<AggregateSyllable> > contoursLeft;
    QMap<QString, QList<AggregateSyllable> > targets;
    QMap<QString, QList<AggregateSyllable> > contoursRight;
    if (!com) return "Error";
    QPointer<Corpus> corpus = com->corpus();
    if (!corpus) return "Error";
    // Select contextLength
    int contextLeft(3), contextRight(3);
    // Per stimulus measures
    // QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/03_prosodie-relations-de-discours/Production Analyses";
    // QString path = "/Users/george/Documents";
    QString path = "/home/george";

    QFile filePerStim(path + "/prosodic_measures_per_stimulus.txt");
    if ( !filePerStim.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return "Error writing output file";
    QTextStream outPerStim(&filePerStim);
    outPerStim.setCodec("UTF-8");
    outPerStim << "CommunicationID\tSpeakerID\tStimulusID\tUtteranceID\tDiscourseMarker\tDiscourseRelation\t";
    outPerStim << "T1_duration\tT2_duration\tT1_f0_mean\tT2_f0_mean\tT1_intersyllab\tT2_intersyllab\tR1_intersyllab\t";
    outPerStim << "L3_durRel30\tL2_durRel30\tL1_durRel30\tT1_durRel30\tT2_durRel30\tR1_durRel30\tR2_durRel30\tR3_durRel30\t";
    outPerStim << "L1_prom\tT1_prom\tT2_prom\tR1_prom\tL1_boundary\tT1_boundary\tT2_boundary\tR1_boundary\t";
    outPerStim << "L3_tone\tL3_tonemvt\tL2_tone\tL2_tonemvt\tL1_tone\tL1_tonemvt\t";
    outPerStim << "T1_tone\tT1_tonemvt\tT2_tone\tT2_tonemvt\t";
    outPerStim << "R1_tone\tR1_tonemvt\tR2_tone\tR2_tonemvt\tR3_tone\tR3_tonemvt\t";
    outPerStim << "SpeechRateS1\tSpeechRateS2\tPauseBeforeMD\tPauseAfterMD\n";

    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        QPointer<CorpusRecording> rec = com->recordings().first();
        if (!rec) continue;
        QString annotationID = rec->ID();
        QString speakerID = com->property("SubjectID").toString();
        QString stimulusID = com->property("StimulusID").toString();
        // Get tiers
        IntervalTier *tier_tokens = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "tok_min"));
        if (!tier_tokens) return "No tier tokens";
        IntervalTier *tier_syll = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "syll"));
        if (!tier_syll) return "No tier syll";
        IntervalTier *tier_sequence = qobject_cast<IntervalTier *>
                (com->repository()->annotations()->getTier(annotationID, speakerID, "sequence"));
        if (!tier_sequence) return "No tier sequence";
        // If this is the first time, insert contour
        if (!contoursLeft.contains(stimulusID)) {
            QList<AggregateSyllable> left;
            for (int i = 1; i <= contextLeft; ++i) left.append(AggregateSyllable(""));
            contoursLeft.insert(stimulusID, left);
            QList<AggregateSyllable> target;
            for (int i = 1; i <= 2; ++i) target.append(AggregateSyllable(""));
            targets.insert(stimulusID, target);
            QList<AggregateSyllable> right;
            for (int i = 1; i <= contextRight; ++i) right.append(AggregateSyllable(""));
            contoursRight.insert(stimulusID, right);
        }
        // Interpolate syllable pitch values
        for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
            Interval *syll = tier_syll->interval(isyll);
            // Smoothing for non-stylised syllables
            if (!syll->isPauseSilent() && syll->attribute("f0_min").toInt() == 0) {
                syll->setAttribute("f0_min", Measures::mean(tier_syll, "f0_min", isyll, 3, 3, true, "f0_min"));
                syll->setAttribute("f0_max", Measures::mean(tier_syll, "f0_max", isyll, 3, 3, true, "f0_min"));
                syll->setAttribute("f0_mean", Measures::mean(tier_syll, "f0_mean", isyll, 3, 3, true, "f0_min"));
                syll->setAttribute("int_peak", Measures::mean(tier_syll, "int_peak", isyll, 3, 3, true, "f0_min"));
            }
        }
        // Find target token and corresponding syllables
        int targetTokenIndex(-1);
        for (int i = 0; i < tier_tokens->count(); ++i) {
            if (tier_tokens->at(i)->attribute("target").toString() == "*") {
                targetTokenIndex = i;
                break;
            }
        }
        if (targetTokenIndex < 0) { ret.append(com->ID()).append("Target token not found"); continue; }
        QPair<int, int> targetSyllIndices = tier_syll->getIntervalIndexesOverlappingWith(tier_tokens->at(targetTokenIndex));
        // Hack for alors
        if (targetSyllIndices.second - targetSyllIndices.first >= 2) targetSyllIndices.second--;
        // Prosodic measures on sequences
        double speechRateS1(0.0), speechRateS2(0.0), pauseBeforeMD(0.0), pauseAfterMD(0.0);
        for (int iseq = 0; iseq < tier_sequence->count(); ++iseq) {
            QString s = tier_sequence->at(iseq)->text();
            Interval *unit = new Interval(tier_tokens->getBoundaryClosestTo(tier_sequence->at(iseq)->tMin()),
                                          tier_tokens->getBoundaryClosestTo(tier_sequence->at(iseq)->tMax()), s);
            if (s == "S1") {
                QPair<int, int> syllsS1 = tier_syll->getIntervalIndexesContainedIn(unit);
                if (syllsS1.second - syllsS1.first > 0)
                    speechRateS1 = ((double) (syllsS1.second - syllsS1.first + 1)) / unit->duration().toDouble();
            }
            else if (s == "MD") {
                if ((iseq - 1 > 0) && (tier_sequence->interval(iseq - 1)->isPauseSilent())) {
                    pauseBeforeMD = tier_sequence->interval(iseq - 1)->duration().toDouble();
                }
                if ((iseq + 1 < tier_sequence->count()) && (tier_sequence->interval(iseq + 1)->isPauseSilent())) {
                    pauseAfterMD = tier_sequence->interval(iseq + 1)->duration().toDouble();
                }
            }
            else if (s == "S2") {
                QPair<int, int> syllsS2 = tier_syll->getIntervalIndexesContainedIn(unit);
                if (syllsS2.second - syllsS2.first > 0)
                    speechRateS2 = ((double) (syllsS2.second - syllsS2.first + 1)) / unit->duration().toDouble();
            }
        }
        // Start
        Interval *L3(0); Interval *L2(0); Interval *L1(0); Interval *T1(0); Interval *T2(0); Interval *R1(0); Interval *R2(0); Interval *R3(0);
        int indexL3(-1), indexL2(-1), indexL1(-1), indexT1(-1), indexT2(-1), indexR1(-1), indexR2(-1), indexR3(-1);
        QString textSyll, tonalAnnotation;
        // Get f0_mean baseline
        double f0_mean_baseline = tier_syll->at(targetSyllIndices.first)->attribute("f0_mean").toDouble();
        double normalise(0.0);
        // Left context
        int syllIndex(0); int contextSyllCount(0);
        syllIndex = targetSyllIndices.first - 1;
        while ((syllIndex >= 0) && (contextSyllCount < contextLeft)) {
            Interval *syll = tier_syll->at(syllIndex);
            if (syll->isPauseSilent()) {

            }
            else {
                textSyll.prepend(".").prepend(syll->text());
                tonalAnnotation.prepend(".").prepend(syll->attribute("tonal_annotation").toString());
                contoursLeft[stimulusID][contextSyllCount].durations << syll->duration().toDouble();
                contoursLeft[stimulusID][contextSyllCount].f0_means  << syll->attribute("f0_mean").toDouble() - normalise;
                contextSyllCount++;
                if (contextSyllCount == 1) { L1 = syll; indexL1 = syllIndex; }
                if (contextSyllCount == 2) { L2 = syll; indexL2 = syllIndex; }
                if (contextSyllCount == 3) { L3 = syll; indexL3 = syllIndex; }
            }
            syllIndex--;
        }
        if (textSyll.endsWith(".")) textSyll.chop(1);
        textSyll.append("{");
        if (tonalAnnotation.endsWith(".")) tonalAnnotation.chop(1);
        tonalAnnotation.append("{");
        // Target
        contextSyllCount = 0;
        for (syllIndex = targetSyllIndices.first; syllIndex <= targetSyllIndices.second; ++syllIndex) {
            if ((syllIndex < 0) || (syllIndex >= tier_syll->count())) break;
            Interval * syll = tier_syll->at(syllIndex);
            textSyll.append(syll->text()).append(".");
            tonalAnnotation.append(syll->attribute("tonal_annotation").toString()).append(".");
            targets[stimulusID][syllIndex - targetSyllIndices.first].durations << syll->duration().toDouble();
            targets[stimulusID][syllIndex - targetSyllIndices.first].f0_means  << syll->attribute("f0_mean").toDouble() - normalise;
            contextSyllCount++;
            if (contextSyllCount == 1) { T1 = syll; indexT1 = syllIndex; }
            if (contextSyllCount == 2) { T2 = syll; indexT2 = syllIndex; }
        }
        if (textSyll.endsWith(".")) textSyll.chop(1);
        textSyll.append("}");
        if (tonalAnnotation.endsWith(".")) tonalAnnotation.chop(1);
        tonalAnnotation.append("}");
        // Right context
        contextSyllCount = 0;
        syllIndex = targetSyllIndices.second + 1;
        while ((syllIndex < tier_syll->count()) && (contextSyllCount < contextRight)) {
            Interval *syll = tier_syll->at(syllIndex);
            if (syll->isPauseSilent()) {

            }
            else {
                textSyll.append(syll->text()).append(".");
                tonalAnnotation.append(syll->attribute("tonal_annotation").toString()).append(".");
                contoursRight[stimulusID][contextSyllCount].durations << syll->duration().toDouble();
                contoursRight[stimulusID][contextSyllCount].f0_means  << syll->attribute("f0_mean").toDouble() - normalise;
                contextSyllCount++;
                if (contextSyllCount == 1) { R1 = syll; indexR1 = syllIndex; }
                if (contextSyllCount == 2) { R2 = syll; indexR2 = syllIndex; }
                if (contextSyllCount == 3) { R3 = syll; indexR3 = syllIndex; }
            }
            syllIndex++;
        }
        if (textSyll.endsWith(".")) textSyll.chop(1);
        if (tonalAnnotation.endsWith(".")) tonalAnnotation.chop(1);

        // Output per-stimulus prosodic measures
        // CommunicationID  SpeakerID  StimulusID  UtteranceID  DiscourseMarker  DiscourseRelation
        // T1_duration  T2_duration  T1_f0_mean  T2_f0_mean T1_intersyllab  T2_intersyllab  R1_intersyllab
        // L3_durRel30  L2_durRel30  L1_durRel30  T1_durRel30  T2_durRel30  R1_durRel30  R2_durRel30  R3_durRel30
        // L1_prom  T1_prom  T2_prom  R1_prom  L1_boundary  T1_boundary  T2_boundary  R1_boundary
        // L3_tone  L3_tonemvt  ...  R3_tone  R3_tonemvt
        // SpeechRateS1  SpeechRateS2  PauseBeforeMD  PauseAfterMD

        // Stylised?
        bool T1stylised = (T1) ? (T1->attribute("nucl_t1").toDouble() > 0.0) : false;
        bool T2stylised = (T2) ? (T2->attribute("nucl_t1").toDouble() > 0.0) : false;
        bool R1stylised = (R1) ? (R1->attribute("nucl_t1").toDouble() > 0.0) : false;
        // output
        outPerStim << com->ID() << "\t" << speakerID << "\t" << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
        // duration of the MD
        if (T1) outPerStim << T1->duration().toDouble() << "\t"; else outPerStim << "NA\t";
        if (T2) outPerStim << T2->duration().toDouble() << "\t"; else outPerStim << "NA\t";
        // pitch of the MD
        if (T1stylised) outPerStim << T1->attribute("f0_mean").toDouble() << "\t"; else outPerStim << "NA\t";
        if (T2stylised) outPerStim << T2->attribute("f0_mean").toDouble() << "\t"; else outPerStim << "NA\t";
        // intersyllabic movement S1-->T1 and T1/T2-->R1
        if (T1stylised) outPerStim << T1->attribute("intersyllab").toDouble() << "\t"; else outPerStim << "NA\t";
        if (T2stylised) outPerStim << T2->attribute("intersyllab").toDouble() << "\t"; else outPerStim << "NA\t";
        if (R1stylised) outPerStim << R1->attribute("intersyllab").toDouble() << "\t"; else outPerStim << "NA\t";
        // relative duration (searching for lengthenings)
        if (L3) outPerStim <<  Measures::relative(tier_syll, "duration", indexL3, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (L2) outPerStim <<  Measures::relative(tier_syll, "duration", indexL2, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (L1) outPerStim <<  Measures::relative(tier_syll, "duration", indexL1, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (T1) outPerStim <<  Measures::relative(tier_syll, "duration", indexT1, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (T2) outPerStim <<  Measures::relative(tier_syll, "duration", indexT2, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim <<  Measures::relative(tier_syll, "duration", indexR1, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim <<  Measures::relative(tier_syll, "duration", indexR2, 3, 0, true, "", false) << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim <<  Measures::relative(tier_syll, "duration", indexR3, 3, 0, true, "", false) << "\t"    ; else outPerStim << "NA\t";
        // prominence
        if (L1) outPerStim << L1->attribute("promise_pos").toString() << "\t"; else outPerStim << "NA\t";
        if (T1) outPerStim << T1->attribute("promise_pos").toString() << "\t"; else outPerStim << "NA\t";
        if (T2) outPerStim << T2->attribute("promise_pos").toString() << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim << R1->attribute("promise_pos").toString() << "\t"; else outPerStim << "NA\t";
        // boundaries
        if (L1) outPerStim << L1->attribute("promise_boundary").toString() << "\t"; else outPerStim << "NA\t";
        if (T1) outPerStim << T1->attribute("promise_boundary").toString() << "\t"; else outPerStim << "NA\t";
        if (T2) outPerStim << T2->attribute("promise_boundary").toString() << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim << R1->attribute("promise_boundary").toString() << "\t"; else outPerStim << "NA\t";
        // tonal annotation
        if (L3) outPerStim << L3->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (L3) outPerStim << L3->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (L2) outPerStim << L2->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (L2) outPerStim << L2->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (L1) outPerStim << L1->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (L1) outPerStim << L1->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (T1) outPerStim << T1->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (T1) outPerStim << T1->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (T2) outPerStim << T2->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (T2) outPerStim << T2->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim << R1->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (R1) outPerStim << R1->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (R2) outPerStim << R2->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (R2) outPerStim << R2->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        if (R3) outPerStim << R3->attribute("tonal_label").toString() << "\t"; else outPerStim << "NA\t";
        if (R3) outPerStim << R3->attribute("tonal_movement").toString() << "\t"; else outPerStim << "NA\t";
        // speech rate - pauses
        outPerStim << speechRateS1 << "\t" << speechRateS2 << "\t";
        outPerStim << pauseBeforeMD << "\t" << pauseAfterMD << "\n";

        // Update Communication with prosodic measures
        com->setProperty("speechRateS1", speechRateS1);
        com->setProperty("speechRateS2", speechRateS2);
        com->setProperty("pauseBeforeMD", pauseBeforeMD);
        com->setProperty("pauseAfterMD", pauseAfterMD);

        // User output
        ret.append(stimulusID).append("\t").append(speakerID).append("\t").append(textSyll).append("\t").append(tonalAnnotation).append("\n");
        delete tier_tokens;
        delete tier_syll;
    }
    // Output
    QFile fileAggr(path + "/average_contours.txt");
    if ( !fileAggr.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return "Error writing output file";
    QTextStream outAggr(&fileAggr);
    outAggr.setCodec("UTF-8");
    outAggr << "StimulusID\tUtteranceID\tDiscourseMarker\tDiscourseRelation\tSyll\tTime\tDurationMean\tDurationStdev\tPitchMean\tPitchStdev\n";
    foreach (QString stimulusID, contoursLeft.keys()) {
        QList<AggregateSyllable> syllablesLeft   = contoursLeft[stimulusID];
        QList<AggregateSyllable> syllablesTarget = targets[stimulusID];
        QList<AggregateSyllable> syllablesRight  = contoursRight[stimulusID];
        double time(0.0);
        // left
        for (int i = syllablesLeft.count() - 1; i >= 0; --i) {
            AggregateSyllable syll = syllablesLeft.at(i);
            StatisticalSummary summary_duration(syll.durations);
            StatisticalSummary summary_pitch(syll.f0_means);
            time = time - summary_duration.mean();
            outAggr << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            outAggr << QString("L%1").arg(- contextLeft + i) << "\t" << time << "\t";
            outAggr << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            outAggr << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
        }
        time = 0.0;
        for (int i = 0; i < syllablesTarget.count(); ++i) {
            AggregateSyllable syll = syllablesTarget.at(i);
            if (syll.durations.count() == 0) break;
            StatisticalSummary summary_duration(syll.durations);
            StatisticalSummary summary_pitch(syll.f0_means);
            outAggr << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            outAggr << QString("T%1").arg(i + 1) << "\t" << time << "\t";
            outAggr << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            outAggr << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
            time = time + summary_duration.mean();
        }
        for (int i = 0; i < syllablesRight.count(); ++i) {
            AggregateSyllable syll = syllablesRight.at(i);
            if (syll.durations.count() == 0) break;
            StatisticalSummary summary_duration(syll.durations);
            StatisticalSummary summary_pitch(syll.f0_means);
            outAggr << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            outAggr << QString("R%1").arg(i + 1) << "\t" << time << "\t";
            outAggr << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            outAggr << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
            time = time + summary_duration.mean();
        }
    }
    // Save prosodic measures written to metadata
    corpus->save();
    // Clean-up output files
    fileAggr.close();
    filePerStim.close();
    return ret;
}
