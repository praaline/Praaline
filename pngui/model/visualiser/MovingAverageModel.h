#ifndef MOVINGAVERAGEMODEL_H
#define MOVINGAVERAGEMODEL_H

#include <QPointer>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "pncore/base/RealTime.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/Model.h"

namespace Praaline {
namespace Core {
class AnnotationTierGroup;
}
}

class SparseTimeValueModel;

struct MovingAverageModelData;

class MovingAverageModel : public Model
{
    Q_OBJECT
public:
    MovingAverageModel(sv_samplerate_t sampleRate,
                       Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                       const QString &levelID, const QString &attributeID);
    virtual ~MovingAverageModel();

    virtual std::string getType() const { return "MovingAverageModel"; }

    // Model base class implementation
    virtual bool isOK() const { return true; }
    virtual sv_frame_t getStartFrame() const;
    virtual sv_frame_t getEndFrame() const;
    virtual sv_samplerate_t getSampleRate() const;
    virtual QString getTypeName() const { return tr("Moving Average"); }

    virtual void toXml(QTextStream &out, QString indent = "", QString extraAttributes = "") const;

    // Speakers
    int countSpeakers() const;
    QList<QString> speakers() const;
    void excludeSpeakerIDs(const QList<QString> &list);
    void clearExcludedSpeakerIDs();

    // Model parameters
    QString levelID() const;
    void setLevelID(const QString &);
    QString attributeID() const;
    void setAttributeID(const QString &);
    int smoothingStepMsec() const;
    void setSmoothingStepMsec(int);
    int windowLeftMsec() const;
    void setWindowLeftMsec(int);
    int windowRightMsec() const;
    void setWindowRightMsec(int);
    bool skipSilentPausesCounting() const;
    void setSkipSilentPausesCounting(bool);

    // Returns the calculated (smoothed) moving average model
    QPointer<SparseTimeValueModel> smoothModel(const QString &speakerID);

protected:
    MovingAverageModelData *d;
    void recalculate();
};

#endif // MOVINGAVERAGEMODEL_H
