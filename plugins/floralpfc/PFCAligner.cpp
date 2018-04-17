#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>
#include <QDebug>
#include <QElapsedTimer>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "pnlib/asr/htk/HTKForcedAligner.h"
#include "pnlib/asr/syllabifier/SyllabifierEasy.h"
using namespace  Praaline::ASR;

#include "PFCAligner.h"

struct PFCAlignerData {
    HTKForcedAligner *htk;
};

PFCAligner::PFCAligner() : d(new PFCAlignerData())
{
    d->htk = new HTKForcedAligner();
}

PFCAligner::~PFCAligner()
{
    delete d->htk;
    delete d;
}

QString PFCAligner::align(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QElapsedTimer timer;

    if (!com) return "No Communication";
    if (com->recordings().isEmpty()) return QString("No Recordings for %1").arg(com->ID());
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return "No Recording";
    QString annotationID = com->ID();

    timer.start();
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->
            getTiersAllSpeakers(annotationID, QStringList() << "segment" << "tok_min" << "phone");
    foreach (QString speakerID, tiersAll.keys()) {
        AnnotationTierGroup *tiers = tiersAll.value(speakerID);

        IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
        if (!tier_segment) continue;
        IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
        if (!tier_tok_min) continue;
        IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
        if (!tier_phone) tier_phone = new IntervalTier("phone", tier_segment->tMin(), tier_segment->tMax());

        int i(0);
        int indexUtterance = tier_segment->count() - 1;
        while (indexUtterance >= 0) {
            if (tier_segment->at(indexUtterance)->isPauseSilent()) { indexUtterance--; continue; }
            QString alignerOutput;
            bool result(false);
            result = d->htk->alignUtterance(rec->filePath(), tier_segment, indexUtterance, tier_tok_min, tier_phone, alignerOutput);
            i++;
            if (!result) qDebug() << result << tier_segment->at(indexUtterance)->text() << alignerOutput;
            indexUtterance--;
        }
        tier_tok_min->mergeIdenticalAnnotations("_");
        tier_segment->mergeIdenticalAnnotations("_");
        tier_phone->fillEmptyWith("", "_");
        tier_phone->mergeIdenticalAnnotations("_");

        AnnotationTierGroup *txg = new AnnotationTierGroup();
        txg->addTier(tier_phone);
        txg->addTier(tier_tok_min);
        txg->addTier(tier_segment);
        PraatTextGrid::save(QString("/mnt/hgfs/DATA/PFCALIGN/Segmentation_textgrids/%1_%2.TextGrid").arg(annotationID).arg(speakerID), txg);
    }
    qDeleteAll(tiersAll);
    return QString("%1\t%2").arg(com->ID()).arg(timer.elapsed());
}
