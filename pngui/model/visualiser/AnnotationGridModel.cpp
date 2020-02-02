#include <QString>
#include <QList>
#include <QPair>
#include <QMap>
#include <QPointer>
#include <QTextStream>
#include <QDebug>

#include "PraalineCore/Annotation/AnnotationTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/PointTier.h"
using namespace Praaline::Core;

#include "AnnotationGridPointModel.h"
#include "AnnotationGridModel.h"

struct AnnotationGridModelData {
    AnnotationGridModelData () : sampleRate(0), startFrame(0), endFrame(0) {}

    sv_samplerate_t sampleRate;
    sv_frame_t startFrame;
    sv_frame_t endFrame;
    SpeakerAnnotationTierGroupMap tiers;            // Speaker ID, corresponding tiers
    QList<QPair<QString, QString> > attributes;                     // Level ID, Attribute ID
    QMap<QString, QPointer<AnnotationGridPointModel> > boundaries;  // Level ID, corresponding points (all speakers)
    QList<QString> excludedSpeakerIDs;
};

AnnotationGridModel::AnnotationGridModel(sv_samplerate_t sampleRate,
                                         SpeakerAnnotationTierGroupMap &tiers,
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
                for (int i = 0; i < tier_intv->count(); ++i) {
                    Interval *intv = tier_intv->interval(i);
                    points->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(intv->tMin(), sampleRate), speakerID, levelID));
                }
            }
            PointTier *tier_point = qobject_cast<PointTier *>(tier);
            if (tier_point) {
                for (int i = 0; i < tier_point->count(); ++i) {
                    Point *point = tier_point->point(i);
                    points->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(point->time(), sampleRate), speakerID, levelID));
                }
            }
        }
    }
}

AnnotationGridModel::~AnnotationGridModel()
{
    if (d) qDeleteAll(d->boundaries);
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

QVariant AnnotationGridModel::data(const AnnotationGridPointModel::Point &boundary, const QString &attributeID) const
{
    return data(boundary.speakerID, boundary.levelID, boundary.frame, attributeID);
}

QVariant AnnotationGridModel::data(const QString &speakerID, const QString &levelID, const sv_frame_t frame, const QString &attributeID) const
{
    if (!d->tiers.contains(speakerID)) return QVariant();
    AnnotationTierGroup *tiers_spk = d->tiers.value(speakerID);
    if (!tiers_spk) return QVariant();
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(levelID);
    if (tier_intv) {
        Interval *intv = tier_intv->intervalAtTime(RealTime::frame2RealTime(frame + 1, d->sampleRate));
        if (intv) return (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID);
        return QVariant();
    }
    PointTier *tier_point = tiers_spk->getPointTierByName(levelID);
    if (tier_point) {
        Point *point = tier_point->pointAtTime(RealTime::frame2RealTime(frame, d->sampleRate));
        if (point) return (attributeID.isEmpty()) ? point->text() : point->attribute(attributeID);
        return QVariant();
    }
    return QVariant();
}

bool AnnotationGridModel::setData(const AnnotationGridPointModel::Point &boundary, const QString &attributeID, const QVariant &value)
{
    return setData(boundary.speakerID, boundary.levelID, boundary.frame, attributeID, value);
}

bool AnnotationGridModel::setData(const QString &speakerID, const QString &levelID, const sv_frame_t frame, const QString &attributeID, const QVariant &value)
{
    if (!d->tiers.contains(speakerID)) return false;
    AnnotationTierGroup *tiers_spk = d->tiers.value(speakerID);
    if (!tiers_spk) return false;
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(levelID);
    if (tier_intv) {
        Interval *intv = tier_intv->intervalAtTime(RealTime::frame2RealTime(frame + 1, d->sampleRate));
        if (!intv) return false;
        if (attributeID.isEmpty())
            intv->setText(value.toString());
        else
            intv->setAttribute(attributeID, value);
        return true;
    }
    PointTier *tier_point = tiers_spk->getPointTierByName(levelID);
    if (tier_point) {
        Point *point = tier_point->pointAtTime(RealTime::frame2RealTime(frame, d->sampleRate));
        if (!point) return false;
        if (attributeID.isEmpty())
            point->setText(value.toString());
        else
            point->setAttribute(attributeID, value);
        return true;
    }
    return false;
}

RealTime AnnotationGridModel::elementDuration(const AnnotationGridPointModel::Point &boundary) const
{
    if (!d->tiers.contains(boundary.speakerID)) return RealTime();
    AnnotationTierGroup *tiers_spk = d->tiers.value(boundary.speakerID);
    if (!tiers_spk) return RealTime();
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(boundary.levelID);
    if (tier_intv) {
        Interval *intv = tier_intv->intervalAtTime(RealTime::frame2RealTime(boundary.frame + 1, d->sampleRate));
        if (intv)
            return intv->duration();
        else
            return RealTime();
    }
    return RealTime();
}

sv_frame_t AnnotationGridModel::elementMoveLimitFrameLeft(const AnnotationGridPointModel::Point &boundary) const
{
    if (!d->tiers.contains(boundary.speakerID)) return this->getStartFrame();
    AnnotationTierGroup *tiers_spk = d->tiers.value(boundary.speakerID);
    if (!tiers_spk) return this->getStartFrame();
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(boundary.levelID);
    if (tier_intv) {
        int index = tier_intv->intervalIndexAtTime(RealTime::frame2RealTime(boundary.frame + 1, d->sampleRate));
        if (index - 1 > 0 && index - 1 < tier_intv->count())
            return RealTime::realTime2Frame(tier_intv->interval(index - 1)->tMax(), this->getSampleRate());
        else
            return this->getStartFrame();
    }
    return this->getStartFrame();
}

sv_frame_t AnnotationGridModel::elementMoveLimitFrameRight(const AnnotationGridPointModel::Point &boundary) const
{
    if (!d->tiers.contains(boundary.speakerID)) return this->getEndFrame();
    AnnotationTierGroup *tiers_spk = d->tiers.value(boundary.speakerID);
    if (!tiers_spk) return this->getEndFrame();
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(boundary.levelID);
    if (tier_intv) {
        int index = tier_intv->intervalIndexAtTime(RealTime::frame2RealTime(boundary.frame + 1, d->sampleRate));
        if (index + 1 > 0 && index + 1 < tier_intv->count())
            return RealTime::realTime2Frame(tier_intv->interval(index + 1)->tMin(), this->getSampleRate());
        else
            return this->getEndFrame();
    }
    return this->getEndFrame();
}

// ==============================================================================================================================
// Commands
// ==============================================================================================================================

bool AnnotationGridModel::addBoundary(const AnnotationGridPointModel::Point &boundary)
{
    if (!d->tiers.contains(boundary.speakerID)) return false;
    AnnotationTierGroup *tiers_spk = d->tiers.value(boundary.speakerID);
    if (!tiers_spk) return false;
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(boundary.levelID);
    if (tier_intv) {
        int index = tier_intv->intervalIndexAtTime(RealTime::frame2RealTime(boundary.frame, d->sampleRate));
        if ((index < 0) || (index >= tier_intv->count())) return false;
        Interval *split = tier_intv->split(index, RealTime::frame2RealTime(boundary.frame, d->sampleRate));
        if (split) {
            d->boundaries[boundary.levelID]->addPoint(boundary);
            qDebug() << "addBoundary level=" << boundary.levelID << " speaker=" << boundary.speakerID << " frame=" << boundary.frame
                     << " index=" << index << " time=" << RealTime::frame2RealTime(boundary.frame, d->sampleRate).toString();
            return true;
        } else return false;
    }
    return false;
}

bool AnnotationGridModel::deleteBoundary(const AnnotationGridPointModel::Point &boundary)
{
    if (!d->tiers.contains(boundary.speakerID)) return false;
    AnnotationTierGroup *tiers_spk = d->tiers.value(boundary.speakerID);
    if (!tiers_spk) return false;
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(boundary.levelID);
    if (tier_intv) {
        int index = tier_intv->intervalIndexAtTime(RealTime::frame2RealTime(boundary.frame, d->sampleRate));
        if ((index < 0) || (index + 1 >= tier_intv->count())) return false;
        // Merge intervals
        Interval *merged = tier_intv->merge(index, index + 1, " ");
        // Remove the boundary
        if (merged) {
            d->boundaries[boundary.levelID]->deletePoint(boundary);
            qDebug() << "deleteBoundary level=" << boundary.levelID << " speaker=" << boundary.speakerID << " frame=" << boundary.frame
                     << " index=" << index << " time=" << RealTime::frame2RealTime(boundary.frame, d->sampleRate).toString();
            return true;
        } else return false;
    }
    return false;
}

bool AnnotationGridModel::moveBoundary(const AnnotationGridPointModel::Point &oldBoundary, const AnnotationGridPointModel::Point &newBoundary)
{
    if ((oldBoundary.levelID != newBoundary.levelID) || (oldBoundary.speakerID != newBoundary.speakerID)) return false;
    if (!d->tiers.contains(oldBoundary.speakerID)) return false;
    AnnotationTierGroup *tiers_spk = d->tiers.value(oldBoundary.speakerID);
    if (!tiers_spk) return false;
    IntervalTier *tier_intv = tiers_spk->getIntervalTierByName(oldBoundary.levelID);
    if (tier_intv) {
        int index = tier_intv->intervalIndexAtTime(RealTime::frame2RealTime(oldBoundary.frame, d->sampleRate));
        if ((index < 0) || (index >= tier_intv->count())) return false;
        // Move boundary
        tier_intv->moveBoundary(index, RealTime::frame2RealTime(newBoundary.frame, d->sampleRate));
        // Update model
        d->boundaries[oldBoundary.levelID]->deletePoint(oldBoundary);
        d->boundaries[newBoundary.levelID]->addPoint(newBoundary);
        // Debug
        qDebug() << "moveBoundary level=" << newBoundary.levelID << " speaker=" << newBoundary.speakerID << " frame=" << newBoundary.frame
                 << " index=" << index << " time=" << RealTime::frame2RealTime(newBoundary.frame, d->sampleRate).toString();
        return true;
    }
    return false;
}

void AnnotationGridModel::AddBoundaryCommand::execute()
{
    m_model->addBoundary(m_boundary);
}

void AnnotationGridModel::AddBoundaryCommand::unexecute()
{
    m_model->deleteBoundary(m_boundary);
}

void AnnotationGridModel::DeleteBoundaryCommand::execute()
{
    m_model->deleteBoundary(m_boundary);
}

void AnnotationGridModel::DeleteBoundaryCommand::unexecute()
{
    m_model->addBoundary(m_boundary);
}

void AnnotationGridModel::MoveBoundaryCommand::execute()
{
    if (m_model->moveBoundary(m_oldBoundary, m_newBoundary)) std::swap(m_oldBoundary, m_newBoundary);
}

void AnnotationGridModel::MoveBoundaryCommand::unexecute()
{
    execute();
}

void AnnotationGridModel::EditBoundaryCommand::addBoundary(const AnnotationGridPointModel::Point &boundary)
{
    addCommand(new AddBoundaryCommand(m_model, boundary), true);
}

void AnnotationGridModel::EditBoundaryCommand::deleteBoundary(const AnnotationGridPointModel::Point &boundary)
{
    addCommand(new DeleteBoundaryCommand(m_model, boundary), true);
}

void AnnotationGridModel::EditBoundaryCommand::moveBoundary(const AnnotationGridPointModel::Point &boundary, sv_frame_t newFrame)
{
    addCommand(new MoveBoundaryCommand(m_model, boundary, newFrame), true);
}

void AnnotationGridModel::EditBoundaryCommand::addCommand(UndoableCommand *command, bool executeFirst)
{
    if (executeFirst) command->execute();
    if (!m_commands.empty()) {
        DeleteBoundaryCommand *dbc = dynamic_cast<DeleteBoundaryCommand *>(command);
        if (dbc) {
            AddBoundaryCommand *abc = dynamic_cast<AddBoundaryCommand *>(m_commands[m_commands.size() - 1]);
            typename AnnotationGridPointModel::Point::Comparator comparator;
            if (abc) {
                if (!comparator(abc->getBoundary(), dbc->getBoundary()) &&
                    !comparator(dbc->getBoundary(), abc->getBoundary())) {
                    deleteCommand(abc);
                    return;
                }
            }
        }
    }
    UndoableMacroCommand::addCommand(command);
}

AnnotationGridModel::EditBoundaryCommand *AnnotationGridModel::EditBoundaryCommand::finish()
{
    if (!m_commands.empty()) {
        return this;
    } else {
        delete this;
        return 0;
    }
}

