#include <QDebug>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "prosodicboundaries.h"
#include "ProsodyCourse.h"

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




void ProsodyCourse::syllableTables(Corpus *corpus)
{
    if (!corpus) return;
    QFile file("/home/george/Dropbox/Prosody2016_Syllables.txt");
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    QStringList extraAttributes;
    extraAttributes << "promise_pos" << "promise" << "delivery" << "f0_mean";
    out << "communicationID\tspeakerID\tsyll_ID\tsyll_tmin\tsyll\t"
           "expertBoundaryType\texpertContour\texpertBoundary\t"
           "durNextPause\tlogdurNextPause\tlogdurNextPauseZ\t"
           "durSyllRel20\tdurSyllRel30\tdurSyllRel40\tdurSyllRel50\t"
           "logdurSyllRel20\tlogdurSyllRel30\tlogdurSyllRel40\tlogdurSyllRel50\t"
           "f0meanSyllRel20\tf0meanSyllRel30\tf0meanSyllRel40\tf0meanSyllRel50\t"
           "intrasyllab_up\tintrasyllab_down\ttrajectory\t"
           "tok_mwu\tsequence\trection\tsyntacticBoundaryType\tpos_mwu\tpos_mwu_cat\tpos_clilex\t"
           "promise_pos\tpromise\tdelivery2\tf0_mean\n";
    foreach (CorpusCommunication *com, corpus->communications()) {
        foreach (QString annotationID, com->annotationIDs()) {
            QMap<QString, QPointer<AnnotationTierGroup> > tiers = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiers.keys()) {
                QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
                IntervalTier *tier_syll = tiersSpk->getIntervalTierByName("syll");
                if (!tier_syll) continue;
                QList<int> syllables;
                for (int i = 0; i < tier_syll->countItems(); ++i) { syllables << i; }
                if (syllables.isEmpty()) continue;
                QStringList results = ProsodicBoundaries::analyseBoundaryListToStrings(corpus, annotationID, syllables, extraAttributes);
                foreach (QString line, results) out << line.replace(",", ".") << "\n";
            }
            qDeleteAll(tiers);
            qDebug() << com->ID();
        }
    }
    file.close();
}


//int countDone = 0;
//madeProgress(0);
//printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: Exporting multi-tier textgrid"));
//printMessage(QString("Exporting %1").arg(com->ID()));
//countDone++;
//madeProgress(countDone * 100 / communications.count());
//}
