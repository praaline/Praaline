#include <QDebug>
#include <QMessageBox>
#include <math.h>

#include "PCAPlotWidget.h"
#include "ui_PCAPlotWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "qcustomplot.h"
#include "pngui/widgets/GridViewWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginPCA {

struct PCAPlotWidgetData {
    PCAPlotWidgetData() :
        repository(0)
    {}
    CorpusRepository *repository;
    QCustomPlot *plot;
    QList<QPointer<CorpusCommunication> > communications;
};

PCAPlotWidget::PCAPlotWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::PCAPlotWidget), d(new PCAPlotWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;

    ui->splitter->setSizes(QList<int>() << 400 << 100);

    // List of metadata attributes for Communication -> Filter attribute
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
        ui->comboBoxFilterAttribute->addItem(attr->name(), attr->ID());
    }
    connect(ui->comboBoxFilterAttribute, SIGNAL(currentIndexChanged(QString)), this, SLOT(filterAttributeChanged(QString)));
    // Selection
    connect(ui->listFilterAttributeValues, SIGNAL(currentTextChanged(QString)), this, SLOT(fileterListSelectionChanged(QString)));

    // Set up plot
    d->plot = new QCustomPlot(this);
    ui->gridLayoutPlot->addWidget(d->plot);

    // Handle click and double click on graph points
    connect(d->plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(plotItemClick(QCPAbstractPlottable*,int,QMouseEvent*)));
    connect(d->plot, SIGNAL(plottableDoubleClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(plotItemDoubleClick(QCPAbstractPlottable*,int,QMouseEvent*)));

    ui->comboBoxFilterAttribute->setCurrentText("Utterance ID");
}

PCAPlotWidget::~PCAPlotWidget()
{
    delete ui;
}

void PCAPlotWidget::filterAttributeChanged(const QString &attributeName)
{
    Q_UNUSED(attributeName)
    if (!d->repository) return;
    QString attributeID = ui->comboBoxFilterAttribute->currentData().toString();
    if (!d->repository->metadataStructure()->attributeIDs(CorpusObject::Type_Communication).contains(attributeID)) return;
    QList<QPair<QList<QVariant>, long> > values;
    QPair<QList<QVariant>, long> value;
    values = d->repository->metadata()->getDistinctValues(CorpusObject::Type_Communication, QStringList() << attributeID);
    ui->listFilterAttributeValues->clear();
    foreach (value, values) {
        ui->listFilterAttributeValues->addItem(value.first.first().toString());
    }
}

void PCAPlotWidget::fileterListSelectionChanged(const QString &attributeValue)
{
    if (!d->repository) return;
    QString attributeID = ui->comboBoxFilterAttribute->currentData().toString();
    if (!d->repository->metadataStructure()->attributeIDs(CorpusObject::Type_Communication).contains(attributeID)) return;

    // Select communications
    if (d->repository->listCorporaIDs().isEmpty()) return;
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(d->repository->listCorporaIDs().first());
    if (!corpus) return;
    QList<QPointer<CorpusCommunication> > communications;
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        if (!com) continue;
        if (com->property(attributeID).toString() == attributeValue)
            communications << com;
    }

    // Generate data points
    QVector<double> x(communications.count()), y(communications.count()); // initialize with entries 0..100
    for (int i = 0; i < communications.count(); ++i)
    {
        x[i] = communications.at(i)->property("PCA_dim1").toDouble();
        y[i] = communications.at(i)->property("PCA_dim2").toDouble();
    }
    // create graph and assign data to it:
    d->plot->clearGraphs();
    d->plot->addGraph();
    d->plot->graph(0)->setData(x, y);
    d->plot->graph(0)->setLineStyle(QCPGraph::lsNone);
    d->plot->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    // Axis labels
    d->plot->xAxis->setLabel("PCA Dimension 1");
    d->plot->yAxis->setLabel("PCA Dimension 2");
    // Set axes so that all data is visible, with a little margin
    d->plot->rescaleAxes(true);
    d->plot->xAxis->setRange(d->plot->xAxis->range().lower - 1.0, d->plot->xAxis->range().upper + 1.0);
    d->plot->yAxis->setRange(d->plot->yAxis->range().lower - 1.0, d->plot->yAxis->range().upper + 1.0);
    // Set graph title
    d->plot->plotLayout()->clear();
    d->plot->plotLayout()->insertRow(0);
    d->plot->plotLayout()->addElement(0, 0, new QCPTextElement(d->plot, QString("Principal Component Analysis - Samples for %1").arg(attributeValue),
                                                               QFont("sans", 12, QFont::Bold)));
    d->plot->replot();
}

void PCAPlotWidget::plotItemClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    QMessageBox::information(this, "Click", QString("%1 %2").arg(plottable->name()).arg(index));
}

void PCAPlotWidget::plotItemDoubleClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    QMessageBox::information(this, "Double Click", QString("%1 %2").arg(plottable->name()).arg(index));
}

} // namespace StatisticsPluginPCA
} // namespace Plugins
} // namespace Praaline
