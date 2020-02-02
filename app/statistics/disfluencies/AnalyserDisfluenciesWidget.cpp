#include <QApplication>
#include <QProgressBar>
#include <QStandardItem>
#include <QStandardItemModel>

#include <QtCharts/QChartView>
#include <QtCharts/QBoxPlotSeries>
#include <QtCharts/QBoxSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
QT_CHARTS_USE_NAMESPACE

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusSpeaker.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Statistics/StatisticalSummary.h"
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

#include "AnalyserDisfluencies.h"
#include "AnalyserDisfluenciesItem.h"
#include "AnalyserDisfluenciesWidget.h"
#include "ui_AnalyserDisfluenciesWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginDisfluencies {

struct AnalyserDisfluenciesWidgetData {
    AnalyserDisfluenciesWidgetData() :
        repository(nullptr), analyser(nullptr), gridviewResults(nullptr), modelCom(nullptr), modelSpk(nullptr),
        gridviewMeasureDefinitions(nullptr), modelMeasureDefinitionsCom(nullptr), modelMeasureDefinitionsSpk(nullptr),
        gridviewTimeline(nullptr), modelTimeline(nullptr)
    {}

    CorpusRepository *repository;
    AnalyserDisfluencies *analyser;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelCom;
    QStandardItemModel *modelSpk;
    GridViewWidget *gridviewMeasureDefinitions;
    QStandardItemModel *modelMeasureDefinitionsCom;
    QStandardItemModel *modelMeasureDefinitionsSpk;
    GridViewWidget *gridviewTimeline;
    QStandardItemModel *modelTimeline;
};

AnalyserDisfluenciesWidget::AnalyserDisfluenciesWidget(CorpusRepository *repository, AnalyserDisfluencies *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::AnalyserDisfluenciesWidget), d(new AnalyserDisfluenciesWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Analyser
    d->analyser = analyser;
    connect(d->analyser, SIGNAL(madeProgress(int)), this, SLOT(madeProgress(int)));
    // ================================================================================================================
    // MAIN WIDGET AND RESUTLS TAB
    // ================================================================================================================
    // Corpora combobox
    ui->comboBoxCorpus->addItems(repository->listCorporaIDs());
    // Metadata attributes
    int i = 0;
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Communication)){
        ui->comboBoxMetadataCom->insertItem(i, attr->name(), false);
        ++i;
    }
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)){
        ui->comboBoxMetadataSpk->insertItem(i, attr->name(), false);
        ++i;
    }
    // Levels for syllables and tokens
    QStringList levelIDs = repository->annotationStructure()->levelIDs();
    ui->comboBoxLevelSyllables->addItems(levelIDs);
    ui->comboBoxLevelTokens->addItems(levelIDs);
    if (levelIDs.contains(d->analyser->levelSyllables()))   ui->comboBoxLevelSyllables->setCurrentText(d->analyser->levelSyllables());
    if (levelIDs.contains(d->analyser->levelTokens()))      ui->comboBoxLevelTokens->setCurrentText(d->analyser->levelTokens());
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Results grid view
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);
    // Timeline grid view
    d->gridviewTimeline = new GridViewWidget(this);
    d->gridviewTimeline->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutTimeline->addWidget(d->gridviewTimeline);
    // Measure definitions grid view
    d->gridviewMeasureDefinitions = new GridViewWidget(this);
    d->gridviewMeasureDefinitions->tableView()->verticalHeader()->setDefaultSectionSize(20);
    d->gridviewMeasureDefinitions->tableView()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->gridLayoutMeasureDefinitions->addWidget(d->gridviewMeasureDefinitions);
    // Create measure definitions table models
    createMeasureDefinitionsTableModels();
    // Default
    ui->optionCommunications->setChecked(true);
    ui->optionOrientationVertical->setChecked(true);
    // Change display
    connect(ui->optionCommunications, SIGNAL(toggled(bool)), this, SLOT(changeDisplayedModel()));
    connect(ui->optionSpeakers, SIGNAL(toggled(bool)), this, SLOT(changeDisplayedModel()));
    connect(ui->optionOrientationVertical, SIGNAL(toggled(bool)), this, SLOT(changeDisplayedModel()));
    connect(ui->optionOrientationHorizontal, SIGNAL(toggled(bool)), this, SLOT(changeDisplayedModel()));
    // ================================================================================================================
    // CHARTS TAB
    // ================================================================================================================
    // Measure combobox (start with measures for Communications by default)
    foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForCommunication())
        ui->comboBoxMeasure->addItem(AnalyserDisfluenciesDefinitions::measureDefinition(measureID).displayName(), measureID);
    // Group by attributes
    if (d->repository->metadataStructure()) {
        ui->comboBoxGroupByCom->addItem("", "");
        ui->comboBoxGroupByCom->addItem("Communication ID", "ID");
        foreach (MetadataStructureAttribute *attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
            ui->comboBoxGroupByCom->addItem(attr->name(), attr->ID());
        }
        ui->comboBoxGroupByCom->setCurrentText("");
        // speaker
        ui->comboBoxGroupBySpk->addItem("", "");
        ui->comboBoxGroupBySpk->addItem("Speaker ID", "ID");
        foreach (MetadataStructureAttribute *attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
            ui->comboBoxGroupBySpk->addItem(attr->name(), attr->ID());
        }
        ui->comboBoxGroupBySpk->setCurrentText("");
    }
    // Command Draw Chart
    connect(ui->commandDrawChart, SIGNAL(clicked(bool)), this, SLOT(drawChart()));
    // Defaults
    ui->checkBoxSetYMinMax->setChecked(true);
    // Go to results tab
    ui->tabWidget->setCurrentIndex(0);
}

AnalyserDisfluenciesWidget::~AnalyserDisfluenciesWidget()
{
    delete ui;
    delete d;
}

void AnalyserDisfluenciesWidget::createMeasureDefinitionsTableModels()
{
    // Measures for communications
    d->modelMeasureDefinitionsCom = new QStandardItemModel(this);
    d->modelMeasureDefinitionsCom->setHorizontalHeaderLabels(QStringList() <<
                                                             "Measure ID" << "Measure" << "Units" << "Description");
    foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForCommunication()) {
        QList<QStandardItem *> items;
        StatisticalMeasureDefinition def = AnalyserDisfluenciesDefinitions::measureDefinition(measureID);
        items << new QStandardItem(measureID);
        items << new QStandardItem(def.displayName());
        items << new QStandardItem(def.units());
        items << new QStandardItem(def.description());
        d->modelMeasureDefinitionsCom->appendRow(items);
    }
    // Measures for speakers
    d->modelMeasureDefinitionsSpk = new QStandardItemModel(this);
    d->modelMeasureDefinitionsSpk->setHorizontalHeaderLabels(QStringList() <<
                                                             "Measure ID" << "Measure" << "Units" << "Description");
    foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForSpeaker()) {
        QList<QStandardItem *> items;
        StatisticalMeasureDefinition def = AnalyserDisfluenciesDefinitions::measureDefinition(measureID);
        items << new QStandardItem(measureID);
        items << new QStandardItem(def.displayName());
        items << new QStandardItem(def.units());
        items << new QStandardItem(def.description());
        d->modelMeasureDefinitionsSpk->appendRow(items);
    }
    // Start by showing model for com
    d->gridviewMeasureDefinitions->tableView()->setModel(d->modelMeasureDefinitionsCom);
    d->gridviewMeasureDefinitions->tableView()->resizeColumnsToContents();
}

void AnalyserDisfluenciesWidget::madeProgress(int value)
{
    ui->progressBar->setValue(value);
    QApplication::processEvents();
}

void AnalyserDisfluenciesWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();

    // Analyse corpus
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(corpus->communicationsCount());
    d->analyser->setCorpus(corpus);
    d->analyser->setLevelSyllables(ui->comboBoxLevelSyllables->currentText());
    d->analyser->setLevelTokens(ui->comboBoxLevelTokens->currentText());
    d->analyser->analyse();
    ui->progressBar->setValue(ui->progressBar->maximum());

    changeDisplayedModel();
}

void AnalyserDisfluenciesWidget::changeDisplayedModel()
{
    if (ui->optionCommunications->isChecked()) {
        d->gridviewMeasureDefinitions->tableView()->setModel(d->modelMeasureDefinitionsCom);
        d->gridviewMeasureDefinitions->tableView()->resizeColumnsToContents();
        ui->comboBoxMeasure->clear();
        foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForCommunication())
            ui->comboBoxMeasure->addItem(AnalyserDisfluenciesDefinitions::measureDefinition(measureID).displayName(), measureID);
        showAnalysisForCom();
    }  else {
        d->gridviewMeasureDefinitions->tableView()->setModel(d->modelMeasureDefinitionsSpk);
        d->gridviewMeasureDefinitions->tableView()->resizeColumnsToContents();
        ui->comboBoxMeasure->clear();
        foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForSpeaker())
            ui->comboBoxMeasure->addItem(AnalyserDisfluenciesDefinitions::measureDefinition(measureID).displayName(), measureID);
        showAnalysisForSpk();
    }
}

void AnalyserDisfluenciesWidget::showAnalysisForCom()
{
    if (!d->repository) return;
    if (!d->analyser->corpus()) return;
    buildModelForCom();
    // Update table
    d->gridviewResults->tableView()->setModel(d->modelCom);
    d->gridviewTimeline->tableView()->setModel(d->modelTimeline);
}

void AnalyserDisfluenciesWidget::showAnalysisForSpk()
{
    if (!d->repository) return;
    if (!d->analyser->corpus()) return;
    buildModelForSpk();
    // Update table
    d->gridviewResults->tableView()->setModel(d->modelSpk);
}

void AnalyserDisfluenciesWidget::buildModelForCom()
{
    // Select orientation
    Qt::Orientation orientation = Qt::Vertical;
    if (ui->optionOrientationHorizontal->isChecked()) orientation = Qt::Horizontal;
    // Create new model if needed
    if (!d->modelCom) d->modelCom = new QStandardItemModel(this);
    d->modelCom->clear();
    // Timeline model
    if (!d->modelTimeline) d->modelTimeline = new QStandardItemModel(this);
    d->modelTimeline->clear();
    // Checks
    if (!d->analyser->corpus()) return;
    // Create model headers
    QStringList labels, labelsTimeline;
    if (orientation == Qt::Vertical) labels << "CommunicationID";
    labelsTimeline << "CommunicationID";
    for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
        if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
        MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
        if (orientation == Qt::Vertical) labels << attr->ID(); else labels << attr->name();
        labelsTimeline << attr->ID();
    }
    foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForCommunication())
        if (orientation == Qt::Vertical) labels << measureID; else labels << AnalyserDisfluenciesDefinitions::measureDefinition(measureID).displayName();
    // Headers for Com model
    if (orientation == Qt::Vertical)
        d->modelCom->setHorizontalHeaderLabels(labels);
    else {
        QList<QStandardItem *> itemsHeader;
        foreach (QString label, labels) itemsHeader << new QStandardItem(label);
        d->modelCom->appendColumn(itemsHeader);
    }
    // Timeline labels
    labelsTimeline << "Speaker" << "Timecode" << "IntervalType" << "Duration";
    // Headers for timeline model
    d->modelTimeline->setHorizontalHeaderLabels(labelsTimeline);
    // Data
    QStringList horizontalHeader; horizontalHeader << "Measure";
    foreach (CorpusCommunication *com, d->analyser->corpus()->communications()) {
        if (!com) continue;
        if (!d->analyser->item(com->ID())) continue;

        QList<QStandardItem *> itemsCom;
        QStandardItem *item;
        if (orientation == Qt::Vertical) {
            item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsCom << item;
        } else {
            horizontalHeader << com->ID();
        }
        // selected metadata attributes
        for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
            if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
            MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
            item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsCom << item;
        }
        // measures
        foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForCommunication()) {
            // analyser->measureDefinitionForCommunication(measureID).displayNameUnit()
            item = new QStandardItem(); item->setData(d->analyser->item(com->ID())->measureCom(measureID), Qt::DisplayRole); itemsCom << item;
        }
        if (orientation == Qt::Vertical)
            d->modelCom->appendRow(itemsCom);
        else
            d->modelCom->appendColumn(itemsCom);
    }
    // In horizontal orientation, column labels are the Communication IDs
    if (orientation == Qt::Horizontal) d->modelCom->setHorizontalHeaderLabels(horizontalHeader);
}

void AnalyserDisfluenciesWidget::buildModelForSpk()
{
    // Select orientation
    Qt::Orientation orientation = Qt::Vertical;
    if (ui->optionOrientationHorizontal->isChecked()) orientation = Qt::Horizontal;
    // Create new model
    if (!d->modelSpk) d->modelSpk = new QStandardItemModel(this);
    d->modelSpk->clear();
    // Checks
    if (!d->analyser->corpus()) return;
    // Create model headers
    QStringList labels;
    labels << "CommunicationID" << "SpeakerID";
    for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
        if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
        MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
        if (orientation == Qt::Vertical) labels << attr->ID(); else labels << attr->name();
    }
    for (int i = 0; i < ui->comboBoxMetadataSpk->count(); ++i) {
        if (!ui->comboBoxMetadataSpk->itemData(i).toBool()) continue;
        MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker).at(i);
        if (orientation == Qt::Vertical) labels << attr->ID(); else labels << attr->name();
    }
    foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForSpeaker()) {
        if (orientation == Qt::Vertical) labels << measureID; else labels << AnalyserDisfluenciesDefinitions::measureDefinition(measureID).displayName();
    }
    // Headers for speaker model
    if (orientation == Qt::Vertical) {
        d->modelSpk->setHorizontalHeaderLabels(labels);
    }
    else {
        QList<QStandardItem *> itemsHeader;
        foreach (QString label, labels) itemsHeader << new QStandardItem(label);
        d->modelSpk->appendColumn(itemsHeader);
    }
    // Data
    foreach (CorpusCommunication *com, d->analyser->corpus()->communications()) {
        if (!com) continue;
        if (!d->analyser->item(com->ID())) continue;
        foreach (QString speakerID, d->analyser->item(com->ID())->speakerIDs()) {
            QPointer<CorpusSpeaker> spk = d->analyser->corpus()->speaker(speakerID);
            QList<QStandardItem *> itemsSpk;
            QStandardItem *item;
            item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsSpk << item;
            item = new QStandardItem(); item->setData(speakerID, Qt::DisplayRole); itemsSpk << item;
            // selected metadata attributes (Com, then Spk)
            for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
                if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
                MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
                item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsSpk << item;
            }
            for (int i = 0; i < ui->comboBoxMetadataSpk->count(); ++i) {
                if (!ui->comboBoxMetadataSpk->itemData(i).toBool()) continue;
                MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker).at(i);
                item = new QStandardItem(); item->setData(spk->property(attr->ID()), Qt::DisplayRole); itemsSpk << item;
            }
            // measures
            foreach (QString measureID, AnalyserDisfluenciesDefinitions::measureIDsForSpeaker()) {
                item = new QStandardItem(); item->setData(d->analyser->item(com->ID())->measureSpk(speakerID, measureID), Qt::DisplayRole); itemsSpk << item;
            }
            if (orientation == Qt::Vertical)
                d->modelSpk->appendRow(itemsSpk);
            else
                d->modelSpk->appendColumn(itemsSpk);
        }
    }
}

void AnalyserDisfluenciesWidget::drawChart()
{
    if (!d->analyser) return;
    if (!d->analyser->corpus()) return;

    // Get parameters from user interface
    QString measureID = ui->comboBoxMeasure->currentData().toString();
    QStringList groupAttributeIDsCom; groupAttributeIDsCom << ui->comboBoxGroupByCom->currentData().toString();
    QStringList groupAttributeIDsSpk; groupAttributeIDsSpk << ui->comboBoxGroupBySpk->currentData().toString();
    double yMin = ui->doubleSpinBoxYMin->value();
    double yMax = ui->doubleSpinBoxYMax->value();
    // Aggregate selected measure, over selected metadata attributes
    QMap<QString, QList<double> > aggregates;
    QString groupAttributes;
    if (ui->optionCommunications->isChecked()) {
        aggregates = d->analyser->aggregateMeasureCom(measureID, groupAttributeIDsCom);
        groupAttributes = ui->comboBoxGroupByCom->currentText();
    }
    else {
        aggregates = d->analyser->aggregateMeasureSpk(measureID, groupAttributeIDsCom, groupAttributeIDsSpk);
        QStringList sl; sl << ui->comboBoxGroupByCom->currentText() << ui->comboBoxGroupBySpk->currentText();
        groupAttributes = sl.join(", ");
    }
    if (groupAttributes.endsWith(", ")) groupAttributes.chop(2);
    // Create chart series
    QChart *chart = new QChart();
    double min(0), max(0);
    if (true) {
        QBoxPlotSeries *series = new QBoxPlotSeries();
        series->setName(measureID);
        foreach (QString groupID, aggregates.keys()) {
            QBoxSet *set = new QBoxSet(groupID);
            StatisticalSummary summary;
            summary.calculate(aggregates.value(groupID));
    //        set->setValue(QBoxSet::LowerExtreme, summary.firstQuartile() - 1.5 * summary.interQuartileRange());
    //        set->setValue(QBoxSet::UpperExtreme, summary.thirdQuartile() + 1.5 * summary.interQuartileRange());
            set->setValue(QBoxSet::LowerExtreme, summary.min());
            set->setValue(QBoxSet::UpperExtreme, summary.max());
            set->setValue(QBoxSet::Median, summary.median());
            set->setValue(QBoxSet::LowerQuartile, summary.firstQuartile());
            set->setValue(QBoxSet::UpperQuartile, summary.thirdQuartile());
            series->append(set);
            if (min > summary.min()) min = summary.min();
            if (max < summary.max()) max = summary.max();
        }
        chart->addSeries(series);
        chart->createDefaultAxes();
    } else {
        QBarSeries *series = new QBarSeries();
        series->setName(measureID);
        foreach (QString groupID, aggregates.keys()) {
            QBarSet *set = new QBarSet(groupID);
            StatisticalSummary summary;
            summary.calculate(aggregates.value(groupID));
            set->append(summary.mean());
            series->append(set);
            if (min > summary.min()) min = summary.min();
            if (max < summary.max()) max = summary.max();
        }
        chart->addSeries(series);
        chart->createDefaultAxes();
    }
    // Configure chart
    chart->setTitle(QString("%1 per %2").arg(ui->comboBoxMeasure->currentText()).arg(groupAttributes));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    if (ui->checkBoxSetYMinMax->isChecked()) {
        chart->axisY()->setMin(yMin);
        chart->axisY()->setMax(yMax);
    } else {
        chart->axisY()->setMin(qRound(min * 0.9));
        chart->axisY()->setMax(qRound(max * 1.1));
    }
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    // Clear previous chart
    QLayoutItem *item;
    while ((item = ui->gridLayoutChart->takeAt(0)) != 0) { delete item; }
    QList<QChartView *> chartviews;
    chartviews = findChildren<QChartView *>();
    qDeleteAll(chartviews);
    // Create new chart view widget
    QChartView *chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    // Show chart view
    ui->gridLayoutChart->addWidget(chartView);
}

} // namespace StatisticsPluginDisfluencies
} // namespace Plugins
} // namespace Praaline
