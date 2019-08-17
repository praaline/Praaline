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
    QStringList regions;
    HTKForcedAligner *htk;
    MFAExternal *mfa;
};

PFCAligner::PFCAligner() : d(new PFCAlignerData())
{
    d->regions << "11a" << "12a" << "13a" << "13b" << "21a" << "31a" << "38a" << "42a" << "44a" << "50a"
               << "54b" << "61a" << "64a" << "69a" << "75c" << "75x" << "81a" << "85a" << "92a" << "974"
               << "aba" << "aca" << "bfa" << "bga" << "bla" << "bta" << "caa" << "cia" << "cqa" << "cqb"
               << "cya" << "maa" << "rca" << "sca" << "sga" << "sna" << "sva";
    d->htk = new HTKForcedAligner();
    d->mfa = new MFAExternal();
}

PFCAligner::~PFCAligner()
{
    delete d->htk;
    delete d->mfa;
    delete d;
}

QString PFCAligner::align(Praaline::Core::CorpusCommunication *com, const QString &method)
{
    QElapsedTimer timer;

    if (!com) return "No Communication";
    if (com->recordings().isEmpty()) return QString("No Recordings for %1").arg(com->ID());
    CorpusRecording *rec = com->recordings().first();
    if (!rec) return "No Recording";
    QString annotationID = com->ID();

    timer.start();
    SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->
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

QString PFCAligner::scriptCrossAlignment()
{
    QString ret;
    foreach (QString regionToAlign, d->regions) {
        foreach (QString regionModel, d->regions) {
            if (regionToAlign == regionModel) continue;
            ret.append(QString("./mfa_align /home/george/pfc/%1_text /home/george/pfc/%1_text/%1_text.dic "
                       "/home/george/rs_model/model_%2_text.zip /home/george/rs_align/%1_text_%2_text "
                       "-t /home/george/rs_temp/%1_text_%2_text -c -j 7 -i\n").arg(regionToAlign).arg(regionModel));
        }
    }
    return ret;
}

QString PFCAligner::combineDictionaries()
{
    QString ret = "OK";
    QMap<QString, int> combined;
    foreach (QString region, d->regions) {
        QFile fileRegionDic(QString("/mnt/hgfs/DATA/PFCALIGN/MFA_region_style/text_dictionaries/%1_text.dic").arg(region));
        if ( !fileRegionDic.open( QIODevice::ReadOnly | QIODevice::Text ) ) continue;
        QTextStream dic(&fileRegionDic);
        dic.setCodec("UTF-8");
        while (!dic.atEnd()) {
            QString line = dic.readLine();
            if (line.isEmpty()) continue;
            if (combined.contains(line)) continue;
            combined.insert(line, 1);
        }
        fileRegionDic.close();
    }
    QFile fileOut("/mnt/hgfs/DATA/PFCALIGN/MFA_region_style/text_dictionaries/all_text.dic");
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return "Error";
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString string, combined.keys()) {
        out << string;
        if (!string.endsWith("\n")) out << "\n";
    }
    fileOut.close();
    return ret;
}
