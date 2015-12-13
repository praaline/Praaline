#ifndef PROSOGRAMMODEL_H
#define PROSOGRAMMODEL_H

#include <QPointer>
#include <QMap>
#include <QList>
#include <QPair>
#include <QStringList>

#include "pncore/base/RealTime.h"
#include "svcore/base/XmlExportable.h"
#include "svcore/data/model/Model.h"
#include "AnnotationGridPointModel.h"
#include "ProsogramTonalSegmentModel.h"

struct ProsogramModelData;
class CorpusRecording;
class SparseTimeValueModel;

class ProsogramModel : public Model
{
    Q_OBJECT

public:
    ProsogramModel(sv_samplerate_t sampleRate, QPointer<CorpusRecording> rec);
    virtual ~ProsogramModel();

    virtual std::string getType() const { return "ProsogramModel"; }

    // Model base class implementation
    virtual bool isOK() const { return true; }
    virtual sv_frame_t getStartFrame() const;
    virtual sv_frame_t getEndFrame() const;
    virtual sv_samplerate_t getSampleRate() const;
    virtual QString getTypeName() const { return tr("Prosogram"); }

    virtual void toXml(QTextStream &out, QString indent = "", QString extraAttributes = "") const;

    QPointer<ProsogramTonalSegmentModel> segmentModel();
    QPointer<AnnotationGridPointModel> phoneModel();
    QPointer<AnnotationGridPointModel> syllModel();
    QPointer<AnnotationGridPointModel> vuvRegionModel();

    QPointer<SparseTimeValueModel> pitchModel();
    QPointer<SparseTimeValueModel> intensityModel();

protected:
    ProsogramModelData *d;

    bool readProsogramFiles(sv_samplerate_t sampleRate, const QString &speakerID,
                            QPointer<ProsogramTonalSegmentModel> segments,
                            const QString &filenameNuclei, const QString &filenameStylPitchTier,
                            const QString &filenamePitchTier, const QString &filenameIntensityTier);
};

#endif // PROSOGRAMMODEL_H
