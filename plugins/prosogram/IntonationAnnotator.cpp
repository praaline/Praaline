#include <QDebug>
#include <QString>
#include <QList>
#include <QHash>
#include <QPair>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/Interval.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "IntonationAnnotator.h"

// Intonation Annotator - Polytonia Model
// Five pitch levels are defined: Bottom and Top of the speaker's pitch range, as well as Low, Mid, and High,
// which are determined on the basis of pitch changes in the local context.
// Five elementary pitch movements of individual syllables are distinguished on the basis of direction
// (rise, fall, level) and size (large and small melodic intervals, adjusted to the speaker's pitch range).

struct IntonationAnnotatorData {
    QHash<QString, IntonationAnnotator::PitchRangeInfo> pitchRanges;
};

IntonationAnnotator::IntonationAnnotator() :
    d(new IntonationAnnotatorData)
{
}

IntonationAnnotator::~IntonationAnnotator()
{
    delete d;
}

IntonationAnnotator::PitchRangeInfo IntonationAnnotator::pitchRangeInfo(const QString &speakerID) const
{
    return d->pitchRanges.value(speakerID);
}

double calculatePercentile(const QList<double> &data, int percentile)
{
    int n = data.count();
    double i = ((double)(n * percentile)) / 100.0 + 0.5;
    int k = (int) floor(i);
    double f = i - floor(i);
    if ((k < 0) || (k + 1 >= data.count())) return qQNaN();
    return (1.0 - f) * data.at(k) + f * data.at(k + 1);
}

// Pitch range estimation
// For each syllable pronounced by a given speaker, two pitch values are obtained: the minimum and maximum pitch
// inside the syllabic nucleus.
// The 2nd and 98th percentiles of this set of data provide an estimate of the bottom and top of the global pitch
// range, respectively. In this way, outliers due to pitch detection errors and co-intrinsic pitch phenomena are
// mostly eliminated. Pitch range detection is based on all syllables for a given speaker in the corpus, rather
// than on individual utterances.
void IntonationAnnotator::estimatePitchRange(QPointer<Corpus> corpus, const QStringList &speakerIDs)
{
    if (!corpus) return;
    QStringList effectiveSpeakerIDs;
    if (speakerIDs.isEmpty())
        effectiveSpeakerIDs = corpus->speakerIDs();
    else
        effectiveSpeakerIDs = speakerIDs;
    d->pitchRanges.clear();
    foreach (QString speakerID, effectiveSpeakerIDs) {
        QList<Interval *> syllables = corpus->repository()->annotations()->getIntervals(
                    AnnotationDatastore::Selection("", speakerID, "syll", QStringList()
                                                   << "f0_min" << "f0_max" << "f0_median"));
        QList<double> f0_mininma, f0_maxima, f0_median;
        foreach (Interval *syll, syllables) {
            if (syll->isPauseSilent()) continue;                        // silent pause
            if (syll->attribute("f0_min").toDouble() == 0.0) continue;  // not stylized
            f0_mininma << syll->attribute("f0_min").toDouble();
            f0_maxima  << syll->attribute("f0_max").toDouble();
            f0_median  << syll->attribute("f0_median").toDouble();
        }
        qSort(f0_mininma);
        qSort(f0_median);
        qSort(f0_maxima);
        PitchRangeInfo pitchRange;
        pitchRange.medianHz = calculatePercentile(f0_median, 50);
        // Remove outliers
        while ((!f0_mininma.isEmpty()) &&
               (fabs((12.0 * log2(f0_mininma.first())) - pitchRange.medianST()) > 18.0)) {
            f0_mininma.takeFirst();
        }
        while ((!f0_maxima.isEmpty()) &&
               (fabs((12.0 * log2(f0_maxima.last())) - pitchRange.medianST()) > 18.0)) {
            f0_maxima.takeLast();
        }
        pitchRange.bottomHz = calculatePercentile(f0_mininma, 2);
        pitchRange.topHz = calculatePercentile(f0_maxima, 98);
        pitchRange.countSyll = qMin(f0_mininma.count(), f0_maxima.count());
        d->pitchRanges.insert(speakerID, pitchRange);
        qDebug() << speakerID << "\t" << pitchRange.bottomST() << "\t" << pitchRange.medianST() << "\t"
                 << pitchRange.topST() << "\t" << pitchRange.countSyll;
    }
}

// Intra-syllabic pitch variation
// Tonal segments with an audible pitch variation are categorized as large or small pitch intervals.
// Pitch intervals: large - small
// Pitch range      Large interval      Small interval
// > 8.5 ST         > 4.5 ST            3.0 - 4.5 ST
// 7.0 – 8.5 ST     > 3.5 ST            2.5 - 3.5 ST
// < 7.0 ST         > 3.2 ST            2.5 - 3.2 ST
QString IntonationAnnotator::classifyPitchInterval(const PitchRangeInfo &pitchRange, double fromHz, double toHz)
{
    // Pitches are stored in Hz. Convert to ST relative to 1 Hz.
    double fromST = 12.0 * log2(fromHz);
    double toST = 12.0 * log2(toHz);
    double mvtST = toST - fromST;
    if      (pitchRange.rangeST() > 8.5) {
        if      (mvtST > 4.5)   return "R";
        else if (mvtST < -4.5)  return "F";
        else if ((mvtST >= 3.0 ) && (mvtST <= 4.5))   return "r";
        else if ((mvtST <= -3.0 ) && (mvtST >= -4.5)) return "f";
        else return "_";
    }
    else if ((pitchRange.rangeST() >= 7.0) && (pitchRange.rangeST() <= 8.5)) {
        if      (mvtST > 3.5)   return "R";
        else if (mvtST < -3.5)  return "F";
        else if ((mvtST >= 2.5 ) && (mvtST <= 3.5))   return "r";
        else if ((mvtST <= -2.5 ) && (mvtST >= -3.5)) return "f";
        else return "_";
    }
    else {
        if      (mvtST > 3.2)   return "R";
        else if (mvtST < -3.2)  return "F";
        else if ((mvtST >= 2.5 ) && (mvtST <= 3.2))   return "r";
        else if ((mvtST <= -2.5 ) && (mvtST >= -3.2)) return "f";
        else return "_";
    }
    return "_";
}

// Pitch level detection based on pitch span
QString IntonationAnnotator::pitchLevelBasedOnPitchSpan(const PitchRangeInfo &pitchRange, IntervalTier *tier_syll, int indexTargetSyll)
{
    if (!tier_syll) return QString();
    if ((indexTargetSyll < 0) || (indexTargetSyll >= tier_syll->count())) return QString();
    Interval *syll = tier_syll->at(indexTargetSyll);
    double f0_start = syll->attribute("f0_start").toDouble();
    double f0_start_ST = 12.0 * log2(f0_start);
    if (f0_start_ST > pitchRange.topST()) {
        if (pitchRange.upperST() > 8.0) return "T"; else return "H";
    }
    else if (f0_start_ST < pitchRange.bottomST()) {
        return "B";
    }
    return QString();
}

// Pitch level detection based on local pitch change
QString IntonationAnnotator::pitchLevelBasedOnLocalPitchChange(const PitchRangeInfo &pitchRange, IntervalTier *tier_syll, int indexTargetSyll)
{
    if (!tier_syll) return QString();
    if ((indexTargetSyll < 0) || (indexTargetSyll >= tier_syll->count())) return QString();
    Interval *syll = tier_syll->at(indexTargetSyll);
    QList<Interval *> context;
    for (int i = indexTargetSyll - 1; i >= indexTargetSyll - 3; --i) {
        if (i < 0) break;
        // A silent pause breaks the context
        if (tier_syll->at(i)->isPauseSilent()) break;
        // Syllables in the context must be within 500 ms of the target syllable
        if (syll->tMin() - tier_syll->at(i)->tMin() > RealTime::fromMilliseconds(500)) break;
        // Non-stylised syllables are ignored
        if (tier_syll->at(i)->attribute("f0_min").toDouble() == 0.0) continue;
        // Syllables annotated as Top or Bottom are ignored
        if (tier_syll->at(i)->attribute("tonal_label_method").toString() == "PITCH_SPAN") continue;
        // Finally! A syllable worthy to be part of the left context
        context << tier_syll->at(i);
    }
    if (context.isEmpty()) return QString();
    // the minimum and maximum pitch value of the context
    double fmin(context.first()->attribute("f0_min").toDouble());
    double fmax(context.first()->attribute("f0_max").toDouble());
    foreach (Interval *contextsyll, context) {
        if (contextsyll->attribute("f0_min").toDouble() < fmin) fmin = contextsyll->attribute("f0_min").toDouble();
        if (contextsyll->attribute("f0_max").toDouble() > fmax) fmax = contextsyll->attribute("f0_max").toDouble();
    }
    // Calculate Iup, Idown and assign pitch level accordingly
    double f0_start = syll->attribute("f0_start").toDouble();
    QString Iup = classifyPitchInterval(pitchRange, fmin, f0_start);
    if (Iup == "R") return "H";
    if (Iup == "r") return "M";
    QString Idown = classifyPitchInterval(pitchRange, fmax, f0_start);
    if (Idown == "F") return "L";
    return QString();
}

// Pitch level inferred from intra-syllabic pitch movements
QString IntonationAnnotator::pitchLevelInferredFromIntrasyllabicMvt(const PitchRangeInfo &pitchRange, IntervalTier *tier_syll, int indexTargetSyll)
{
    if (!tier_syll) return QString();
    if ((indexTargetSyll < 0) || (indexTargetSyll >= tier_syll->count())) return QString();
    Interval *syll = tier_syll->at(indexTargetSyll);
    double f0_start = syll->attribute("f0_start").toDouble();
    double f0_start_ST = 12.0 * log2(f0_start);
    double f0_end = syll->attribute("f0_end").toDouble();
    double f0_end_ST = 12.0 * log2(f0_end);
    double distFromCenter = f0_start_ST - pitchRange.medianST();
    QString mvt = syll->attribute("tonal_movement").toString();
    // A large rise (R) starting below the center is labeled as a low pitch level (L).
    if ((mvt == "R") && (f0_start_ST < pitchRange.medianST())) return "L";
    // A large rise (R) starting slightly above the center receives a mid pitch level (M).
    if ((mvt == "R") && (distFromCenter > 0.0) && (distFromCenter < 2.0)) return "M";
    // A large fall (F) starting below the center and ending at the bottom of the pitch range receives a low pitch level (L).
    if ((mvt == "F") && (distFromCenter < 0.0) && (f0_end_ST <= pitchRange.bottomST())) return "L";
    // A large fall (F) starting in the upper quarter of the pitch range receives a high pitch level (H).
    if ((mvt == "F") && (f0_start_ST > (pitchRange.medianST() + pitchRange.upperST() / 2.0))) return "H";
    // A small rise (r) which starts slightly higher (within a "small" interval) than the preceding syllable, which is labeled low,
    // will be labeled as starting at a low pitch level (L).
    if ((mvt == "r") && (indexTargetSyll > 0) && tier_syll->at(indexTargetSyll-1)->attribute("tonal_label") == "L") {
        QString intv = classifyPitchInterval(pitchRange, tier_syll->at(indexTargetSyll-1)->attribute("f0_end").toDouble(), f0_start);
        if (intv == "r") return "L";
    }
    // A syllable with a level pitch (_), starting in the upper quarter of the pitch range is labeled high (H).
    if ((mvt == "_") && (f0_start_ST > (pitchRange.medianST() + pitchRange.upperST() / 2.0))) return "H";
    return QString();
}

// Extrapolating pitch level information
QString IntonationAnnotator::pitchLevelExtrapolated(const PitchRangeInfo &pitchRange, Interval *target_syll, Interval *ref_syll)
{
    if (!target_syll) return QString();
    if (!ref_syll) return QString();
    QString tonal_label_ref = ref_syll->attribute("tonal_label").toString();
    if (tonal_label_ref.isEmpty()) return QString();
    // Extrapolate
    double f0_start = target_syll->attribute("f0_start").toDouble();
    double f0_ref = ref_syll->attribute("f0_start").toDouble();
    QString intv = classifyPitchInterval(pitchRange, f0_ref, f0_start);
    if (intv == "_")
        return tonal_label_ref;
    if (intv == "R" && tonal_label_ref == "L") return "H";
    if (intv == "R" && tonal_label_ref == "M") return "H";
    if (intv == "R" && tonal_label_ref == "H") return "H";
    if (intv == "r" && tonal_label_ref == "L") return "M";
    if (intv == "r" && tonal_label_ref == "M") return "H";
    if (intv == "r" && tonal_label_ref == "H") return "H";
    if (intv == "F" && tonal_label_ref == "L") return "L";
    if (intv == "F" && tonal_label_ref == "M") return "L";
    if (intv == "F" && tonal_label_ref == "H") return "L";
    if (intv == "f" && tonal_label_ref == "L") return "M";
    if (intv == "f" && tonal_label_ref == "M") return "L";
    if (intv == "f" && tonal_label_ref == "H") return "L";
    return tonal_label_ref;
}

void IntonationAnnotator::pitchLevelExtrapolated(const PitchRangeInfo &pitchRange, IntervalTier *tier_syll)
{
    if (!tier_syll) return;
    for (int index = 0; index < tier_syll->count(); ++index) {
        Interval *syll = tier_syll->at(index);
        if (syll->isPauseSilent()) continue;
        if (syll->attribute("f0_min").toDouble() == 0.0) continue;
        if (!syll->attribute("tonal_label").toString().isEmpty()) continue;
        // Backward search - the target precedes the reference syllable
        bool found(false);
        int indexRef = index + 1;
        while ((!found) && (indexRef < tier_syll->count())) {
            Interval *syllRef = tier_syll->at(indexRef);
            if (syllRef->tMin() - syll->tMax() > RealTime::fromMilliseconds(500)) break;
            QString extrapolated = pitchLevelExtrapolated(pitchRange, syll, syllRef);
            if (!extrapolated.isEmpty()) {
                syll->setAttribute("tonal_label", extrapolated);
                syll->setAttribute("tonal_label_method", "EXTRAPOLATE");
                found = true;
            }
            ++indexRef;
        }
        indexRef = index - 1;
        while ((!found) && (indexRef >= 0)) {
            Interval *syllRef = tier_syll->at(indexRef);
            if (syll->tMin() - syllRef->tMax() > RealTime::fromMilliseconds(500)) break;
            QString extrapolated = pitchLevelExtrapolated(pitchRange, syll, syllRef);
            if (!extrapolated.isEmpty()) {
                syll->setAttribute("tonal_label", extrapolated);
                syll->setAttribute("tonal_label_method", "EXTRAPOLATE");
                found = true;
            }
            --indexRef;
        }
    }
}

// Pitch level for plateaus
void IntonationAnnotator::pitchLevelForPlateaus(const PitchRangeInfo &pitchRange, IntervalTier *tier_syll)
{
    if (!tier_syll) return;
    for (int index = 1; index < tier_syll->count() - 1; ++index) {
        Interval *syllA = tier_syll->at(index - 1);
        Interval *syllB = tier_syll->at(index);
        Interval *syllC = tier_syll->at(index + 1);
        if (syllA->isPauseSilent() || syllB->isPauseSilent() || syllC->isPauseSilent()) continue;
        if (syllA->attribute("f0_min").toDouble() == 0.0) continue;
        if (syllB->attribute("f0_min").toDouble() == 0.0) continue;
        if (syllC->attribute("f0_min").toDouble() == 0.0) continue;
        if (!syllA->attribute("tonal_label").toString().isEmpty()) continue;
        if (!syllB->attribute("tonal_label").toString().isEmpty()) continue;
        if (!syllC->attribute("tonal_label").toString().isEmpty()) continue;
        // Found a sequence of three non-labelled but stylised syllables
        double f0_start_A_ST = 12.0 * log2(syllA->attribute("f0_start").toDouble());
        double f0_start_B_ST = 12.0 * log2(syllB->attribute("f0_start").toDouble());
        double f0_start_C_ST = 12.0 * log2(syllC->attribute("f0_start").toDouble());
        // Check that the movement between them is negligible
        if (fabs(f0_start_B_ST - f0_start_A_ST) > 1.2) continue;
        if (fabs(f0_start_C_ST - f0_start_B_ST) > 1.2) continue;
        // OK, label first syllable
        if (f0_start_A_ST < pitchRange.medianST()) {
            syllA->setAttribute("tonal_label", "L");
            syllA->setAttribute("tonal_label_method", "PLATEAU");
        }
        else if (f0_start_A_ST < (pitchRange.medianST() + pitchRange.upperST() / 2.0)) {
            syllA->setAttribute("tonal_label", "M");
            syllA->setAttribute("tonal_label_method", "PLATEAU");
        }
    }
}


// Pitch movement classification
// Large movements: R large rise, F large fall
// Small movements: r small rise, f small fall
// No movement:     _ flat
// Special symbols: S sustain - indicates a syllable with a uniform level pitch and minimal duration of 250 ms
//                              a marked contour which is fairly rare in French.
//                  C creak   - indicates a syllable with creak
// Pitch level assignment:
// Global levels (relative to the speaker's pitch range) : T top, B bottom
// Local levels   (relative to the context)              : L low, M mid, H high
void IntonationAnnotator::annotate(QPointer<CorpusCommunication> com)
{
    if (!com) return;
    if (!com->repository()) return;
    foreach (QPointer<CorpusRecording> rec, com->recordings()) {
        if (!rec) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                PitchRangeInfo pitchRange = d->pitchRanges.value(speakerID);
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                // Annotate pitch movements inside syllables (tonal segments)
                IntervalTier *tier_tonal_segments = tiers->getIntervalTierByName("prosogram_tonal_segments");
                if (!tier_tonal_segments) continue;
                foreach (Interval *segment, tier_tonal_segments->intervals()) {
                    if (segment->isPauseSilent()) continue;
                    double f0_start_Hz = segment->attribute("f0_start").toDouble();
                    double f0_end_Hz   = segment->attribute("f0_end").toDouble();
                    QString label = classifyPitchInterval(pitchRange, f0_start_Hz, f0_end_Hz);
                    if (segment->duration().toDouble() > 0.250 && label == "_") label = "S"; // sustain
                    segment->setAttribute("tone_label", label);
                }
                IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
                if (!tier_syll) continue;
                for (int indexSyll = 0; indexSyll < tier_syll->count(); ++indexSyll) {
                    Interval *syll = tier_syll->at(indexSyll);
                    if (syll->isPauseSilent()) continue;
                    double f0_start = syll->attribute("f0_start").toDouble();
                    if (f0_start == 0.0) continue;      // not stylised
                    // Get tonal segment movement information
                    QString tonal_movement;
                    foreach (Interval *segment, tier_tonal_segments->getIntervalsContainedIn(syll)) {
                        tonal_movement.append(segment->attribute("tone_label").toString());
                    }
                    tonal_movement = tonal_movement.replace("__", "_").replace("__", "_");
                    syll->setAttribute("tonal_movement", tonal_movement);
                    // Assign pitch level
                    QString tonal_label;
                    if (tonal_label.isEmpty()) {
                        tonal_label = pitchLevelBasedOnPitchSpan(pitchRange, tier_syll, indexSyll);
                        if (!tonal_label.isEmpty()) {
                            syll->setAttribute("tonal_label", tonal_label);
                            syll->setAttribute("tonal_label_method", "PITCH_SPAN");
                        }
                    }
                    if (tonal_label.isEmpty()) {
                        tonal_label = pitchLevelBasedOnLocalPitchChange(pitchRange, tier_syll, indexSyll);
                        if (!tonal_label.isEmpty()) {
                            syll->setAttribute("tonal_label", tonal_label);
                            syll->setAttribute("tonal_label_method", "LOCAL_CHANGE");
                        }
                    }
                    if (tonal_label.isEmpty()) {
                        tonal_label = pitchLevelInferredFromIntrasyllabicMvt(pitchRange, tier_syll, indexSyll);
                        if (!tonal_label.isEmpty()) {
                            syll->setAttribute("tonal_label", tonal_label);
                            syll->setAttribute("tonal_label_method", "INTRASYLLAB");
                        }
                    }
                }
                // Extrapolate + Pitch for plateaus
                pitchLevelExtrapolated(pitchRange, tier_syll);
                pitchLevelForPlateaus(pitchRange, tier_syll);
                pitchLevelExtrapolated(pitchRange, tier_syll);
                pitchLevelExtrapolated(pitchRange, tier_syll);
                pitchLevelExtrapolated(pitchRange, tier_syll);
                // Save changes
                com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tonal_segments);
                com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_syll);
            }
            qDeleteAll(tiersAll);
        }
    }
}
