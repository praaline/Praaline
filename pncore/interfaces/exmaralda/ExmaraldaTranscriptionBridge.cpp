#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include "annotation/AnnotationTier.h"
#include "annotation/PointTier.h"
#include "ExmaraldaBasicTranscription.h"
#include "ExmaraldaTranscriptionBridge.h"

namespace Praaline {
namespace Core {

ExmaraldaTranscriptionBridge::ExmaraldaTranscriptionBridge(QObject *parent) :
    QObject(parent)
{
}

void ExmaraldaTranscriptionBridge::addTier(const QString &tierID, const QString &speakerID,
                                           const QString &type, const QString &category,
                                           const QString &displayName,
                                           AnnotationTier *tierToExport)
{
    if (!tierToExport) return;
    TierToExportInfo info;
    info.tierID = tierID;
    info.speakerID = speakerID;
    info.type = type;
    info.category = category;
    info.displayName = displayName;
    info.tierToExport = tierToExport;
    m_tiersToExport.append(info);
}

void ExmaraldaTranscriptionBridge::exportPraalineToPartitur(ExmaraldaBasicTranscription &partitur)
{
    // Default info in partitur
    partitur.setComment("Created with Praaline");
    // Create common timeline
    QMap<RealTime, QString> timelineLabels;
    foreach (TierToExportInfo tierInfo, m_tiersToExport) {
        IntervalTier *tier = qobject_cast<IntervalTier *>(tierInfo.tierToExport);
        if (!tier) continue;
        foreach (Interval *intv, tier->intervals()) {
            timelineLabels[intv->tMin()] = "T";
        }
        timelineLabels[tier->tMax()] = "T";
    }
    int i = 0;
    foreach(RealTime t, timelineLabels.keys()) {
        ExmaraldaBasicTranscription::TimelineInfo tli;
        QString label = QString("T%1").arg(i);
        tli.id = label;
        tli.t = t;
        partitur.commonTL().append(tli);
        timelineLabels[t] = label;
        i++;
    }
    // Create tiers
    foreach (TierToExportInfo tierInfo, m_tiersToExport) {
        IntervalTier *tier = qobject_cast<IntervalTier *>(tierInfo.tierToExport);
        if (!tier) continue;
        ExmaraldaBasicTranscription::TierInfo tier_exm;
        tier_exm.speaker = tierInfo.speakerID;
        tier_exm.type = tierInfo.type;
        tier_exm.category = tierInfo.category;
        tier_exm.displayName = tierInfo.displayName;
        foreach (Interval *intv, tier->intervals()) {
            if (intv->isPauseSilent()) continue;
            ExmaraldaBasicTranscription::EventInfo event;
            event.start = timelineLabels.value(intv->tMin());
            event.end = timelineLabels.value(intv->tMax());
            event.text = intv->text();
            tier_exm.events.append(event);
        }
        partitur.tiers().insert(tierInfo.tierID, tier_exm);
    }
}

} // namespace Core
} // namespace Praaline
