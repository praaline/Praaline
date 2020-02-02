#include <QDebug>
#include <QString>
#include <QList>
#include <QStack>
#include <QMap>
#include <QPointer>
#include <QTextStream>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/SequenceTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "SilentPauseManipulator.h"

struct SilentPauseManipulatorData {
    SilentPauseManipulatorData() {
        levelIDTokens = "tok_mwu";
        levelIDSequences = "syntactic_units";
        attributeAddPauseAfter = "add_pause_after";
    }

    QString levelIDTokens;
    QString levelIDSequences;
    QString attributeAddPauseAfter;
};

SilentPauseManipulator::SilentPauseManipulator() :
    d(new SilentPauseManipulatorData)
{
}

SilentPauseManipulator::~SilentPauseManipulator()
{
    delete d;
}

QString SilentPauseManipulator::process(CorpusCommunication *com)
{
    QString ret;
    if (!com) return ret;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelIDTokens);
            if (!tier_tokens) continue;
            SequenceTier *tier_sequences = tiers->getSequenceTierByName(d->levelIDSequences);
            if (!tier_sequences) continue;

            foreach (Sequence *seq, tier_sequences->sequences()) {
                if (seq->text() == "_") continue;
                if (seq->text() == "DM") continue;
                int start = seq->indexFrom();
                int end = seq->indexTo();
                RealTime timeSilentPauseTotal;
                RealTime timePausesToAdd;
                int numberOfPausesToAdd(0);
                for (int i = start; i <= end; ++i) {
                    Interval *token = tier_tokens->at(i);
                    if (token->isPauseSilent())
                        timeSilentPauseTotal = timeSilentPauseTotal + token->duration();
                    else if (token->attribute(d->attributeAddPauseAfter).toInt() > 0) {
                        timePausesToAdd = timePausesToAdd +
                                RealTime::fromMilliseconds(token->attribute(d->attributeAddPauseAfter).toInt());
                        numberOfPausesToAdd++;
                    }
                }
                // If there are no pauses to manipulate, skip sequence
                if (timeSilentPauseTotal == RealTime()) continue;
                // Correct exaggerations
                if (timePausesToAdd > timeSilentPauseTotal) {
                    RealTime diff = timePausesToAdd - timeSilentPauseTotal;
                    RealTime reduceBy = diff / numberOfPausesToAdd;
                    timePausesToAdd = RealTime();
                    for (int i = start; i <= end; ++i) {
                        Interval *token = tier_tokens->at(i);
                        if (token->attribute(d->attributeAddPauseAfter).toInt() > 0) {
                            int new_pause_ms = token->attribute(d->attributeAddPauseAfter).toInt() - static_cast<int>(reduceBy.toDouble() * 1000);
                            if (new_pause_ms < 0) new_pause_ms = 10;
                            token->setAttribute(d->attributeAddPauseAfter, new_pause_ms);
                            timePausesToAdd = timePausesToAdd + RealTime::fromMilliseconds(new_pause_ms);
                            // ret.append(QString("%1 ").arg(new_pause_ms));
                        }
                    }
                }

                double reductionFactor = (timeSilentPauseTotal - timePausesToAdd).toDouble() / timeSilentPauseTotal.toDouble();
                if (reductionFactor < 0) {
                    ret.append(QString("%1\t%2\t%3-%4\t%5\t%6\t%7\n")
                                       .arg(annotationID).arg(speakerID).arg(start).arg(end)
                                       .arg(timeSilentPauseTotal.toDouble()).arg(timePausesToAdd.toDouble())
                                       .arg(reductionFactor));
                }

                // If there are no manipulations needed, skip sequence
                if (timePausesToAdd == RealTime()) continue;
                for (int i = start; i <= end; ++i) {
                    Interval *token = tier_tokens->at(i);
                    if (token->isPauseSilent()) {
                        RealTime newDuration = token->duration() * reductionFactor;
                        token->setAttribute("pause_manipulation_trim_pos", (token->tMin() + newDuration / 2).toNanoseconds());
                        token->setAttribute("pause_manipulation_trim_len", (token->duration() - newDuration).toNanoseconds());
                        token->setAttribute("pause_manipulation_reduce_to", newDuration.toNanoseconds());
                    }
                }
            }

            // Textgrid corresponding to the manipulation operation
            AnnotationTierGroup *txg_trim = new AnnotationTierGroup();

            IntervalTier *tier_tokens_new = tier_tokens->clone();
            QString soxTrimCommand = QString("sox %1 %2_trimmed.wav trim 0 ")
                    .arg(com->ID() + ".wav").arg(com->ID());
            for (int i = 0; i < tier_tokens->count(); ++i) {
                Interval *token = tier_tokens->at(i);
                if (token->attribute("pause_manipulation_reduce_to").toLongLong() > 0) {
                    RealTime newDuration = RealTime::fromNanoseconds(token->attribute("pause_manipulation_reduce_to").toLongLong());
                    RealTime trimPos = RealTime::fromNanoseconds(token->attribute("pause_manipulation_trim_pos").toLongLong());
                    RealTime trimLen = RealTime::fromNanoseconds(token->attribute("pause_manipulation_trim_len").toLongLong());
                    RealTime trimEnd = trimPos + trimLen;
                    soxTrimCommand.append("=").append(QString::number(trimPos.toDouble(), 'f', 8)).append(" "); // out-point
                    soxTrimCommand.append("=").append(QString::number(trimEnd.toDouble(), 'f', 8)).append(" "); // in-point
                    tier_tokens_new->modifyIntervalDuration(i, newDuration - token->duration());
                }
            }
            QString soxPadCommand = QString("sox %1_trimmed.wav %2.wav pad ")
                    .arg(com->ID()).arg(com->ID() + "_manip");

            txg_trim->addTier(tier_tokens->clone("original"));
            txg_trim->addTier(tier_tokens_new->clone("trimmed"));

            for (int i = 0; i < tier_tokens_new->count(); ++i) {
                Interval *token = tier_tokens_new->at(i);
                if (token->attribute(d->attributeAddPauseAfter).toInt() > 0) {
                    RealTime duration = RealTime::fromMilliseconds(token->attribute(d->attributeAddPauseAfter).toInt());
                    RealTime position = token->tMax();
                    soxPadCommand.append(QString("%1@%2 ")
                                         .arg(QString::number(duration.toDouble(), 'f', 4))
                                         .arg(QString::number(position.toDouble(), 'f', 8)));
                }
            }
            int i = tier_tokens_new->count() - 1;
            while (i >= 0) {
                if (tier_tokens_new->at(i)->attribute(d->attributeAddPauseAfter).toInt() > 0) {
                    RealTime duration = RealTime::fromMilliseconds(tier_tokens_new->at(i)->attribute(d->attributeAddPauseAfter).toInt());
                    Interval *pause = new Interval(RealTime(), duration, "_");
                    tier_tokens_new->insertInterval(i + 1, pause);
                }
                i--;
            }

            txg_trim->addTier(tier_tokens_new->clone("manipulated"));
            PraatTextGrid::save("/home/george/Dropbox/2017 ICIQ3 Granada/Stimuli/test.textgrid", txg_trim);
            delete txg_trim;

            ret.append(soxTrimCommand).append("\n").append(soxPadCommand).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}


