#include <QDebug>
#include <QString>
#include <QList>
#include <QStack>
#include <QMap>
#include <QPointer>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "ProsodicUnitsAnnotator.h"

struct ProsodicUnitsAnnotatorData {
    ProsodicUnitsAnnotatorData() :
        levelTokens("tok_mwu"), levelSyllables("syll"), attributeBoundary("promise_boundary"),
        levelProsodicUnit("prosodic_unit")
    {}

    QString levelTokens;
    QString levelSyllables;
    QString attributeBoundary;
    QString levelProsodicUnit;
};

ProsodicUnitsAnnotator::ProsodicUnitsAnnotator() :
    d(new ProsodicUnitsAnnotatorData())
{
}

ProsodicUnitsAnnotator::~ProsodicUnitsAnnotator()
{
    delete d;
}

QString ProsodicUnitsAnnotator::levelTokens() const
{
    return d->levelTokens;
}

void ProsodicUnitsAnnotator::setLevelTokens(const QString &levelID)
{
    d->levelTokens = levelID;
}

QString ProsodicUnitsAnnotator::levelSyllables() const
{
    return d->levelSyllables;
}

void ProsodicUnitsAnnotator::setLevelSyllables(const QString &levelID)
{
    d->levelSyllables = levelID;
}

QString ProsodicUnitsAnnotator::attributeBoundary() const
{
    return d->attributeBoundary;
}

void ProsodicUnitsAnnotator::setAttributeBoundary(const QString &attributeID)
{
    d->attributeBoundary = attributeID;
}

QString ProsodicUnitsAnnotator::levelProsodicUnit() const
{
    return d->levelProsodicUnit;
}
void ProsodicUnitsAnnotator::setLevelProsodicUnit(const QString &levelID)
{
    d->levelProsodicUnit = levelID;
}


QString ProsodicUnitsAnnotator::createProsodicUnitsTierBoundaries(QPointer<CorpusCommunication> com, const QString &boundaryLabel,
                                                                  bool asSequences)
{
    QString ret;
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_tokens = tiers->getIntervalTierByName(d->levelTokens);
            if (!tier_tokens) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllables);
            if (!tier_syll) continue;

            QList<Sequence *> sequences;
            QList<Interval *> intervals;
            int start(-1); RealTime start_t(-1, 0);
            for (int i = 0; i < tier_tokens->count(); ++i) {
                Interval *token = tier_tokens->at(i);
                if (token->isPauseSilent()) continue;
                if (token->text().startsWith("(") || token->text().endsWith(")")) continue;
                // if (token->text() == "ts" || token->text() == "ps") continue;
                // otherwise
                if (start < 0) start = i;
                if (start_t < RealTime::zeroTime) start_t = tier_tokens->interval(i)->tMin();
                // Check if we have reached the end of a sequence
                QList<Interval *> syllables = tier_syll->getIntervalsOverlappingWith(token);
                if (syllables.isEmpty()) continue;
                Interval *lastSyll = syllables.last();
                if (lastSyll->attribute(d->attributeBoundary).toString() == boundaryLabel) {
                    QString text = QString("PU_%1").arg(boundaryLabel);
                    if (asSequences)
                        sequences << new Sequence(start, i, text);
                    else
                        intervals << new Interval(start_t, lastSyll->tMax(), text);
                    start = -1;
                    start_t = RealTime(-1, 0);
                }
            }
            // Save tier
            if (asSequences) {
                foreach (Sequence *seq, sequences) {
                    // Sequence text
                    QString token_text;
                    for (int i = seq->indexFrom(); i <= seq->indexTo(); ++i) {
                        token_text.append(tier_tokens->at(i)->text()).append(" ");
                    }
                    if (token_text.length() > 1) token_text.chop(1);
                    seq->setAttribute("textSequence", token_text);
                }
                SequenceTier *tier_seq = new SequenceTier(d->levelProsodicUnit, sequences, tier_tokens);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_seq);
                delete tier_seq;
                ret.append(QString("OK    %1 %2 Created %3 sequences of prosodic units (based on %4 boundaries).\n")
                           .arg(annotationID).arg(speakerID).arg(sequences.count()).arg(boundaryLabel));
            }
            else {
                foreach (Interval *intv, intervals) {
                    intv->setText(tier_tokens->getIntervalsTextContainedIn(intv->tMin(), intv->tMax()));
                }
                IntervalTier *tier_intv = new IntervalTier(d->levelProsodicUnit, intervals);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_intv);
                delete tier_intv;
                ret.append(QString("OK    %1 %2 Created %3 intervals of prosodic units (based on %4 boundaries).\n")
                           .arg(annotationID).arg(speakerID).arg(intervals.count()).arg(boundaryLabel));
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

