#ifndef MOVINGAVERAGEMODEL_H
#define MOVINGAVERAGEMODEL_H

#include <QPointer>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "pncore/base/RealTime.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/Model.h"

namespace Praaline {
namespace Core {
class CorpusRecording;
}
}

class SparseTimeValueModel;

struct MovingAverageModelData;

class MovingAverageModel : public Model
{
    Q_OBJECT
public:
    MovingAverageModel(sv_samplerate_t sampleRate);
    virtual ~MovingAverageModel();

    virtual std::string getType() const { return "MovingAverageModel"; }

    // Model base class implementation
    virtual bool isOK() const { return true; }
    virtual sv_frame_t getStartFrame() const;
    virtual sv_frame_t getEndFrame() const;
    virtual sv_samplerate_t getSampleRate() const;
    virtual QString getTypeName() const { return tr("Moving Average"); }

    virtual void toXml(QTextStream &out, QString indent = "", QString extraAttributes = "") const;

    QPointer<SparseTimeValueModel> smoothModel();

protected:
    MovingAverageModelData *d;
};

#endif // MOVINGAVERAGEMODEL_H
