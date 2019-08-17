#include <QString>
#include <QList>
#include <QPair>
#include <QMap>
#include <QPointer>
#include <QDebug>

#include "MovingAverageModel.h"

#include "pncore/annotation/AnnotationTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/PointTier.h"
using namespace Praaline::Core;

#include "svcore/data/model/SparseTimeValueModel.h"

struct MovingAverageModelData {
    MovingAverageModelData() : sampleRate(0), startFrame(0), endFrame(0)
    {
        step = RealTime::fromMilliseconds(250);
        slidingWindowLeft = RealTime::fromMilliseconds(1000);
        slidingWindowRight = RealTime::fromMilliseconds(1000);
        skipSilentPausesCounting = true;
    }

    sv_samplerate_t sampleRate;
    sv_frame_t startFrame;
    sv_frame_t endFrame;
    RealTime step;
    RealTime slidingWindowLeft;
    RealTime slidingWindowRight;
    QString levelID;
    QString attributeID;
    bool skipSilentPausesCounting;
    SpeakerAnnotationTierGroupMap tiers;    // Speaker ID, corresponding tiers
    QMap<QString, QPointer<SparseTimeValueModel> > smooth;  // speakerID, smoothed data points
    QList<QString> excludedSpeakerIDs;
};

MovingAverageModel::MovingAverageModel(sv_samplerate_t sampleRate,
                                       SpeakerAnnotationTierGroupMap &tiers,
                                       const QString &levelID, const QString &attributeID) :
    d(new MovingAverageModelData)
{
    d->sampleRate = sampleRate;
    d->levelID = levelID;
    d->attributeID = attributeID;
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

MovingAverageModel::~MovingAverageModel()
{
    if (d) qDeleteAll(d->smooth);
    delete d;
}

sv_frame_t MovingAverageModel::getStartFrame() const
{
    return d->startFrame;
}

sv_frame_t MovingAverageModel::getEndFrame() const
{
    return d->endFrame;
}

sv_samplerate_t MovingAverageModel::getSampleRate() const
{
    return d->sampleRate;
}

void MovingAverageModel::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{

}

// ====================================================================================================================
// Speakers
// ====================================================================================================================

int MovingAverageModel::countSpeakers() const
{
    return d->smooth.keys().count();
}

QList<QString> MovingAverageModel::speakers() const
{
    return d->smooth.keys();
}

void MovingAverageModel::excludeSpeakerIDs(const QList<QString> &list)
{
    d->excludedSpeakerIDs.append(list);
}

void MovingAverageModel::clearExcludedSpeakerIDs()
{
    d->excludedSpeakerIDs.clear();
}

// ====================================================================================================================
// Model parameters
// ====================================================================================================================

QString MovingAverageModel::levelID() const
{
    return d->levelID;
}

void MovingAverageModel::setLevelID(const QString &levelID)
{
    if (d->levelID == levelID) return;
    d->levelID = levelID;
    recalculate();
}

QString MovingAverageModel::attributeID() const
{
    return d->attributeID;
}

void MovingAverageModel::setAttributeID(const QString &attributeID)
{
    if (d->attributeID == attributeID) return;
    d->attributeID = attributeID;
    recalculate();
}

int MovingAverageModel::smoothingStepMsec() const
{
    return ((int) (d->step.toDouble() * 1000));
}

void MovingAverageModel::setSmoothingStepMsec(int step)
{
    if (smoothingStepMsec() == step) return;
    d->step = RealTime::fromMilliseconds(step);
    recalculate();
}

int MovingAverageModel::windowLeftMsec() const
{
    return ((int) (d->slidingWindowLeft.toDouble() * 1000));
}

void MovingAverageModel::setWindowLeftMsec(int left)
{
    if (windowLeftMsec() == left) return;
    d->slidingWindowLeft = RealTime::fromMilliseconds(left);
    recalculate();
}

int MovingAverageModel::windowRightMsec() const
{
    return ((int) (d->slidingWindowRight.toDouble() * 1000));
}

void MovingAverageModel::setWindowRightMsec(int right)
{
    if (windowRightMsec() == right) return;
    d->slidingWindowRight = RealTime::fromMilliseconds(right);
    recalculate();
}

bool MovingAverageModel::skipSilentPausesCounting() const
{
    return d->skipSilentPausesCounting;
}

void MovingAverageModel::setSkipSilentPausesCounting(bool skipSilentPausesCounting)
{
    if (d->skipSilentPausesCounting == skipSilentPausesCounting) return;
    d->skipSilentPausesCounting = skipSilentPausesCounting;
    recalculate();
}

// ====================================================================================================================
// Calculcation
// ====================================================================================================================

QPointer<SparseTimeValueModel> MovingAverageModel::smoothModel(const QString &speakerID)
{
    return d->smooth.value(speakerID);
}

void MovingAverageModel::recalculate()
{
    // convert time values to frames in current resolution
    sv_frame_t stepF = RealTime::realTime2Frame(d->step, d->sampleRate);
    sv_frame_t slidingWindowLeftF = RealTime::realTime2Frame(d->slidingWindowLeft, d->sampleRate);
    sv_frame_t slidingWindowRightF = RealTime::realTime2Frame(d->slidingWindowRight, d->sampleRate);

    qDeleteAll(d->smooth);
    foreach (QString speakerID, d->tiers.keys()) {
        AnnotationTier *tier(0);
        if (d->tiers.value(speakerID)) tier = d->tiers.value(speakerID)->tier(d->levelID);
        if (!tier) continue;
        // Tier to be averaged/smoothed exists for this speaker => create smooth model
        if (d->smooth.contains(speakerID) && d->smooth[speakerID])
            d->smooth[speakerID]->clear();
        else
            d->smooth.insert(speakerID, new SparseTimeValueModel(d->sampleRate, 1, false));

        IntervalTier *tier_intv = qobject_cast<IntervalTier *>(tier);
        // PointTier *tier_point = qobject_cast<PointTier *>(tier);

        for (sv_frame_t frame = d->startFrame; frame <= d->endFrame; frame += stepF) {
            sv_frame_t slidingFrame0 = frame - slidingWindowLeftF;
            sv_frame_t slidingFrame1 = frame + slidingWindowRightF;
            int count(0); double value(0);
            if (tier_intv) {
                QPair<int, int> indexes = tier_intv->getIntervalIndexesOverlappingWith(
                            RealTime::frame2RealTime(slidingFrame0, d->sampleRate), RealTime::frame2RealTime(slidingFrame1, d->sampleRate));
                if ((indexes.first >= 0) && (indexes.second >= 0)) {
                    if (!d->skipSilentPausesCounting)
                        count += indexes.second - indexes.first + 1;
                    else
                        for (int i = indexes.first; i <= indexes.second; ++i)
                            if (!tier_intv->at(i)->isPauseSilent()) count++;
                }
            }
            value = ((double) count) / (d->slidingWindowRight + d->slidingWindowLeft).toDouble();
            d->smooth[speakerID]->addPoint(TimeValuePoint(frame, value, ""));
        }

    }
}
