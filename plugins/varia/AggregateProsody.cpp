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
        // If this is the first time, insert contour
        if (!contoursLeft.contains(stimulusID)) {
            QList<AggregateSyllable> left;
            for (int i = 1; i <= 5; ++i) left.append(AggregateSyllable(""));
            contoursLeft.insert(stimulusID, left);
            QList<AggregateSyllable> target;
            for (int i = 1; i <= 2; ++i) target.append(AggregateSyllable(""));
            targets.insert(stimulusID, target);
            QList<AggregateSyllable> right;
            for (int i = 1; i <= 5; ++i) right.append(AggregateSyllable(""));
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
        QString textSyll;
        // Get f0_mean baseline
        double f0_mean_baseline = tier_syll->at(targetSyllIndices.first)->attribute("f0_mean").toDouble();
        double normalise(0.0);
        // Left context
        int syllIndex(0); int contextSyllCount(0);
        syllIndex = targetSyllIndices.first - 1;
        while ((syllIndex >= 0) && (contextSyllCount < 5)) {
            Interval *syll = tier_syll->at(syllIndex);
            if (syll->isPauseSilent()) {

            }
            else {
                textSyll.prepend(".").prepend(syll->text());
                contoursLeft[stimulusID][contextSyllCount].durations << syll->duration().toDouble();
                contoursLeft[stimulusID][contextSyllCount].f0_means  << syll->attribute("f0_mean").toDouble() - normalise;
                contextSyllCount++;
            }
            syllIndex--;
        }
        if (textSyll.startsWith(".")) textSyll.remove(0, 1);
        textSyll.append("{");
        // Target
        for (syllIndex = targetSyllIndices.first; syllIndex <= targetSyllIndices.second; ++syllIndex) {
            if ((syllIndex < 0) || (syllIndex >= tier_syll->count())) break;
            Interval * syll = tier_syll->at(syllIndex);
            textSyll.append(syll->text()).append(".");
            targets[stimulusID][syllIndex - targetSyllIndices.first].durations << syll->duration().toDouble();
            targets[stimulusID][syllIndex - targetSyllIndices.first].f0_means  << syll->attribute("f0_mean").toDouble() - normalise;
        }
        if (textSyll.endsWith(".")) textSyll.chop(1);
        textSyll.append("}");
        // Right context
        syllIndex = targetSyllIndices.second + 1; contextSyllCount = 0;
        while ((syllIndex < tier_syll->count()) && (contextSyllCount < 5)) {
            Interval *syll = tier_syll->at(syllIndex);
            if (syll->isPauseSilent()) {

            }
            else {
                textSyll.append(syll->text()).append(".");
                contoursRight[stimulusID][contextSyllCount].durations << syll->duration().toDouble();
                contoursRight[stimulusID][contextSyllCount].f0_means  << syll->attribute("f0_mean").toDouble() - normalise;
                contextSyllCount++;
            }
            syllIndex++;
        }
        if (textSyll.endsWith(".")) textSyll.chop(1);
        ret.append(stimulusID).append("\t").append(speakerID).append("\t").append(textSyll).append("\n");
        delete tier_tokens;
        delete tier_syll;
    }
    // return ret;
    // Output
    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/03_prosodie-relations-de-discours/Production Analyses";
    QFile file(path + "/average_contours.txt");
    if ( !file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) ) return "Error writing output file";
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "StimulusID\tUtteranceID\tDiscourseMarker\tDiscourseRelation\tSyll\tTime\tDurationMean\tDurationStdev\tPitchMean\tPitchStdev\n";
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
            out << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            out << QString("L%1").arg(i + 1) << "\t" << time << "\t";
            out << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            out << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
        }
        time = 0.0;
        for (int i = 0; i < syllablesTarget.count(); ++i) {
            AggregateSyllable syll = syllablesTarget.at(i);
            if (syll.durations.count() == 0) break;
            StatisticalSummary summary_duration(syll.durations);
            StatisticalSummary summary_pitch(syll.f0_means);
            out << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            out << QString("T%1").arg(i + 1) << "\t" << time << "\t";
            out << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            out << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
            time = time + summary_duration.mean();
        }
        for (int i = 0; i < syllablesRight.count(); ++i) {
            AggregateSyllable syll = syllablesRight.at(i);
            if (syll.durations.count() == 0) break;
            StatisticalSummary summary_duration(syll.durations);
            StatisticalSummary summary_pitch(syll.f0_means);
            out << stimulusID << "\t" << stimulusID.left(3) << "\t" << stimulusID.left(1) << "\t" << stimulusID.mid(3, 3) << "\t";
            out << QString("R%1").arg(i + 1) << "\t" << time << "\t";
            out << summary_duration.mean() << "\t" << summary_duration.stDev() << "\t";
            out << summary_pitch.mean() << "\t" << summary_pitch.stDev() << "\n";
            time = time + summary_duration.mean();
        }
    }
    file.close();
    return ret;
}
