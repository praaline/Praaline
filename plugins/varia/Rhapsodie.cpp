#include <QSharedPointer>
#include <QString>
#include <QMap>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "Rhapsodie.h"

Rhapsodie::Rhapsodie()
{

}

QString Rhapsodie::updateSyllables(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Open textgrid
        QSharedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
        QString path = "/Users/george/Dropbox/CORPORA/Rhapsodie_files/TextGrids-fev2013/";
        if (!PraatTextGrid::load(path + annotationID + "-Pro.TextGrid" , txg.data())) {
            ret.append("TextGrid not found"); continue;
        }
        // syllable, prom, hes, contour
        IntervalTier *rh_syll = txg->getIntervalTierByName("syllabe");
        if (!rh_syll) { ret.append(annotationID).append("\tNo tier syll"); continue; }
        IntervalTier *rh_prom = txg->getIntervalTierByName("prom");
        if (!rh_prom) { ret.append(annotationID).append("\tNo tier prom"); continue; }
        IntervalTier *rh_hes = txg->getIntervalTierByName("hes");
        if (!rh_hes) { ret.append(annotationID).append("\tNo tier hes"); continue; }
        IntervalTier *rh_contour = txg->getIntervalTierByName("contour");
        if (!rh_contour) { ret.append(annotationID).append("\tNo tier contour"); continue; }
        if (rh_syll->count() != rh_prom->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - prom %2").arg(rh_syll->count()).arg(rh_prom->count())); continue;
        }
        if (rh_syll->count() != rh_hes->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - hes %2").arg(rh_syll->count()).arg(rh_hes->count())); continue;
        }
        if (rh_syll->count() != rh_contour->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - contour %2").arg(rh_syll->count()).arg(rh_contour->count())); continue;
        }

        tiersAll = com->corpus()->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList() << "syll");
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) continue;

            ret.append(annotationID).append("\t").append(speakerID).append(QString("\t%1\n").arg(tier_syll->count()));

        }
        qDeleteAll(tiersAll);
    }
    return ret;
}
