#ifndef REGIONAVERAGEMODEL_H
#define REGIONAVERAGEMODEL_H

#include <QPointer>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/Model.h"

namespace Praaline {
namespace Core {
class AnnotationTierGroup;
}
}

class RegionModel;

struct RegionAverageModelData;

class RegionAverageModel : public Model
{
    Q_OBJECT
public:
    RegionAverageModel(sv_samplerate_t sampleRate,
                       Praaline::Core::SpeakerAnnotationTierGroupMap &tiers,
                       const QString &levelID, const QString &attributeID,
                       const QString &groupingLevelID);
    virtual ~RegionAverageModel();

    virtual std::string getType() const { return "RegionAverageModel"; }

    // Model base class implementation
    virtual bool isOK() const { return true; }
    virtual sv_frame_t getStartFrame() const;
    virtual sv_frame_t getEndFrame() const;
    virtual sv_samplerate_t getSampleRate() const;
    virtual QString getTypeName() const { return tr("Region Average"); }

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
    QString groupingLevelID() const;
    void setGroupingLevelID(const QString &);
    bool showLabel() const;
    void setShowLabel(bool);

    // Returns the calculated region average model
    QPointer<RegionModel> regionModel(const QString &speakerID);

protected:
    RegionAverageModelData *d;
    void recalculate();
};

#endif // REGIONAVERAGEMODEL_H
