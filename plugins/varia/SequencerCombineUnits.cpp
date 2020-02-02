#include <QDebug>
#include <QString>
#include <QList>
#include <QStack>
#include <QSet>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QTextStream>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/SequenceTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "SequencerCombineUnits.h"

struct SequencerCombineUnitsData {
    QString sequencesLevelA;
    QString sequencesLevelB;
    QString sequencesLevelCombined;
};

SequencerCombineUnits::SequencerCombineUnits() :
    d(new SequencerCombineUnitsData())
{
}

SequencerCombineUnits::~SequencerCombineUnits()
{
    delete d;
}

QString SequencerCombineUnits::sequencesLevelA() const
{
    return d->sequencesLevelA;
}

void SequencerCombineUnits::setSequencesLevelA(const QString &levelID)
{
    d->sequencesLevelA = levelID;
}

QString SequencerCombineUnits::sequencesLevelB() const
{
    return d->sequencesLevelB;
}

void SequencerCombineUnits::setSequencesLevelB(const QString &levelID)
{
    d->sequencesLevelB = levelID;
}

QString SequencerCombineUnits::sequencesLevelCombined() const
{
    return d->sequencesLevelCombined;
}

void SequencerCombineUnits::setSequencesLevelCombined(const QString &levelID)
{
    d->sequencesLevelCombined = levelID;
}

QString SequencerCombineUnits::createSequences(Praaline::Core::CorpusCommunication *com)
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
            SequenceTier *tier_sequencesA = tiers->getSequenceTierByName(d->sequencesLevelA);
            if (!tier_sequencesA) continue;
            SequenceTier *tier_sequencesB = tiers->getSequenceTierByName(d->sequencesLevelB);
            if (!tier_sequencesB) continue;

            QMap<int, Sequence *> indexA;
            QMap<int, Sequence *> indexB;
            QSet<int> startA, startB, endA, endB;
            QList<int> startCommon, endCommon;
            foreach (Sequence *seq, tier_sequencesA->sequences()) {
                startA << seq->indexFrom();
                endA << seq->indexTo();
                indexA.insert(seq->indexFrom(), seq);
            }
            foreach (Sequence *seq, tier_sequencesB->sequences()) {
                if (seq->attribute("depth").toInt() <= 1) {
                    startB << seq->indexFrom();
                    endB << seq->indexTo();
                    indexB.insert(seq->indexTo(), seq);
                }
            }
            startCommon = startA.intersect(startB).toList();
            std::sort(startCommon.begin(), startCommon.end());
            endCommon = endA.intersect(endB).toList();
            std::sort(endCommon.begin(), endCommon.end());

            QList<Sequence *> sequences;
            int previousEnd(-1);
            while (!startCommon.isEmpty() && !endCommon.isEmpty()) {
                int start = startCommon.takeFirst();
                while ((start < previousEnd) && !startCommon.isEmpty()) start = startCommon.takeFirst();
                int end = endCommon.takeFirst();
                while ((end < start) && !endCommon.isEmpty()) end = endCommon.takeFirst();
                sequences << new Sequence(start, end, "");
                previousEnd = end;
            }

            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_mwu");
            IntervalTier *tier_response = tiers->getIntervalTierByName("response");
            foreach (Sequence *seq, sequences) {
                // Sequence text
                QString token_text;
                for (int i = seq->indexFrom(); i <= seq->indexTo(); ++i) {
                    token_text.append(tier_tokens->at(i)->text()).append(" ");
                }
                if (token_text.length() > 1) token_text.chop(1);
                seq->setAttribute("textSequence", token_text);
                // Groups
                QString groupA, groupB;
                int countUnitsA(0), countUnitsB(0);
                for (int i = seq->indexFrom(); i <= seq->indexTo(); ++i) {
                    if (indexA.contains(i)) {
                        groupA.append(indexA.value(i)->text()).append(" ");
                        countUnitsA++;
                    }
                    if (indexB.contains(i)) {
                        groupB.append(indexB.value(i)->text()).append(" ");
                        countUnitsB++;
                    }
                }
                groupA = groupA.trimmed();
                groupB = groupB.trimmed();
                seq->setAttribute(d->sequencesLevelA, groupA);
                seq->setAttribute(d->sequencesLevelB, groupB);
                seq->setAttribute(QString("count_%1").arg(d->sequencesLevelA), countUnitsA);
                seq->setAttribute(QString("count_%1").arg(d->sequencesLevelB), countUnitsB);
                // Response
                Interval *responseStart = tier_response->intervalAtTime(tier_tokens->at(seq->indexFrom())->tCenter());
                Interval *responseEnd   = tier_response->intervalAtTime(tier_tokens->at(seq->indexTo())->tCenter());
                QString responseTextStart = (responseStart) ? responseStart->text() : "";
                QString responseTextEnd   = (responseEnd)   ? responseStart->text() : "";
                if (responseTextStart == responseTextEnd)
                    seq->setAttribute("response", responseTextStart);
                else
                    seq->setAttribute("response", responseTextStart + "+" + responseTextEnd);
                // BDU stuff
                if (countUnitsA == 1 && countUnitsB == 1 && (groupB == "euh" || groupB == "md"))
                    seq->setText("md");
                else if (countUnitsA == 1 && countUnitsB == 1)
                    seq->setText("bdu-c");
                else if (countUnitsA > 1 && countUnitsB == 1)
                    seq->setText("bdu-s");
                else if (countUnitsA == 1 && countUnitsB > 1)
                    seq->setText("bdu-i");
                else
                    seq->setText("bdu-x");
            }

            // Save sequences
            SequenceTier *tier_seq = new SequenceTier(d->sequencesLevelCombined, sequences, tier_tokens);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_seq);
            ret.append(QString("OK    %1 %2 Created %3 sequences\n").arg(annotationID).arg(speakerID).arg(sequences.count()));
            delete tier_seq;

//            if (startCommon.count() != endCommon.count()) {
//                ret.append(annotationID).append("\n");
//                QString s;
//                foreach (int i, startCommon) s.append(QString("%1\t").arg(i));
//                ret.append(s).append("\n");
//                s.clear();
//                foreach (int i, endCommon) s.append(QString("%1\t").arg(i));
//                ret.append(s).append("\n\n");
//            }
            // ret.append(QString("%1 %2").arg(startCommon.count()).arg(endCommon.count()));

        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
