#ifndef PROSODICBOUNDARIESANNOTATOR_H
#define PROSODICBOUNDARIESANNOTATOR_H

#include <QObject>
#include <QString>
#include <QList>

#include "pncore/base/RealValueList.h"
#include "pncore/annotation/Interval.h"
#include "pncore/annotation/IntervalTier.h"

struct ProsodicBoundariesAnnotatorData;

class ProsodicBoundariesAnnotator : public QObject
{
    Q_OBJECT
public:
    explicit ProsodicBoundariesAnnotator(QObject *parent = 0);
    ~ProsodicBoundariesAnnotator();

    // Statistical models
    QString modelsPath() const;
    void setModelsPath(const QString &modelsPath);
    QString modelFilenameBoundary() const;
    void setModelFilenameBoundary(const QString &filename);
    QString modelFilenameBoundaryCountours() const;
    void setModelFilenameBoundaryCountours(const QString &filename);

    // Parameters
    QString attributeBoundaryTrain() const;
    void setAttributeBoundaryTrain(const QString &attributeID);
    QString attributeBoundaryContourTrain() const;
    void setAttributeBoundaryContourTrain(const QString &attributeID);

    // Methods to control the creation of feature tables and training files
    bool openFeaturesTableFile(const QString &filename);
    bool openCRFDataFile(const QString &filename);
    void closeFeaturesTableFile();
    void closeCRFDataFile();

    static void prepareFeatures(QHash<QString, RealValueList> &features, Praaline::Core::IntervalTier *tier_syll);

    Praaline::Core::IntervalTier *annotate(
            const QString &annotationID, const QString &speakerID, const QString &tierName,
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
            bool annotateContours);

private:
    int outputCRF(Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                  QHash<QString, RealValueList> &features, bool withPOS, bool annotateContours,
                  QTextStream &out, bool createSequences = true);
    Praaline::Core::IntervalTier *annotateWithCRF(
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
            QHash<QString, RealValueList> &features, bool withPOS, bool annotateContours,
            const QString &filenameModel, const QString &tier_name = "boundary_auto");

    ProsodicBoundariesAnnotatorData *d;
};

#endif // PROSODICBOUNDARIESANNOTATOR_H
