#include <QDebug>
#include <QMessageBox>
#include <math.h>

#include "PCAPlotWidget.h"
#include "ui_PCAPlotWidget.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusSpeaker.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "svgui/layer/ColourDatabase.h"

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
    QMap<QString, QList<CorpusCommunication *> > communications;
};

PCAPlotWidget::PCAPlotWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::PCAPlotWidget), d(new PCAPlotWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;

    ui->splitter->setSizes(QList<int>() << 400 << 100);

    // List of metadata attributes for Communication
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
        ui->comboBoxClassificationAttribute->addItem(attr->name(), attr->ID());
        ui->comboBoxFilterAttribute->addItem(attr->name(), attr->ID());
    }
    // Properties of selected communication
    ui->treeWidgetItemProperties->setHeaderLabels(QStringList() << tr("Property") << tr("Value"));
    // Respond to user changes in the parameters
    connect(ui->comboBoxFilterAttribute, SIGNAL(currentIndexChanged(QString)), this, SLOT(filterAttributeChanged(QString)));
    connect(ui->listFilterAttributeValues, SIGNAL(currentTextChanged(QString)), this, SLOT(replot()));
    connect(ui->comboBoxClassificationAttribute, SIGNAL(currentIndexChanged(QString)), this, SLOT(replot()));

    // Set up plot
    d->plot = new QCustomPlot(this);
    ui->gridLayoutPlot->addWidget(d->plot);

    // Handle click and double click on graph points
    connect(d->plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(plotItemClick(QCPAbstractPlottable*,int,QMouseEvent*)));
    connect(d->plot, SIGNAL(plottableDoubleClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(plotItemDoubleClick(QCPAbstractPlottable*,int,QMouseEvent*)));

    ui->comboBoxFilterAttribute->setCurrentText("Utterance ID");
    ui->comboBoxClassificationAttribute->setCurrentText("Discourse Relation");
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

void PCAPlotWidget::replot()
{
    // Check for valid repository and corpus
    if (!d->repository) return;
    if (d->repository->listCorporaIDs().isEmpty()) return;
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(d->repository->listCorporaIDs().first());
    if (!corpus) return;
    // Get attributes
    QString filterAttributeID = ui->comboBoxFilterAttribute->currentData().toString();
    if (!d->repository->metadataStructure()->attributeIDs(CorpusObject::Type_Communication).contains(filterAttributeID)) return;
    QString classificationAttributeID = ui->comboBoxClassificationAttribute->currentData().toString();
    if (!d->repository->metadataStructure()->attributeIDs(CorpusObject::Type_Communication).contains(classificationAttributeID)) classificationAttributeID = "";
    // Filter attribute
    if (!ui->listFilterAttributeValues->currentItem()) return;
    QString filterAttributeValue = ui->listFilterAttributeValues->currentItem()->text();

    // Select communications and classify them
    d->communications.clear();
    foreach (CorpusCommunication *com, corpus->communications()) {
        if (!com) continue;
        // Filter
        if (com->property(filterAttributeID).toString() != filterAttributeValue ) continue;
        // Classify
        QString comClass = com->property(classificationAttributeID).toString();
        if (!d->communications.contains(comClass)) {
            d->communications.insert(comClass, QList<CorpusCommunication *>());
        }
        d->communications[comClass] << com;
    }

    // Create a graph for each class
    int indexClass(0);
    QVector<QCPScatterStyle::ScatterShape> shapes;

    shapes << QCPScatterStyle::ssDisc << QCPScatterStyle::ssSquare << QCPScatterStyle::ssTriangle
           << QCPScatterStyle::ssCircle << QCPScatterStyle::ssDiamond << QCPScatterStyle::ssStar
           << QCPScatterStyle::ssCross << QCPScatterStyle::ssPlus  << QCPScatterStyle::ssTriangleInverted
           << QCPScatterStyle::ssCrossSquare << QCPScatterStyle::ssPlusSquare << QCPScatterStyle::ssCrossCircle << QCPScatterStyle::ssPlusCircle;
    ColourDatabase *cdb = ColourDatabase::getInstance();
    d->plot->clearGraphs();
    foreach (QString comClass, d->communications.keys()) {
        QList<CorpusCommunication *> communications = d->communications[comClass];
        // Generate data points
        QVector<double> x(communications.count()), y(communications.count());
        for (int i = 0; i < communications.count(); ++i) {
            x[i] = communications.at(i)->property("PCA_dim1").toDouble();
            y[i] = communications.at(i)->property("PCA_dim2").toDouble();
        }
        // Create graph and assign data to it
        if (d->plot->graphCount() < indexClass + 1) {
            d->plot->addGraph();
            d->plot->graph(indexClass)->setLineStyle(QCPGraph::lsNone);
            d->plot->graph(indexClass)->setScatterStyle(shapes.at(indexClass % shapes.count()));
            d->plot->graph(indexClass)->setPen(QPen(cdb->getColour((indexClass + 1) % cdb->getColourCount())));
            d->plot->graph(indexClass)->setBrush(QBrush(cdb->getColour((indexClass + 1) % cdb->getColourCount())));
        }
        d->plot->graph(indexClass)->addData(x, y);
        d->plot->graph(indexClass)->setName(comClass);
        indexClass++;
    }
    // Axis labels
    d->plot->xAxis->setLabel("PCA Dimension 1");
    d->plot->yAxis->setLabel("PCA Dimension 2");
    // Set axes so that all data is visible, with a little margin
    d->plot->rescaleAxes(true);
    d->plot->xAxis->setRange(d->plot->xAxis->range().lower - 1.0, d->plot->xAxis->range().upper + 1.0);
    d->plot->yAxis->setRange(d->plot->yAxis->range().lower - 1.0, d->plot->yAxis->range().upper + 1.0);
    // Legend
    d->plot->legend->setVisible(true);
    d->plot->legend->setFont(QFont("Helvetica", 9));
    d->plot->replot();
}

void PCAPlotWidget::plotItemClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    QList<CorpusCommunication *> communications = d->communications.value(plottable->name());
    if (index < 0) return;
    if (index >= communications.count()) return;
    CorpusCommunication *com = communications.at(index);
    if (!com) return;
    QList<QTreeWidgetItem *> items;
    foreach (MetadataStructureAttribute *attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
        QString attributeValue = com->property(attr->ID()).toString();
        items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList() << attr->name() << attributeValue));
    }
    ui->treeWidgetItemProperties->clear();
    ui->treeWidgetItemProperties->insertTopLevelItems(0, items);
}

void PCAPlotWidget::plotItemDoubleClick(QCPAbstractPlottable *plottable, int index, QMouseEvent *event)
{
    QMessageBox::information(this, "Double Click", QString("%1 %2").arg(plottable->name()).arg(index));
}

} // namespace StatisticsPluginPCA
} // namespace Plugins
} // namespace Praaline
