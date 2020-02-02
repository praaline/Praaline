#ifndef SYLLABLEPROMINENCEANNOTATOR_H
#define SYLLABLEPROMINENCEANNOTATOR_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QPair>
#include <QTextStream>
#include "PraalineCore/Base/RealValueList.h"
#include "PraalineCore/Annotation/Interval.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"

struct SyllableProminenceAnnotatorData;

class SyllableProminenceAnnotator : public QObject
{
    Q_OBJECT
public:
    explicit SyllableProminenceAnnotator(QObject *parent = nullptr);
    ~SyllableProminenceAnnotator();

    // Statistical models
    QString modelsPath() const;
    void setModelsPath(const QString &modelsPath);
    QString modelFilenameWithoutPOS() const;
    void setModelFilenameWithoutPOS(const QString &filename);
    QString modelFilenameWithPOS() const;
    void setModelFilenameWithPOS(const QString &filename);

    // Parameters
    QString attributeDelivery() const;
    void setAttributeDelivery(const QString &attributeID);
    QString attributeProminenceTrain() const;
    void setAttributeProminenceTrain(const QString &attributeID);

    // Methods to control the creation of feature tables and training files
    bool openFeaturesTableFile(const QString &filename);
    bool openCRFDataFile(const QString &filename);
    void closeFeaturesTableFile();
    void closeCRFDataFile();

    static void prepareFeatures(QHash<QString, RealValueList> &features,
                                Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_phones = 0);

    Praaline::Core::IntervalTier *annotate(
            const QString &annotationID, const QString &speakerID, const QString &tierName,
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
            bool withPOS = true);

signals:

public slots:

private:
    void outputRFACE(const QString &sampleID,
                     Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                     QHash<QString, RealValueList> &features, QTextStream &out);
    void readRFACEprediction(QString filename, Praaline::Core::IntervalTier *tier_syll, QString attribute);
    void annotateRFACE(QString filenameModel, const QString &sampleID,
                       Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                       QHash<QString, RealValueList> &features, QString attributeOutput);
    void outputSVM(Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                   QHash<QString, RealValueList> &features, QTextStream &out);
    int outputCRF(Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                  QHash<QString, RealValueList> &features, bool withPOS, QTextStream &out,
                  bool createSequences = true);
    Praaline::Core::IntervalTier *annotateWithCRF(
            Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
            QHash<QString, RealValueList> &features, bool withPOS,
            const QString &filenameModel, const QString &tierName = "promise");

    SyllableProminenceAnnotatorData *d;
};

#endif // SYLLABLEPROMINENCEANNOTATOR_H
