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

#include "IntervalTierCombinations.h"

#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

struct IntervalTierCombinationsData {
    QString intervalsLevelA;
    QString intervalsLevelB;
    QString intervalsLevelCombined;
    QStringList thirdLevelsToCount;
};

IntervalTierCombinations::IntervalTierCombinations() :
    d(new IntervalTierCombinationsData())
{
}

IntervalTierCombinations::~IntervalTierCombinations()
{
    delete d;
}

QString IntervalTierCombinations::intervalsLevelA() const
{
    return d->intervalsLevelA;
}

void IntervalTierCombinations::setIntervalsLevelA(const QString &levelID)
{
    d->intervalsLevelA = levelID;
}

QString IntervalTierCombinations::intervalsLevelB() const
{
    return d->intervalsLevelB;
}

void IntervalTierCombinations::setIntervalsLevelB(const QString &levelID)
{
    d->intervalsLevelB = levelID;
}

QString IntervalTierCombinations::intervalsLevelCombined() const
{
    return d->intervalsLevelCombined;
}

void IntervalTierCombinations::setIntervalsLevelCombined(const QString &levelID)
{
    d->intervalsLevelCombined = levelID;
}

QStringList IntervalTierCombinations::thirdLevelsToCount() const
{
    return d->thirdLevelsToCount;
}

void IntervalTierCombinations::setThirdLevelsToCount(const QStringList &levelIDs)
{
    d->thirdLevelsToCount = levelIDs;
}

// ===============================================================================================================

QString IntervalTierCombinations::combineIntervalTiers(Praaline::Core::CorpusCommunication *com)
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
            IntervalTier *tier_intervalsA = tiers->getIntervalTierByName(d->intervalsLevelA);
            if (!tier_intervalsA) continue;
            IntervalTier *tier_intervalsB = tiers->getIntervalTierByName(d->intervalsLevelB);
            if (!tier_intervalsB) continue;

            QSet<long long> startA, startB, endA, endB;
            QList<long long> startCommon, endCommon;
            foreach (Interval *intv, tier_intervalsA->intervals()) {
                startA << intv->tMin().toNanoseconds();
                endA << intv->tMax().toNanoseconds();
            }
            foreach (Interval *intv, tier_intervalsB->intervals()) {
                startB << intv->tMin().toNanoseconds();
                endB << intv->tMax().toNanoseconds();
            }
            startCommon = startA.intersect(startB).toList();
            std::sort(startCommon.begin(), startCommon.end());
            endCommon = endA.intersect(endB).toList();
            std::sort(endCommon.begin(), endCommon.end());

            QList<Interval *> intervals;
            RealTime previousEnd(-1, 0);
            while (!startCommon.isEmpty() && !endCommon.isEmpty()) {
                RealTime start = RealTime::fromNanoseconds(startCommon.takeFirst());
                while ((start < previousEnd) && !startCommon.isEmpty()) start = RealTime::fromNanoseconds(startCommon.takeFirst());
                RealTime end = RealTime::fromNanoseconds(endCommon.takeFirst());
                while ((end < start) && !endCommon.isEmpty()) end = RealTime::fromNanoseconds(endCommon.takeFirst());
                intervals << new Interval(start, end, "");
                previousEnd = end;
            }

            IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_min");
            foreach (Interval *intv, intervals) {
                // Sequence text               
                intv->setText(tier_tokens->getIntervalsTextContainedIn(intv));
                // Groups
                int countUnitsA(0), countUnitsB(0);
                foreach (Interval *intvA, tier_intervalsA->getIntervalsContainedIn(intv)) {
                    if (!intvA->isPauseSilent()) countUnitsA++;
                }
                foreach (Interval *intvB, tier_intervalsB->getIntervalsContainedIn(intv)) {
                    if (!intvB->isPauseSilent()) countUnitsB++;
                }
                intv->setAttribute(QString("count_%1").arg(d->intervalsLevelA), countUnitsA);
                intv->setAttribute(QString("count_%1").arg(d->intervalsLevelB), countUnitsB);
                QString type;
                if      ((countUnitsA == 1) && (countUnitsB == 1))  type = "1:1";
                else if ((countUnitsA > 1)  && (countUnitsB == 1))  type = "N:1";
                else if ((countUnitsA == 1) && (countUnitsB > 1))   type = "1:M";
                else if ((countUnitsA > 1)  && (countUnitsB > 1))   type = "N:M";
                intv->setAttribute("unit_type", type);
            }

            // Save combined units
            IntervalTier *tier_combined = new IntervalTier(d->intervalsLevelCombined, intervals);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_combined);
            ret.append(QString("OK    %1 %2 Created %3 sequences\n").arg(annotationID).arg(speakerID).arg(intervals.count()));
            delete tier_combined;
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}
