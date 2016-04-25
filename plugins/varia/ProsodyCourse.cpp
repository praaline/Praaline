#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/interfaces/praat/praattextgrid.h"

#include "ProsodyCourse.h"
#include "TemporalAnalyser.h"

ProsodyCourse::ProsodyCourse()
{
}

void ProsodyCourse::exportMultiTierTextgrids(Corpus *corpus, QPointer<CorpusCommunication> com)
{
    QString path = corpus->basePath();

    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    if (!com) return;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;

            IntervalTier *tier_phone = 0, *tier_syll = 0;
            IntervalTier *tier_tok_min = 0, *tier_tok_mwu = 0, *tier_ortho = 0;
            QStringList tierNamesToExclude; tierNamesToExclude << "words" << "tok-mwu" << "pos-mwu";
            QList<AnnotationTier *> restOfTiers;
            foreach(AnnotationTier *atier, tiers->tiers()) {
                if      (atier->name() == "phone")       tier_phone = qobject_cast<IntervalTier *>(atier);
                else if (atier->name() == "syll")        tier_syll = qobject_cast<IntervalTier *>(atier);
                else if (atier->name() == "tok_min")     tier_tok_min = qobject_cast<IntervalTier *>(atier);
                else if (atier->name() == "tok_mwu")     tier_tok_mwu = qobject_cast<IntervalTier *>(atier);
                else if (atier->name() == "ortho")       tier_ortho = qobject_cast<IntervalTier *>(atier);
                else {
                    restOfTiers << atier;
                }
            }

            QPointer<AnnotationTierGroup> txg = new AnnotationTierGroup();
            if (tier_phone) {
                txg->addTier(new IntervalTier(tier_phone));
            }
            if (tier_syll) {
                txg->addTier(new IntervalTier(tier_syll));
                IntervalTier *tier_promdelivery = new IntervalTier(tier_syll, "prom-delivery");
                foreach (Interval *intv, tier_promdelivery ->intervals()) {
                    intv->setText(intv->attribute("delivery1").toString());
                    if (intv->text().isEmpty())
                        intv->setText(intv->attribute("promise_pos").toString());
                }
                txg->addTier(tier_promdelivery);
            }
            if (tier_tok_min) {\
                txg->addTier(new IntervalTier(tier_tok_min, "tok-min"));
                IntervalTier *tier_pos_min = new IntervalTier(tier_tok_min, "pos-min");
                foreach (Interval *intv, tier_pos_min->intervals())
                    intv->setText(intv->attribute("pos_min").toString());
                txg->addTier(tier_pos_min);
                IntervalTier *tier_disfluencyL1 = new IntervalTier(tier_tok_min, "disfluency-lex");
                foreach (Interval *intv, tier_disfluencyL1->intervals()) {
                    intv->setText(intv->attribute("dis1_lex").toString());
                }
                txg->addTier(tier_disfluencyL1);
                IntervalTier *tier_disfluencyL2 = new IntervalTier(tier_tok_min, "disfluency-rep");
                foreach (Interval *intv, tier_disfluencyL2->intervals()) {
                    intv->setText(intv->attribute("dis1_rep").toString());
                }
                txg->addTier(tier_disfluencyL2);
                IntervalTier *tier_disfluencyL3 = new IntervalTier(tier_tok_min, "disfluency-struct");
                foreach (Interval *intv, tier_disfluencyL3->intervals()) {
                    intv->setText(intv->attribute("dis1_struct").toString());
                }
                txg->addTier(tier_disfluencyL3);
            }
            if (tier_tok_mwu) {
                txg->addTier(new IntervalTier(tier_tok_mwu, "tok-mwu"));
                IntervalTier *tier_pos_mwu = new IntervalTier(tier_tok_mwu, "pos-mwu");
                foreach (Interval *intv, tier_pos_mwu->intervals())
                    intv->setText(intv->attribute("pos_mwu").toString());
                txg->addTier(tier_pos_mwu);
            }
            if (tier_ortho) {
                txg->addTier(new IntervalTier(tier_ortho));
            }
            if (txg->tiersCount() > 0) {
//                    if (tier_phone) {
//                        foreach (AnnotationTier *atier, txg->tiers()) {
//                            IntervalTier *tier = qobject_cast<IntervalTier *>(atier);
//                            if (tier)
//                                tier->fixBoundariesBasedOnTier(tier_phone);
//                        }
//                    }
                // add rest of tiers (not aligned to phones)
                foreach (AnnotationTier *atier, restOfTiers) {
                    if (atier->tierType() == AnnotationTier::TierType_Points)
                        txg->addTier(new PointTier(qobject_cast<PointTier *>(atier)));
                    else if (atier->tierType() == AnnotationTier::TierType_Intervals)
                        txg->addTier(new IntervalTier(qobject_cast<IntervalTier *>(atier)));
                }
                // exclude tiers
                foreach (QString name, tierNamesToExclude) {
                    txg->removeTierByName(name);
                }
                PraatTextGrid::save(path + "/" + annot->ID() + "_disfprom.TextGrid", txg);
            }
        }
        qDeleteAll(tiersAll);
    }
}

void ProsodyCourse::temporalVariables(Corpus *corpus, QPointer<CorpusCommunication> com)
{


//    QString path = corpus->basePath();
//    QPointer<AnnotationTierGroup> txg = new AnnotationTierGroup();

//    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//        if (!annot) continue;
//        QString annotationID = annot->ID();
//        QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
//        foreach (QString speakerID, tiersAll.keys()) {
//            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//            if (!tiers) continue;
//            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//            IntervalTier *tier_tokmin = tiers->getIntervalTierByName("tok_min");
//            txg->addTier(new IntervalTier(tier_syll));
//            txg->addTier(new IntervalTier(tier_tokmin));
//        }
//        qDeleteAll(tiersAll);
//    }

//    txg->addTier(tier_timeline);
//    IntervalTier *tier_timelineT = new IntervalTier(tier_timeline, "timelineT");
//    foreach (Interval *intv, tier_timelineT->intervals())
//        intv->setText(intv->attribute("temporal").toString());
//    txg->addTier(tier_timelineT);

//    PraatTextGrid::save(path + "/" + com->ID() + ".TextGrid", txg);

}


//int countDone = 0;
//madeProgress(0);
//printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: Exporting multi-tier textgrid"));
//printMessage(QString("Exporting %1").arg(com->ID()));
//countDone++;
//madeProgress(countDone * 100 / communications.count());
//}
