#ifndef PROSOGRAM_H
#define PROSOGRAM_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;
#include "annotationpluginpraatscript.h"

class ProsoGram : public AnnotationPluginPraatScript
{
    Q_OBJECT
public:
    explicit ProsoGram(QObject *parent = 0);

    void runProsoGram(Corpus *corpus, CorpusRecording *rec, QPointer<AnnotationTierGroup> tiers, QString annotationID, QString speakerID);

    double timeRangeFrom;
    double timeRangeTo;
    double f0DetectionMin;
    double f0DetectionMax;
    double framePeriod;
    int segmentationMethod;
    QString levelPhone;
    QString levelSyllable;
    QString levelSegmentation;
    int glissandoThreshold;
    bool keepIntermediateFiles;
    bool createImageFiles;
    QString tiersOnImageFiles;
    QString fileFormat;

    static bool updateGlobal(CorpusCommunication *com, const QString &filenameGlobalsheet);

signals:

public slots:

};

#endif // PROSOGRAM_H
