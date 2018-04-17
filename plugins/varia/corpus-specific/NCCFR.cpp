#include <QString>
#include <QMap>
#include <QElapsedTimer>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "pnlib/asr/htk/HTKForcedAligner.h"
using namespace Praaline::ASR;

#include "NCCFR.h"

NCCFR::NCCFR()
{

}

QString NCCFR::prepareTranscription(QPointer<CorpusCommunication> com)
{
    if (!com) return "No Communication";
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << "tok_min");
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                QString t = tok_min->attribute("text_NCCFR").toString().isEmpty() ? tok_min->text() : tok_min->attribute("text_NCCFR").toString();
                tok_min->setAttribute("text_NCCFR", t);
                t = t.replace("*", "").replace("^", "").replace("$", "").replace("&", "");
                if (t.startsWith("(")) t = t.replace("(", "").replace(")", "");
                if (t.contains("(")) t = t.section("(", 0, 0).append("/");
                tok_min->setText(t);
            }
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return com->ID();
}


QString NCCFR::align(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QElapsedTimer timer;

    if (!com) return "No Communication";
    if (com->recordings().isEmpty()) return QString("No Recordings for %1").arg(com->ID());
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return "No Recording";
    QString annotationID = com->ID();

    timer.start();
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->
            getTiersAllSpeakers(annotationID, QStringList() << "transcription" << "tok_min" << "phone");
    foreach (QString speakerID, tiersAll.keys()) {
        AnnotationTierGroup *tiers = tiersAll.value(speakerID);

        IntervalTier *tier_utterance = tiers->getIntervalTierByName("transcription");
        if (!tier_utterance) continue;
        IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
        if (!tier_tok_min) continue;
        IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
        if (!tier_phone) tier_phone = new IntervalTier("phone", tier_utterance->tMin(), tier_utterance->tMax());

        HTKForcedAligner aligner;
        aligner.alignAllUtterances(rec->filePath(), tier_utterance, tier_tok_min, tier_phone);

        AnnotationTierGroup *txg = new AnnotationTierGroup();
        txg->addTier(tier_phone);
        txg->addTier(tier_tok_min);
        txg->addTier(tier_utterance);
        PraatTextGrid::save(QString("/mnt/hgfs/DATA/NCCFR/Segmentation_Textgrids/%1_%2.TextGrid").arg(annotationID).arg(speakerID), txg);
    }
    qDeleteAll(tiersAll);
    return QString("%1\t%2").arg(com->ID()).arg(timer.elapsed());
}
