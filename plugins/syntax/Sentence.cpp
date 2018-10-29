#include <QString>
#include <QPointer>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "Sentence.h"

Sentence::Sentence()
{

}

QString Sentence::exportSentences(QPointer<Praaline::Core::CorpusCommunication> com)
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

            IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
            if (!tier_tok_mwu) continue;
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;

            ret.append(annotationID).append("\t").append(speakerID).append("\t");
            foreach (Interval *tok_mwu, tier_tok_mwu->intervals()) {
                if (tok_mwu->isPauseSilent()) continue;
                if (tok_mwu->attribute("discourse") == "*") {
                    ret.append(tok_mwu->text()).append("\n").append(annotationID).append("\t").append(speakerID).append("\t");
                } else {
                    ret.append(tok_mwu->text()).append(" ");
                }
            }
            ret = ret.trimmed().append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
