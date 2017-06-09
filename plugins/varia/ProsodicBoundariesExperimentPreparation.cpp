#include <QDebug>
#include <QString>
#include <QList>
#include <QPointer>

#include "pncore/base/RealValueList.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "ProsodicBoundariesExperimentPreparation.h"

// PROSODIC BOUNDARIES EXPERIMENT
// ====================================================================================================================
// 1. Stimuli selection
// ====================================================================================================================

bool ProsodicBoundariesExperimentPreparation::ipuIsMonological(Interval *ipu, IntervalTier *timeline, QString &speaker)
{
    QString spk;
    QList<Interval *> timelineIntervals = timeline->getIntervalsOverlappingWith(ipu);
    bool first = true;
    foreach (Interval *timelineIntv, timelineIntervals) {
        if (timelineIntv->text().isEmpty()) continue;
        if (first) {
            spk = timelineIntv->text(); first = false;
        }
        else {
            if (spk != timelineIntv->text()) {
                qDebug() << spk << timelineIntv->text();
                return false;
            }
        }
    }
    speaker = spk;
    return true;
}

void ProsodicBoundariesExperimentPreparation::measuresForPotentialStimuli(QPointer<CorpusAnnotation> annot, QList<Interval *> &stimuli, QTextStream &out,
                                         IntervalTier *tier_syll, IntervalTier *tier_tokmin)
{
    foreach (Interval *stim, stimuli) {
        double pauseTotalDur = 0.0;
        int numSyllables = 0, numSilentPauses = 0, numFilledPauses = 0, numRepetitionDIPs = 0;
        int numProsodicBoundaries = 0, numBoundariesT2 = 0, numBoundariesT3 = 0;
        double trajectory = 0.0;
        QList<Interval *> syllables = tier_syll->getIntervalsContainedIn(stim);
        QList<Interval *> tokens = tier_tokmin->getIntervalsContainedIn(stim);
        RealValueList syll_durations;
        foreach (Interval *syll, syllables) {
            if (syll->text() == "_" || syll->text().isEmpty()) {
                numSilentPauses++;
                pauseTotalDur += syll->duration().toDouble();
            } else {
                numSyllables++;
                trajectory += syll->attribute("trajectory").toDouble();
                // exclude filled pauses from CV calculation
                Interval *token = tier_tokmin->intervalAtTime(syll->tCenter());
                if (!token->attribute("disfluency").toString().contains("FIL"))
                    syll_durations.append(syll->duration().toDouble());
            }
            QString boundary = syll->attribute("boundary").toString();
            QString contour = syll->attribute("contour").toString();
            if (boundary.contains("//") || boundary.contains("///")) {
                numProsodicBoundaries++;
                if (contour == "T" && boundary.contains("///")) numBoundariesT3++;
                else if (contour == "T" && boundary.contains("//")) numBoundariesT2++;
            }
        }
        double articulationRatio = (stim->duration().toDouble() - pauseTotalDur) / stim->duration().toDouble();
        double speechRate = syllables.count() / stim->duration().toDouble();
        double articulationRate = syllables.count() / (stim->duration().toDouble() - pauseTotalDur);
        double syllDurationCV = syll_durations.stddev() / syll_durations.mean();
        double melodicPath = trajectory / (stim->duration().toDouble() - pauseTotalDur);
        foreach (Interval *token, tokens) {
            if (token->attribute("disfluency").toString().contains("FIL")) numFilledPauses++;
            if (token->attribute("disfluency").toString().contains("REP*")) numRepetitionDIPs++;
        }

        out << annot->ID() << "\t" << stim->text() << "\t";
        out << QString("%1").arg(stim->duration().toDouble()).replace(".", ",") << "\t";
        // Silent pauses, number and duration
        out << numSilentPauses << "\t";
        out << QString("%1").arg(pauseTotalDur).replace(".", ",") << "\t";
        // Speech rate, articulation rate
        out << QString("%1").arg(articulationRatio).replace(".", ",") << "\t";
        out << QString("%1").arg(articulationRate).replace(".", ",") << "\t";
        out << QString("%1").arg(speechRate).replace(".", ",") << "\t";
        // Coefficient of variation of syllable length
        out << QString("%1").arg(syllDurationCV).replace(".", ",") << "\t";
        // Disfluencies
        out << numFilledPauses << "\t" << numRepetitionDIPs << "\t";
        out << QString("%1").arg(double(numFilledPauses) / double(numSyllables)).replace(".", ",") << "\t";
        // Melodic path
        out << QString("%1").arg(melodicPath).replace(".", ",") << "\t";
        // Perceptual boundary annotation
        out << numProsodicBoundaries << "\t" << numBoundariesT2 << "\t" << numBoundariesT3 << "\t";
        out << QString("%1").arg(double(numSilentPauses) / double(numProsodicBoundaries)).replace(".", ",") << "\t";

        out << stim->attribute("transcription").toString() << "\n";
    }

}

void ProsodicBoundariesExperimentPreparation::potentialStimuliFromSample(Corpus *corpus, QPointer<CorpusAnnotation> annot, QTextStream &out)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->repository()->annotations()->getTiersAllSpeakers(annot->ID());
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
        IntervalTier *timeline = corpus->repository()->annotations()->getSpeakerTimeline("", annot->ID(), "tok_mwu");
        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        if (!tier_syll) continue;
        IntervalTier *tier_tokmin = tiers->getIntervalTierByName("tok_min");
        if (!tier_tokmin) continue;
        QList<Interval *> pauseBoundariesForIPUs;
        QList<Interval *> potentialStimuli;
        double pauseThreshold = 0.250, minimumStimulusLength = 20.0, maximumStimulusLength = 60.0;

        // Select pauses that are over the threshold => potential IPU boundaries
        foreach (Interval *syll, tier_syll->intervals()) {
            if (syll->text() == "_") {
                // first and last silence are always included
                int pause_index = timeline->intervalIndexAtTime(syll->tCenter());
                if (pause_index == 0 || pause_index == timeline->count() - 1) {
                    pauseBoundariesForIPUs << new Interval(syll);
                }
                // exclude if pause is smaller than threshold
                if (syll->duration().toDouble() < pauseThreshold) continue;
                // add to IPU boundaries
                pauseBoundariesForIPUs << new Interval(syll);
            }
        }
        while (potentialStimuli.isEmpty() && (minimumStimulusLength > 2.0)) {
            // Select sequences of IPUs that are monological and are between the min, max stimulus length
            for (int i = 0; i < pauseBoundariesForIPUs.count() - 1; ++i) {
                for (int j = i + 1; j < pauseBoundariesForIPUs.count(); ++j) {
                    Interval *left = pauseBoundariesForIPUs.at(i); Interval *right = pauseBoundariesForIPUs.at(j);
                    RealTime stimulusDuration = right->tMin() - left->tMax();
                    if (stimulusDuration.toDouble() < minimumStimulusLength) continue;
                    if (stimulusDuration.toDouble() > maximumStimulusLength) break;
                    qDebug() << stimulusDuration.toDouble();
                    Interval *potentialStimulus = new Interval(left->tMax(), right->tMin(), "");
                    QString spk;
                    if (!ipuIsMonological(potentialStimulus, timeline, spk)) {
                        delete potentialStimulus;
                        continue;
                    }
                    // Got a potential stimulus!
                    QList<Interval *> tokens = tier_tokmin->getIntervalsContainedIn(potentialStimulus);
                    QString transcription;
                    foreach (Interval *token, tokens) {
                        transcription.append(token->text()).append(" ");
                    }
                    potentialStimulus->setText(QString("IPU %1 (%2) %3 - %4").arg(i+1).arg(spk)
                                               .arg(left->tMax().toDouble()).arg(right->tMin().toDouble()));
                    potentialStimulus->setAttribute("transcription", transcription);
                    potentialStimulus->setAttribute("speaker", spk);
                    potentialStimuli << potentialStimulus;
                }
            }
            // Measures on potential stimuli
            measuresForPotentialStimuli(annot, potentialStimuli, out, tier_syll, tier_tokmin);
            minimumStimulusLength -= 2.0;
        }
    }
    qDeleteAll(tiersAll);
}

void ProsodicBoundariesExperimentPreparation::potentialStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    QFile file("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/actual_stimuli.txt");
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "annotationID\tstimulusID\tduration\tnumSilentPauses\tpauseTotalDur\tarticulationRatio\tarticulationRate\tspeechRate\tsyllDurationCV\tnumFilledPauses\tnumRepetitionDIPs\tratioFILtoNumSyll\tmelodicPath\tnumProsodicBoundaries\tnumBoundariesT2\tnumBoundariesT3\tratioSILtoPBs\ttranscription\n";

    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            potentialStimuliFromSample(corpus, annot, out);
        }
    }
    file.close();
}

void ProsodicBoundariesExperimentPreparation::actualStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    QFile file("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/actual_stimuli.txt");
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "annotationID\tstimulusID\tduration\tnumSilentPauses\tpauseTotalDur\tarticulationRatio\tarticulationRate\tspeechRate\tsyllDurationCV\tnumFilledPauses\tnumRepetitionDIPs\tratioFILtoNumSyll\tmelodicPath\tnumProsodicBoundaries\tnumBoundariesT2\tnumBoundariesT3\tratioSILtoPBs\ttranscription\n";

    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->repository()->annotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
                if (!tier_syll) continue;
                IntervalTier *tier_tokmin = tiers->getIntervalTierByName("tok_min");
                if (!tier_tokmin) continue;
                QList<Interval *> actualStimuli;
                actualStimuli << new Interval(tier_syll->first()->tMax(), tier_syll->last()->tMin(), "");
                measuresForPotentialStimuli(annot, actualStimuli, out, tier_syll, tier_tokmin);
            }
            qDeleteAll(tiersAll);
        }
    }
    file.close();
}
