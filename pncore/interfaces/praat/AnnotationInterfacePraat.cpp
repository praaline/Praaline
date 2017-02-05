#include <QString>
#include <QList>
#include <QScopedPointer>

#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "AnnotationInterfacePraat.h"

namespace Praaline {
namespace Core {

struct AnnotationInterfacePraatData {
    AnnotationInterfacePraatData() :
        speakerPolicy(AnnotationInterfacePraat::SpeakerPolicy_SinglePerFile),
        exportFilenameTemplate("$CommunicationID_$SpeakerID")
    {}
    QList<AnnotationInterfacePraat::Correspondance> correspondances;
    AnnotationInterfacePraat::SpeakerPolicy speakerPolicy;
    QString exportPath;
    QString exportFilenameTemplate;
};

AnnotationInterfacePraat::AnnotationInterfacePraat(QObject *parent) :
    QObject(parent), d(new AnnotationInterfacePraatData)
{
}

AnnotationInterfacePraat::~AnnotationInterfacePraat()
{
    delete d;
}

const QList<AnnotationInterfacePraat::Correspondance> &AnnotationInterfacePraat::correpondances()
{
    return d->correspondances;
}

void AnnotationInterfacePraat::addCorrepondance(const QString &levelID, const QString &attributeID, const QString &tierName)
{
    for (int i = 0; i < d->correspondances.count(); ++i) {
        if (d->correspondances.at(i).levelID == levelID && d->correspondances.at(i).attributeID == attributeID) {
            d->correspondances[i].tierName = tierName;
            return;
        }
    }
    d->correspondances.append(AnnotationInterfacePraat::Correspondance(levelID, attributeID, tierName));
}

void AnnotationInterfacePraat::removeCorrepondance(const QString &levelID, const QString &attributeID)
{
    bool found(false);
    int i = 0;
    for (i = 0; i < d->correspondances.count(); ++i) {
        if (d->correspondances.at(i).levelID == levelID && d->correspondances.at(i).attributeID == attributeID) {
            found = true; break;
        }
    }
    if (found) {
        d->correspondances.removeAt(i);
    }
}

AnnotationInterfacePraat::SpeakerPolicy AnnotationInterfacePraat::speakerPolicy() const
{
    return d->speakerPolicy;
}

void AnnotationInterfacePraat::setSpeakerPolicy(SpeakerPolicy speakerPolicy)
{
    d->speakerPolicy = speakerPolicy;
}

QString AnnotationInterfacePraat::exportPath() const
{
    return d->exportPath;
}

void AnnotationInterfacePraat::setExportPath(const QString &path)
{
    d->exportPath = path;
}

QString AnnotationInterfacePraat::exportFilenameTemplate() const
{
    return d->exportFilenameTemplate;
}

void AnnotationInterfacePraat::setExportFilenameTemplate(const QString exportFilenameTemplate)
{
    d->exportFilenameTemplate = exportFilenameTemplate;
}

void copyPointTierToTextGrid(AnnotationTierGroup *txg, AnnotationTier *tier, const QString &tierName, const QString &attributeID)
{
    if (!txg || !tier) return;
    PointTier *p_tier = qobject_cast<PointTier *>(tier);
    if (!p_tier) return;
    QList<Point *> points;
    foreach (Point *point, p_tier->points()) {
        points << new Point(point->time(), (attributeID.isEmpty()) ? point->text() : point->attribute(attributeID).toString());
    }
    PointTier *tier_copy = new PointTier(tierName, points);
    txg->addTier(tier_copy);
}

void copyIntervalTierToTextGrid(AnnotationTierGroup *txg, AnnotationTier *tier, const QString &tierName, const QString &attributeID)
{
    if (!txg || !tier) return;
    IntervalTier *i_tier = qobject_cast<IntervalTier *>(tier);
    if (!i_tier) return;
    QList<Interval *> intervals;
    foreach (Interval *intv, i_tier->intervals()) {
        intervals << new Interval(intv->tMin(), intv->tMax(), (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID).toString());
    }
    IntervalTier *tier_copy = new IntervalTier(tierName, intervals);
    txg->addTier(tier_copy);
}

void AnnotationInterfacePraat::copyToTextgrid(AnnotationTierGroup *tiers, AnnotationTierGroup *txg,
                                              const QString &prefixTierNames)
{
    // Copy over tiers depending on correspondances
    foreach (Correspondance correspondance, d->correspondances) {
        AnnotationTier *tier = tiers->tier(correspondance.levelID);
        QString tierName = prefixTierNames + correspondance.tierName;
        if (!tier) continue;
        if      (tier->tierType() == AnnotationTier::TierType_Points)
            copyPointTierToTextGrid(txg, tier, tierName, correspondance.attributeID);
        else if (tier->tierType() == AnnotationTier::TierType_Intervals || tier->tierType() == AnnotationTier::TierType_Grouping)
            copyIntervalTierToTextGrid(txg, tier, tierName, correspondance.attributeID);
        // else build conversion algorithms for the other types of tiers
    }
}

bool AnnotationInterfacePraat::exportAnnotation(CorpusRepository *repository, const QString &annotationID)
{
    if (!repository) return false;
    if (!repository->annotations()) return false;
    if (!repository->annotationStructure()) return false;

    QString filename = d->exportFilenameTemplate;

    // All levels that need to be retrieved from the database
    QStringList levelIDs;
    foreach (Correspondance correspondance, d->correspondances)
        if (!levelIDs.contains(correspondance.levelID)) levelIDs << correspondance.levelID;

    // Get data for all speakers
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = repository->annotations()->getTiersAllSpeakers(annotationID, levelIDs);

    bool result = true;
    if (d->speakerPolicy == SpeakerPolicy_SinglePerFile) {
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            QScopedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup(this)); // the TextGrid
            copyToTextgrid(tiers, txg.data());
            if (txg->tiersCount() > 0) {
                result = result && PraatTextGrid::save(d->exportPath + "/" + QString(filename).replace("$SpeakerID", speakerID), txg.data());
            }
        }
    }
    else if (d->speakerPolicy == SpeakerPolicy_PrefixTierNames) {
        QScopedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup(this)); // the TextGrid
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            copyToTextgrid(tiers, txg.data(), QString("%1-").arg(speakerID));
        }
        if (txg->tiersCount() > 0) {
            result = result && PraatTextGrid::save(d->exportPath + "/" + filename, txg.data());
        }
    }
    qDeleteAll(tiersAll);

    return result;
}


} // namespace Core
} // namespace Praaline

