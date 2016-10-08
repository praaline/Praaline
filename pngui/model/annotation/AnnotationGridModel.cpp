#include <QString>
#include <QList>
#include <QPair>
#include <QMap>
#include <QPointer>
#include <QTextStream>
#include <pncore/annotation/annotationtiergroup.h>
#include "AnnotationGridPointModel.h"
#include "AnnotationGridModel.h"

struct AnnotationGridModelData {
    AnnotationGridModelData () : sampleRate(0), startFrame(0), endFrame(0) {}

    sv_samplerate_t sampleRate;
    sv_frame_t startFrame;
    sv_frame_t endFrame;
    QMap<QString, QPointer<AnnotationTierGroup> > tiers;            // Speaker ID, corresponding tiers
    QList<QPair<QString, QString> > attributes;                     // Level ID, Attribute ID
    QMap<QString, QPointer<AnnotationGridPointModel> > boundaries;  // Level ID, corresponding points (all speakers)
    QList<QString> excludedSpeakerIDs;
};

AnnotationGridModel::AnnotationGridModel(sv_samplerate_t sampleRate,
                                         QMap<QString, QPointer<AnnotationTierGroup> > &tiers,
                                         const QList<QPair<QString, QString> > &attributes) :
    d(new AnnotationGridModelData)
{
    d->sampleRate = sampleRate;
    d->tiers = tiers;
    d->attributes = attributes;

    bool firstTier = true;
    foreach (QString speakerID, d->tiers.keys()) {
        foreach (AnnotationTier *tier, d->tiers.value(speakerID)->tiers()) {
            QString levelID = tier->name();
            QPointer<AnnotationGridPointModel> points;
            if (d->boundaries.contains(levelID))
                points = d->boundaries.value(levelID);
            else {
                points = new AnnotationGridPointModel(sampleRate, 1, true);
                d->boundaries.insert(levelID, points);
            }
            sv_frame_t start = RealTime::realTime2Frame(tier->tMin(), sampleRate);
            if (start < d->startFrame || firstTier) d->startFrame = start;
            sv_frame_t end = RealTime::realTime2Frame(tier->tMax(), sampleRate);
            if (end > d->endFrame || firstTier) d->endFrame = end;
            firstTier = false;
            IntervalTier *tier_intv = qobject_cast<IntervalTier *>(tier);
            if (tier_intv) {
                for (int i = 0; i < tier_intv->countItems(); ++i) {
                    Interval *intv = tier_intv->interval(i);
                    points->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(intv->tMin(), sampleRate),
                                                         RealTime::realTime2Frame(intv->duration(), sampleRate),
                                                         speakerID, levelID, i));
                }
            }
            PointTier *tier_point = qobject_cast<PointTier *>(tier);
            if (tier_point) {
                for (int i = 0; i < tier_point->countItems(); ++i) {
                    Point *point = tier_point->point(i);
                    points->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(point->time(), sampleRate), 0,
                                                         speakerID, levelID, i));
                }
            }
        }
    }
}

AnnotationGridModel::~AnnotationGridModel()
{
    if (d) delete d;
}

sv_frame_t AnnotationGridModel::getStartFrame() const
{
    return d->startFrame;
}

sv_frame_t AnnotationGridModel::getEndFrame() const
{
    return d->endFrame;
}

sv_samplerate_t AnnotationGridModel::getSampleRate() const
{
    return d->sampleRate;
}

void AnnotationGridModel::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{

}

int AnnotationGridModel::countSpeakers() const
{
    return d->tiers.keys().count();
}

int AnnotationGridModel::countLevels() const
{
    return d->boundaries.keys().count();
}

int AnnotationGridModel::countLevelsAttributes() const
{
    int count = 0;
    QPair<QString, QString> pairLA;
    foreach (pairLA, d->attributes) {
        if (pairLA.second == "_context") continue;
        ++count;
    }
    return count;
}

int AnnotationGridModel::countAttributesForLevel(const QString &levelID) const
{
    return attributesForLevel(levelID).count();
}

QList<QString> AnnotationGridModel::speakers() const
{
    return d->tiers.keys();
}

QList<QString> AnnotationGridModel::levels() const
{
    return d->boundaries.keys();
}

QList<QString> AnnotationGridModel::attributesForLevel(const QString &levelID) const
{
    QList<QString> ret;
    QPair<QString, QString> item;
    foreach (item, d->attributes) {
        if (item.first == levelID) {
            if (item.second != "_context")
                ret << item.second;
        }
    }
    return ret;
}

void AnnotationGridModel::excludeSpeakerIDs(const QList<QString> &list)
{
    d->excludedSpeakerIDs.append(list);
}

void AnnotationGridModel::clearExcludedSpeakerIDs()
{
    d->excludedSpeakerIDs.clear();
}


QList<AnnotationGridModel::TierTuple> AnnotationGridModel::tierTuples(const AnnotationGridModel::GridLayout layout) const
{
    QList<AnnotationGridModel::TierTuple> ret;
    QPair<QString, QString> pairLA;
    if (layout == AnnotationGridModel::LayoutSpeakersThenLevelAttributes) {
        foreach (QString speakerID, d->tiers.keys()) {
            if (d->excludedSpeakerIDs.contains(speakerID)) continue;
            int i = 0;
            foreach (pairLA, d->attributes) {
                if (pairLA.second == "_context") continue;
                ret << TierTuple(speakerID, pairLA.first, pairLA.second, i);
                ++i;
            }
        }
    } else { // LevelsAttributesThenSpeakers
        int i = 0;
        foreach (pairLA, d->attributes) {
            if (pairLA.second == "_context") continue;
            foreach (QString speakerID, d->tiers.keys()) {
                if (d->excludedSpeakerIDs.contains(speakerID)) continue;
                ret << TierTuple(speakerID, pairLA.first, pairLA.second, i);
            }
            ++i;
        }
    }
    return ret;
}


QPointer<AnnotationGridPointModel> AnnotationGridModel::boundariesForLevel(const QString &levelID)
{
    return d->boundaries.value(levelID);
}

QVariant AnnotationGridModel::data(const QString &speakerID, const QString &levelID, const QString &attributeID, const int index) const
{
    if (!d->tiers.contains(speakerID)) return QVariant();
    QPointer<AnnotationTierGroup> tiers_spk = d->tiers.value(speakerID);
    if (!tiers_spk) return QVariant();
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(levelID);
    if (tier_intv) {
        if (index > 0 && index < tier_intv->count())
            return (attributeID.isEmpty()) ? tier_intv->interval(index)->text() : tier_intv->interval(index)->attribute(attributeID);
        return QVariant();
    }
    PointTier *tier_point = tiers_spk->getPointTierByName(levelID);
    if (tier_point) {
        if (index > 0 && index < tier_point->count())
            return (attributeID.isEmpty()) ? tier_point->point(index)->text() : tier_point->point(index)->attribute(attributeID);
        return QVariant();
    }
    return QVariant();
}

bool AnnotationGridModel::setData(const QString &speakerID, const QString &levelID, const QString &attributeID, const int index, const QVariant &value)
{

}

