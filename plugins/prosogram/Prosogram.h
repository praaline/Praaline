#ifndef PROSOGRAM_H
#define PROSOGRAM_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;
#include "AnnotationPluginPraatScript.h"

class ProsoGram : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit ProsoGram(QObject *parent = nullptr);

    void runProsoGram(Corpus *corpus, CorpusRecording *rec, AnnotationTierGroup *tiers, QString annotationID, QString speakerID);
    void importResultFiles(Corpus *corpus, CorpusRecording *rec, AnnotationTierGroup *tiers, QString annotationID, QString speakerID,
                           QString prosoPath = QString());
    static bool updateGlobal(CorpusCommunication *com, const QString &filenameGlobalsheet);

    double timeRangeFrom;
    double timeRangeTo;
    double f0DetectionMin;
    double f0DetectionMax;
    double framePeriod;
    int segmentationMethod;
    QString levelPhone;
    QString levelSyllable;
    QString levelSegmentation;
    QString levelTonalSegments;
    QString levelVUV;
    int glissandoThreshold;
    bool keepIntermediateFiles;
    // Plotting
    bool createImageFiles;
    int plottingStyle;
    double plottingIntervalPerStrip;
    QString plottingTiersToShow;
    bool plottingMultiStrip;
    int plottingFileFormat;
    QString plottingOutputDirectory;

signals:

public slots:

private:
    bool updateTonalSegmentsAndVUV(const QString &filenameNuclei, const QString &filenameStylPitchTier,
                                   IntervalTier *tier_tonal_segments, IntervalTier *tier_vuv);
};

#endif // PROSOGRAM_H
