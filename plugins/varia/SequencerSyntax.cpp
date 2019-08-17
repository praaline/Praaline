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

#include "SequencerSyntax.h"

struct SequencerSyntaxData {
    SequencerSyntaxData() {
        levelIDSyntacticSequences = "syntactic_units";
        levelIDTokens = "tok_mwu";
        attributeIDSyntaxGrouping = "syntax";
        groupOpeningMark = "[";
        groupClosingMark = "]";
    }

    QString levelIDSyntacticSequences;
    QString levelIDTokens;
    QString attributeIDSyntaxGrouping;
    QString groupOpeningMark;
    QString groupClosingMark;
};

SequencerSyntax::SequencerSyntax() :
    d(new SequencerSyntaxData)
{
}

SequencerSyntax::~SequencerSyntax()
{
    delete d;
}


QString SequencerSyntax::checkGroupingAnnotation(CorpusCommunication *com)
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

            QStack<int> stack;
            bool hasErrors(false);
            int tokenIDwhereErrorWasDetected(-1);
            for (int i = 0; i < tier_tokens->count(); ++i) {
                QString syntax = tier_tokens->at(i)->attribute(d->attributeIDSyntaxGrouping).toString().trimmed();
                while (syntax.contains(d->groupOpeningMark)) {
                    stack.push(i);
                    syntax = syntax.remove(syntax.indexOf(d->groupOpeningMark), 1);
                }
                while (syntax.contains(d->groupClosingMark)) {
                    if (stack.isEmpty()) {
                        hasErrors = true;
                        tokenIDwhereErrorWasDetected = i;
                    }
                    else
                        stack.pop();
                    syntax = syntax.remove(syntax.indexOf(d->groupClosingMark), 1);
                }
            }
            if (stack.isEmpty() && !hasErrors) {
                ret.append(QString("OK    %1 %2\n").arg(annotationID).arg(speakerID));
            } else {
                ret.append(QString("ERROR %1 %2 detected on token number %3\n").arg(annotationID).arg(speakerID)
                           .arg(tokenIDwhereErrorWasDetected));
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString SequencerSyntax::createSequencesFromGroupingAnnotation(CorpusCommunication *com)
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

            QList<Sequence *> sequences;
            QStack<int> stack;
            bool hasErrors(false);
            for (int i = 0; i < tier_tokens->count(); ++i) {
                QString syntax = tier_tokens->at(i)->attribute(d->attributeIDSyntaxGrouping).toString().trimmed();
                if (stack.isEmpty() && !syntax.isEmpty() && !syntax.contains(d->groupOpeningMark) && !syntax.contains(d->groupClosingMark)) {
                    sequences << new Sequence(i, i, syntax);
                }
                // else
                while (syntax.contains(d->groupOpeningMark)) {
                    stack.push(i);
                    syntax = syntax.remove(syntax.indexOf(d->groupOpeningMark), 1);
                }
                while (syntax.contains(d->groupClosingMark)) {
                    if (stack.isEmpty()) {
                        hasErrors = true;
                        syntax = syntax.remove(syntax.indexOf(d->groupClosingMark), 1);
                    }
                    else {
                        int start = stack.pop();
                        int pos = syntax.indexOf(d->groupClosingMark);
                        syntax = syntax.remove(pos, 1);
                        QString text = syntax.mid(pos, syntax.indexOf(d->groupClosingMark)).trimmed();
                        Sequence *seq = new Sequence(start, i, text);
                        int depth = stack.size() + 1;
                        seq->setAttribute("depth", depth);
                        sequences << seq;
                    }
                }
            }
            if (stack.isEmpty() && !hasErrors) {
                // Add extra data
                foreach (Sequence *seq, sequences) {
                    // Sequence text
                    QString token_text;
                    for (int i = seq->indexFrom(); i <= seq->indexTo(); ++i) {
                        token_text.append(tier_tokens->at(i)->text()).append(" ");
                    }
                    if (token_text.length() > 1) token_text.chop(1);
                    seq->setAttribute("textSequence", token_text);
                    // Information from extra tiers
                    // Interval *response = tier_response->intervalAtTime(tier_tokens->at(seq->indexFrom())->tCenter());
                    // if (response) seq->setAttribute("response", response->text());
                }
                // Save sequences
                SequenceTier *tier_seq = new SequenceTier(d->levelIDSyntacticSequences, sequences, tier_tokens);
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_seq);
                ret.append(QString("OK    %1 %2 Created %3 sequences\n").arg(annotationID).arg(speakerID).arg(sequences.count()));
                delete tier_seq;
            } else {
                qDeleteAll(sequences);
                ret.append(QString("ERROR %1 %2\n").arg(annotationID).arg(speakerID));
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}


