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

#include "SequencerProsodicUnits.h"

struct SequencerProsodicUnitsData {
    SequencerProsodicUnitsData() :
        levelTokens("tok_mwu"), levelSyllables("syll"), attributeBoundary("boundary")
    {}

    QString levelTokens;
    QString levelSyllables;
    QString attributeBoundary;
};

SequencerProsodicUnits::SequencerProsodicUnits() :
    d(new SequencerProsodicUnitsData())
{
}

SequencerProsodicUnits::~SequencerProsodicUnits()
{
    delete d;
}

QString SequencerProsodicUnits::levelTokens() const
{
    return d->levelTokens;
}

void SequencerProsodicUnits::setLevelTokens(const QString &levelID)
{
    d->levelTokens = levelID;
}

QString SequencerProsodicUnits::levelSyllables() const
{
    return d->levelSyllables;
}

void SequencerProsodicUnits::setLevelSyllables(const QString &levelID)
{
    d->levelSyllables = levelID;
}

QString SequencerProsodicUnits::attributeBoundary() const
{
    return d->attributeBoundary;
}

void SequencerProsodicUnits::setAttributeBoundary(const QString &attributeID)
{
    d->attributeBoundary = attributeID;
}

QString SequencerProsodicUnits::createSequencesFromProsodicBoundaries(QPointer<CorpusCommunication> com, const QString &boundaryLabel)
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
            IntervalTier *tier_response = tiers->getIntervalTierByName("response");

            QList<Sequence *> sequences;
            int start(-1);
            for (int i = 0; i < tier_tokens->count(); ++i) {
                Interval *token = tier_tokens->at(i);
                if (token->isPauseSilent()) continue;
                if (token->text().startsWith("(") || token->text().endsWith(")")) continue;
                if (token->text() == "ts" || token->text() == "ps") continue;
                // otherwise
                if (start < 0) start = i;
                // check to see if end of a sequence
                QList<Interval *> syllables = tier_syll->getIntervalsOverlappingWith(token);
                if (syllables.isEmpty()) continue;
                Interval lastSyll = syllables.last();
                if (lastSyll.attribute(d->attributeBoundary).toString() == boundaryLabel) {
                    sequences << new Sequence(start, i, QString("PU_%1").arg(boundaryLabel));
                    start = -1;
                }
            }
            // Add extra data
            foreach (Sequence *seq, sequences) {
                // Sequence text
                QString token_text;
                for (int i = seq->indexFrom(); i <= seq->indexTo(); ++i) {
                    token_text.append(tier_tokens->at(i)->text()).append(" ");
                }
                if (token_text.length() > 1) token_text.chop(1);
                seq->setAttribute("textSequence", token_text);
                // Response
                Interval *response = tier_response->intervalAtTime(tier_tokens->at(seq->indexFrom())->tCenter());
                if (response) seq->setAttribute("response", response->text());
            }
            // Save sequence tier
            SequenceTier *tier_seq = new SequenceTier("prosodic_units", sequences, tier_tokens);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_seq);
            ret.append(QString("OK    %1 %2 Created %3 sequences of prosodic units (based on %4 boundaries).\n")
                       .arg(annotationID).arg(speakerID).arg(sequences.count()).arg(boundaryLabel));
            delete tier_seq;
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

