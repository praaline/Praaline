#include <QObject>
#include <QString>
#include <QList>
#include <QStandardItemModel>
#include "math.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
#include "pncore/statistics/StatisticalSummary.h"
using namespace Praaline::Core;

#include "AnalyserBasic.h"

struct AnalyserBasicData {
    AnalyserBasicData() : model(0)
    {
    }

    QStandardItemModel *model;
    QStringList metadataAttributeIDsCom;
    QStringList metadataAttributeIDsSpk;
    QStringList metadataAttributeIDsRec;
    QStringList metadataAttributeIDsAnnot;
};

AnalyserBasic::AnalyserBasic(QObject *parent)
    : QObject(parent), d(new AnalyserBasicData)
{
    d->model = new QStandardItemModel(this);
}

AnalyserBasic::~AnalyserBasic()
{
    delete d;
}

