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

SequencerSyntax::SequencerSyntax()
{

}


QString SequencerSyntax::checkGroupingAnnotation(QPointer<CorpusCommunication> com)
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
            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tokens) continue;

            QStack<int> stack;
            bool hasErrors(false);
            int tokenIDwhereErrorWasDetected(-1);
            for (int i = 0; i < tier_tokens->count(); ++i) {
                QString syntax = tier_tokens->at(i)->attribute("syntaxe").toString().trimmed();
                while (syntax.contains("(")) {
                    stack.push(i);
                    syntax = syntax.remove(syntax.indexOf("("), 1);
                }
                while (syntax.contains(")")) {
                    if (stack.isEmpty()) {
                        hasErrors = true;
                        tokenIDwhereErrorWasDetected = i;
                    }
                    else
                        stack.pop();
                    syntax = syntax.remove(syntax.indexOf(")"), 1);
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

QString SequencerSyntax::createSequencesFromGroupingAnnotation(QPointer<CorpusCommunication> com)
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
            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tokens) continue;

            QList<Sequence *> sequences;
            QStack<int> stack;
            bool hasErrors(false);
            for (int i = 0; i < tier_tokens->count(); ++i) {
                QString syntax = tier_tokens->at(i)->attribute("syntaxe").toString().trimmed();
                if (stack.isEmpty() && !syntax.isEmpty() && !syntax.contains("(") && !syntax.contains(")")) {
                    sequences << new Sequence(i, i, syntax);
                }
                // else
                while (syntax.contains("(")) {
                    stack.push(i);
                    syntax = syntax.remove(syntax.indexOf("("), 1);
                }
                while (syntax.contains(")")) {
                    if (stack.isEmpty()) {
                        hasErrors = true;
                        syntax = syntax.remove(syntax.indexOf(")"), 1);
                    }
                    else {
                        int start = stack.pop();
                        int pos = syntax.indexOf(")");
                        syntax = syntax.remove(pos, 1);
                        QString text = syntax.mid(pos, syntax.indexOf(")")).trimmed();
                        sequences << new Sequence(start, i, text);
                    }
                }
            }
            if (stack.isEmpty() && !hasErrors) {
                SequenceTier *tier_seq = new SequenceTier("syntax", sequences, tier_tokens);
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














