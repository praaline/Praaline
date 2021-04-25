#ifndef PROSOGRAM_H
#define PROSOGRAM_H

#include <QObject>
#include <QString>
#include <QStringList>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class CorpusRecording;
class AnnotationTierGroup;
class IntervalTier;
}
}

#include "AnnotationPluginPraatScript.h"

class ProsoGram : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit ProsoGram(QObject *parent = nullptr);

    void runProsoGram(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusRecording *rec, Praaline::Core::AnnotationTierGroup *tiers,
                      QString annotationID, QString speakerID);
    void importResultFiles(Praaline::Core::Corpus *corpus, Praaline::Core::CorpusRecording *rec, Praaline::Core::AnnotationTierGroup *tiers,
                           QString annotationID, QString speakerID, QString prosoPath = QString());
    static bool updateGlobal(Praaline::Core::CorpusCommunication *com, const QString &filenameGlobalsheet);

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
                                   Praaline::Core::IntervalTier *tier_tonal_segments, Praaline::Core::IntervalTier *tier_vuv);
};

#endif // PROSOGRAM_H
