#include "MovingAverageModel.h"

#include "svcore/data/model/SparseTimeValueModel.h"

struct MovingAverageModelData {
    MovingAverageModelData() : sampleRate(0)
    {}

    sv_samplerate_t sampleRate;
    QPointer<SparseTimeValueModel> smooth;
};

MovingAverageModel::MovingAverageModel(sv_samplerate_t sampleRate) :
    d(new MovingAverageModelData)
{
    d->sampleRate = sampleRate;
    d->smooth = new SparseTimeValueModel(sampleRate, 1, false);
}

MovingAverageModel::~MovingAverageModel()
{
    delete d;
}

sv_frame_t MovingAverageModel::getStartFrame() const
{
    return d->smooth->getStartFrame();
}

sv_frame_t MovingAverageModel::getEndFrame() const
{
    return d->smooth->getEndFrame();
}

sv_samplerate_t MovingAverageModel::getSampleRate() const
{
    return d->sampleRate;
}

void MovingAverageModel::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{

}

QPointer<SparseTimeValueModel> MovingAverageModel::smoothModel()
{
    return d->smooth;
}
