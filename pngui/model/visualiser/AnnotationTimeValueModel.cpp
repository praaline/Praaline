#include <QPointer>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "pncore/base/RealTime.h"
#include "pncore/annotation/AnnotationTierGroup.h"

#include "AnnotationTimeValueModel.h"

struct AnnotationTimeValueModelData {
    sv_samplerate_t sampleRate;
    Praaline::Core::SpeakerAnnotationTierGroupMap tiers;
};

AnnotationTimeValueModel::AnnotationTimeValueModel(sv_samplerate_t sampleRate,
                                                   Praaline::Core::SpeakerAnnotationTierGroupMap &tiers) :
    d(new AnnotationTimeValueModelData)
{
    d->sampleRate = sampleRate;
    d->tiers = tiers;
}

AnnotationTimeValueModel::~AnnotationTimeValueModel()
{
    delete d;
}

// Model base class implementation

sv_frame_t AnnotationTimeValueModel::getStartFrame() const
{

}

sv_frame_t AnnotationTimeValueModel::getEndFrame() const
{

}

sv_samplerate_t AnnotationTimeValueModel::getSampleRate() const
{
    return d->sampleRate;
}

void AnnotationTimeValueModel::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{

}

