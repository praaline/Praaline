#include <QDebug>
#include <QString>
#include <QList>
#include <QStack>
#include <QMap>
#include <QHash>
#include <QPointer>
#include <QTextStream>

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "CPROMDISS.h"

CPROMDISS::CPROMDISS()
{
}

// static
QString CPROMDISS::transferBasicTiers(QPointer<CorpusCommunication> com)
{
    QString ret;
    // return "Disactivated"; // DISACTIVATED
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
            // if (!tier_phone && !annotationID.startsWith("int")) {
            if (annotationID.startsWith("iti") || annotationID.startsWith("lec")) {
                IntervalTier *tier_cphone = tiers->getIntervalTierByName("cprom_phones");
                tier_cphone->setName("phone");
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_cphone);
                ret.append(annotationID + " phone\n");
            }
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            // if (!tier_syll && !annotationID.startsWith("int")) {
            if (annotationID.startsWith("iti") || annotationID.startsWith("lec")) {
                IntervalTier *tier_csyll = tiers->getIntervalTierByName("cprom_syll");
                tier_csyll->setName("syll");
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_csyll);
                ret.append(annotationID + " syll\n");
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

// static
QString CPROMDISS::combineTiers(QPointer<CorpusCommunication> com)
{
    QString tiernameBase;
    QStringList tiernamesToAssociate;
    tiernameBase = "locas_tok_min";
    tiernamesToAssociate << "pos_dismo" << "pos_treetag" << "lex";

    QString ret;
    // return "Disactivated"; // DISACTIVATED
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_base = tiers->getIntervalTierByName(tiernameBase);
            if (!tier_base) continue;
            ret.append(annotationID).append(" associated ").append(tiernameBase).append(" with ");
            foreach (QString tiername, tiernamesToAssociate) {
                IntervalTier *tier = tiers->getIntervalTierByName(tiername);
                if (!tier) continue;
                if (tier->count() != tier_base->count()) continue;
                ret.append(tiername).append(" ");
                for (int i = 0; i < tier_base->count(); ++i) {
                    tier_base->interval(i)->setAttribute(tiername, tier->at(i)->text());
                }
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_base);
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

// static
QString CPROMDISS::movePointAnnotationToInterval(QPointer<CorpusCommunication> com)
{
    QString tiernamePoints;
    QString tiernameIntervals;
    QString attribute;
    tiernamePoints = "boundary";
    tiernameIntervals = "syll";
    attribute = "locas_boundary";

    QString ret;
    return "Disactivated"; // DISACTIVATED
    if (!com) return ret;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            PointTier *tier_points = tiers->getPointTierByName(tiernamePoints);
            if (!tier_points) continue;
            IntervalTier *tier_intervals = tiers->getIntervalTierByName(tiernameIntervals);
            if (!tier_intervals) continue;
            foreach (Interval *intv, tier_intervals->intervals()) {
                intv->setAttribute(attribute, QVariant());
            }
            foreach (Point *p, tier_points->points()) {
                Interval *intv = tier_intervals->intervalAtTime(p->time() - RealTime(0, 100));
                if (intv) intv->setAttribute(attribute, p->text());
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_intervals);
            ret.append(QString("%1\t%2").arg(annotationID).arg(speakerID));
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}


