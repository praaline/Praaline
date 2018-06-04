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
#include "pnlib/asr/kaldi/MFAExternal.h"
#include "pnlib/asr/syllabifier/SyllabifierEasy.h"
using namespace  Praaline::ASR;

#include "PFCAligner.h"

struct PFCAlignerData {
    HTKForcedAligner *htk;
    MFAExternal *mfa;
};

PFCAligner::PFCAligner() : d(new PFCAlignerData())
{
    d->htk = new HTKForcedAligner();
    d->mfa = new MFAExternal();
}

PFCAligner::~PFCAligner()
{
    delete d->htk;
    delete d->mfa;
    delete d;
}

QString PFCAligner::align(QPointer<Praaline::Core::CorpusCommunication> com, const QString &method)
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

        if (method == "htk") {
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
        else if (method == "mfa_individual") {
            d->mfa->setOutputPath("/mnt/hgfs/DATA/PFCALIGN/MFA_individual/" + com->ID());
            d->mfa->setSpeakerID(speakerID);
            d->mfa->alignAllUtterances(rec->filePath(), tier_segment, tier_tok_min, tier_phone);
        }
        else if (method == "mfa_regionstyle") {
            QString region = com->ID().left(3);
            QString style = (com->ID().endsWith("t")) ? "text" : "conv";
            d->mfa->setOutputPath("/mnt/hgfs/DATA/PFCALIGN/MFA_region_style/" + QString("%1_%2").arg(region).arg(style));
            d->mfa->setSpeakerID(speakerID);
            d->mfa->alignAllUtterances(rec->filePath(), tier_segment, tier_tok_min, tier_phone);
        }
    }
    qDeleteAll(tiersAll);
    if (method == "htk")
        return QString("%1\t%2").arg(com->ID()).arg(timer.elapsed());
    else if (method == "mfa_individual") {
        return QString("./mfa_train_and_align /mnt/hgfs/DATA/PFCALIGN/MFA/%1 /mnt/hgfs/DATA/PFCALIGN/MFA/%1/%1.dic "
                       "/mnt/hgfs/DATA/PFCALIGN/MFA/%1/align_%1 -o /mnt/hgfs/DATA/PFCALIGN/MFA/%1/model_%1 "
                       "-t /mnt/hgfs/DATA/PFCALIGN/MFA_temp").arg(com->ID());
    }
    else if (method == "mfa_regionstyle")
    {

    }
    return com->ID();
}

void PFCAligner::dictionaryMFAClose(const QString &filename)
{
    d->mfa->finishDictionary(filename);
    d->mfa->startDictionary();
}

void PFCAligner::setMFAPath(const QString &path)
{
    d->mfa->setOutputPath(path);
}

void PFCAligner::setOutputWaveFiles(bool out)
{
    d->mfa->setOutputWaveFiles(out);
}
