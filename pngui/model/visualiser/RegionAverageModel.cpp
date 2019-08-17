#include <QString>
#include <QList>
#include <QPair>
#include <QMap>
#include <QPointer>
#include <QDebug>

#include "RegionAverageModel.h"

#include "pncore/annotation/AnnotationTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/PointTier.h"
using namespace Praaline::Core;

#include "svcore/data/model/RegionModel.h"

struct RegionAverageModelData {
    RegionAverageModelData() :
        sampleRate(0), startFrame(0), endFrame(0), showLabel(false),
        skipSilentPausesGrouping(true), skipSilentPausesCounting(true)
    { }

    sv_samplerate_t sampleRate;
    sv_frame_t startFrame;
    sv_frame_t endFrame;
    QString levelID;
    QString attributeID;
    QString groupingLevelID;
    bool showLabel;
    bool skipSilentPausesGrouping;
    bool skipSilentPausesCounting;
    SpeakerAnnotationTierGroupMap tiers;    // Speaker ID, corresponding tiers
    QMap<QString, QPointer<RegionModel> > regions;          // SpeakerID, calculated averages
    QList<QString> excludedSpeakerIDs;
};

RegionAverageModel::RegionAverageModel(sv_samplerate_t sampleRate,
                                       SpeakerAnnotationTierGroupMap &tiers,
                                       const QString &levelID, const QString &attributeID,
                                       const QString &groupingLevelID) :
    d(new RegionAverageModelData)
{
    d->sampleRate = sampleRate;
    d->levelID = levelID;
    d->attributeID = attributeID;
    d->groupingLevelID = groupingLevelID;
    d->tiers = tiers;

    bool firstTier = true;
    foreach (QString speakerID, d->tiers.keys()) {
        foreach (AnnotationTier *tier, d->tiers.value(speakerID)->tiers()) {
            sv_frame_t start = RealTime::realTime2Frame(tier->tMin(), sampleRate);
            if (start < d->startFrame || firstTier) d->startFrame = start;
            sv_frame_t end = RealTime::realTime2Frame(tier->tMax(), sampleRate);
            if (end > d->endFrame || firstTier) d->endFrame = end;
            firstTier = false;
        }
    }
    recalculate();
}

RegionAverageModel::~RegionAverageModel()
{
    if (d) qDeleteAll(d->regions);
    delete d;
}

sv_frame_t RegionAverageModel::getStartFrame() const
{
    return d->startFrame;
}

sv_frame_t RegionAverageModel::getEndFrame() const
{
    return d->endFrame;
}

sv_samplerate_t RegionAverageModel::getSampleRate() const
{
    return d->sampleRate;
}

void RegionAverageModel::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{

}

// ====================================================================================================================
// Speakers
// ====================================================================================================================

int RegionAverageModel::countSpeakers() const
{
    return d->regions.keys().count();
}

QList<QString> RegionAverageModel::speakers() const
{
    return d->regions.keys();
}

void RegionAverageModel::excludeSpeakerIDs(const QList<QString> &list)
{
    d->excludedSpeakerIDs.append(list);
}

void RegionAverageModel::clearExcludedSpeakerIDs()
{
    d->excludedSpeakerIDs.clear();
}

// ====================================================================================================================
// Model parameters
// ====================================================================================================================

QString RegionAverageModel::levelID() const
{
    return d->levelID;
}

void RegionAverageModel::setLevelID(const QString &levelID)
{
    if (d->levelID == levelID) return;
    d->levelID = levelID;
    recalculate();
}

QString RegionAverageModel::attributeID() const
{
    return d->attributeID;
}

void RegionAverageModel::setAttributeID(const QString &attributeID)
{
    if (d->attributeID == attributeID) return;
    d->attributeID = attributeID;
    recalculate();
}

QString RegionAverageModel::groupingLevelID() const
{
    return d->groupingLevelID;
}

void RegionAverageModel::setGroupingLevelID(const QString &groupingLevelID)
{
    if (d->groupingLevelID == groupingLevelID) return;
    d->groupingLevelID = groupingLevelID;
    recalculate();
}

bool RegionAverageModel::showLabel() const
{
    return d->showLabel;
}

void RegionAverageModel::setShowLabel(bool showLabel)
{
    if (d->showLabel == showLabel) return;
    d->showLabel = showLabel;
    recalculate();
}

// ====================================================================================================================
// Calculcation
// ====================================================================================================================

QPointer<RegionModel> RegionAverageModel::regionModel(const QString &speakerID)
{
    return d->regions.value(speakerID);
}

void RegionAverageModel::recalculate()
{
    qDeleteAll(d->regions);
    foreach (QString speakerID, d->tiers.keys()) {
        AnnotationTier *tier(0);
        AnnotationTier *tierGrouping(0);
        if (d->tiers.value(speakerID)) tier = d->tiers.value(speakerID)->tier(d->levelID);
        if (d->tiers.value(speakerID)) tierGrouping = d->tiers.value(speakerID)->tier(d->groupingLevelID);
        if ((!tier) || (!tierGrouping)) continue;
        // Tier to be averaged and tier to group exist for this speaker => create regions model
        if (d->regions.contains(speakerID) && d->regions[speakerID])
            d->regions[speakerID]->clear();
        else
            d->regions.insert(speakerID, new RegionModel(d->sampleRate, 1, false));

        IntervalTier *tier_intv = qobject_cast<IntervalTier *>(tier);
        IntervalTier *tierGrouping_intv = qobject_cast<IntervalTier *>(tierGrouping);
        // PointTier *tier_point = qobject_cast<PointTier *>(tier);

        if (tierGrouping_intv) {
            foreach (Interval *groupingIntv, tierGrouping_intv->intervals()) {
                if (d->skipSilentPausesGrouping && groupingIntv->isPauseSilent()) continue;
                int count(0); double value(0);
                if (tier_intv) {
                    QPair<int, int> indexes = tier_intv->getIntervalIndexesOverlappingWith(groupingIntv);
                    if ((indexes.first >= 0) && (indexes.second >= 0)) {
                        if (!d->skipSilentPausesCounting)
                            count += indexes.second - indexes.first + 1;
                        else
                            for (int i = indexes.first; i <= indexes.second; ++i)
                                if (!tier_intv->at(i)->isPauseSilent()) count++;
                    }
                }
                value = ((double) count) / (groupingIntv->duration()).toDouble();
                sv_frame_t frame = RealTime::realTime2Frame(groupingIntv->tMin(), d->sampleRate);
                sv_frame_t duration = RealTime::realTime2Frame(groupingIntv->duration(), d->sampleRate);
                QString label;
                if (d->showLabel) label = QString::number(value);
                d->regions[speakerID]->addPoint(RegionRec(frame, value, duration, label));
            }
        }

    }
}
