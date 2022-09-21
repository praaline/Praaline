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
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

#include "AnalyserTemporal.h"
#include "AnalyserTemporalItem.h"
#include "AnalyserTemporalWidget.h"
#include "ui_AnalyserTemporalWidget.h"

#include "statistics/StatisticalAnalysisChartsWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct AnalyserTemporalWidgetData {
    AnalyserTemporalWidgetData() :
        repository(nullptr), analyser(nullptr), gridviewResults(nullptr), modelCom(nullptr), modelSpk(nullptr),
        gridviewMeasureDefinitions(nullptr), modelMeasureDefinitionsCom(nullptr), modelMeasureDefinitionsSpk(nullptr),
        gridviewTimeline(nullptr), modelTimeline(nullptr), chartsWidget(nullptr)
    {}

    CorpusRepository *repository;
    AnalyserTemporal *analyser;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelCom;
    QStandardItemModel *modelSpk;
    GridViewWidget *gridviewMeasureDefinitions;
    QStandardItemModel *modelMeasureDefinitionsCom;
    QStandardItemModel *modelMeasureDefinitionsSpk;
    GridViewWidget *gridviewTimeline;
    QStandardItemModel *modelTimeline;
    StatisticalAnalysisChartsWidget *chartsWidget;
};

AnalyserTemporalWidget::AnalyserTemporalWidget(CorpusRepository *repository, AnalyserTemporal *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::AnalyserTemporalWidget), d(new AnalyserTemporalWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Analyser
    d->analyser = analyser;
    connect(d->analyser, &AnalyserTemporal::madeProgress, this, &AnalyserTemporalWidget::madeProgress);
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
    if (levelIDs.contains(d->analyser->levelIDSyllables())) ui->comboBoxLevelSyllables->setCurrentText(d->analyser->levelIDSyllables());
    if (levelIDs.contains(d->analyser->levelIDTokens())) ui->comboBoxLevelTokens->setCurrentText(d->analyser->levelIDTokens());
    // Command Analyse
    connect(ui->commandAnalyse, &QAbstractButton::clicked, this, &AnalyserTemporalWidget::analyse);
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
    connect(ui->optionCommunications, &QAbstractButton::toggled, this, &AnalyserTemporalWidget::changeDisplayedModel);
    connect(ui->optionSpeakers, &QAbstractButton::toggled, this, &AnalyserTemporalWidget::changeDisplayedModel);
    connect(ui->optionOrientationVertical, &QAbstractButton::toggled, this, &AnalyserTemporalWidget::changeDisplayedModel);
    connect(ui->optionOrientationHorizontal, &QAbstractButton::toggled, this, &AnalyserTemporalWidget::changeDisplayedModel);
    // ================================================================================================================
    // CHARTS TAB
    // ================================================================================================================
    d->chartsWidget = new StatisticalAnalysisChartsWidget(d->repository, d->analyser, this);
    ui->gridLayoutCharts->addWidget(d->chartsWidget);
    // Go to results tab
    ui->tabWidget->setCurrentIndex(0);
}

AnalyserTemporalWidget::~AnalyserTemporalWidget()
{
    delete ui;
    delete d;
}

void AnalyserTemporalWidget::createMeasureDefinitionsTableModels()
{
    // Measures for communications
    d->modelMeasureDefinitionsCom = new QStandardItemModel(this);
    d->modelMeasureDefinitionsCom->setHorizontalHeaderLabels(QStringList() <<
                                                             "Measure ID" << "Measure" << "Units" << "Description");
    foreach (QString measureID, AnalyserTemporalItem::measureIDsForCommunication()) {
        QList<QStandardItem *> items;
        StatisticalMeasureDefinition def = AnalyserTemporalItem::measureDefinition(measureID);
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
    foreach (QString measureID, AnalyserTemporalItem::measureIDsForSpeaker()) {
        QList<QStandardItem *> items;
        StatisticalMeasureDefinition def = AnalyserTemporalItem::measureDefinition(measureID);
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

void AnalyserTemporalWidget::madeProgress(int value)
{
    ui->progressBar->setValue(value);
    QApplication::processEvents();
}

void AnalyserTemporalWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();

    // Analyse corpus
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(corpus->communicationsCount());
    d->analyser->setCorpus(corpus);
    d->analyser->setLevelIDSyllables(ui->comboBoxLevelSyllables->currentText());
    d->analyser->setLevelIDTokens(ui->comboBoxLevelTokens->currentText());
    d->analyser->analyse();
    ui->progressBar->setValue(ui->progressBar->maximum());

    changeDisplayedModel();
}

void AnalyserTemporalWidget::changeDisplayedModel()
{
    if (ui->optionCommunications->isChecked()) {
        d->gridviewMeasureDefinitions->tableView()->setModel(d->modelMeasureDefinitionsCom);
        d->gridviewMeasureDefinitions->tableView()->resizeColumnsToContents();
        d->chartsWidget->showMeasuresForCom();
        showAnalysisForCom();
    }  else {
        d->gridviewMeasureDefinitions->tableView()->setModel(d->modelMeasureDefinitionsSpk);
        d->gridviewMeasureDefinitions->tableView()->resizeColumnsToContents();
        d->chartsWidget->showMeasuresForSpk();
        showAnalysisForSpk();
    }
}

void AnalyserTemporalWidget::showAnalysisForCom()
{
    if (!d->repository) return;
    if (!d->analyser->corpus()) return;
    buildModelForCom();
    // Update table
    d->gridviewResults->tableView()->setModel(d->modelCom);
    d->gridviewTimeline->tableView()->setModel(d->modelTimeline);
}

void AnalyserTemporalWidget::showAnalysisForSpk()
{
    if (!d->repository) return;
    if (!d->analyser->corpus()) return;
    buildModelForSpk();
    // Update table
    d->gridviewResults->tableView()->setModel(d->modelSpk);
}

void AnalyserTemporalWidget::buildModelForCom()
{
    // Select orientation
    Qt::Orientation orientation = Qt::Vertical;
    if (ui->optionOrientationHorizontal->isChecked()) orientation = Qt::Horizontal;
    // Create new model
    if (!d->modelCom)  d->modelCom = new QStandardItemModel(this);
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
    foreach (QString measureID, AnalyserTemporalItem::measureIDsForCommunication())
        if (orientation == Qt::Vertical) labels << measureID; else labels << AnalyserTemporalItem::measureDefinition(measureID).displayName();
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
        foreach (QString measureID, AnalyserTemporalItem::measureIDsForCommunication()) {
            // analyser->measureDefinitionForCommunication(measureID).displayNameUnit()
            item = new QStandardItem(); item->setData(d->analyser->item(com->ID())->measureCom(measureID), Qt::DisplayRole); itemsCom << item;
        }
        if (orientation == Qt::Vertical)
            d->modelCom->appendRow(itemsCom);
        else
            d->modelCom->appendColumn(itemsCom);
        // Timeline
        QPointer<IntervalTier> timeline = d->analyser->item(com->ID())->timelineSpeaker();
        if (timeline) {
            foreach (Interval *intv, timeline->intervals()) {
                QString intervalType = intv->attribute("temporal").toString();
                if      (intervalType == "X")   intervalType = "X    Exclude";
                else if (intervalType == "S")   intervalType = "S    Speech";
                else if (intervalType == "P")   intervalType = "P    Pause";
                else if (intervalType == "G")   intervalType = "G    Gap";
                else if (intervalType == "OVT") intervalType = "OVT  Overlap with turn change";
                else if (intervalType == "OVC") intervalType = "OVT  Overlap keeping floor";

                QStandardItem *itemTimeline; QList<QStandardItem *> itemsTimeline;
                itemTimeline = new QStandardItem(); itemTimeline->setData(com->ID(), Qt::DisplayRole); itemsTimeline << itemTimeline;
                // selected metadata attributes
                for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
                    if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
                    MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
                    itemTimeline = new QStandardItem(); itemTimeline->setData(com->property(attr->ID()), Qt::DisplayRole); itemsTimeline << itemTimeline;
                }
                itemTimeline = new QStandardItem(); itemTimeline->setData(intv->text(), Qt::DisplayRole); itemsTimeline << itemTimeline; // speakers
                itemTimeline = new QStandardItem(); itemTimeline->setData(intv->tMin().toDouble(), Qt::DisplayRole); itemsTimeline << itemTimeline;
                itemTimeline = new QStandardItem(); itemTimeline->setData(intervalType, Qt::DisplayRole); itemsTimeline << itemTimeline;
                itemTimeline = new QStandardItem(); itemTimeline->setData(intv->duration().toDouble(), Qt::DisplayRole); itemsTimeline << itemTimeline;
                d->modelTimeline->appendRow(itemsTimeline);
            }
        }
    }
    // In horizontal orientation, column labels are the Communication IDs
    if (orientation == Qt::Horizontal) d->modelCom->setHorizontalHeaderLabels(horizontalHeader);
}

void AnalyserTemporalWidget::buildModelForSpk()
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
    foreach (QString measureID, AnalyserTemporalItem::measureIDsForSpeaker()) {
        if (orientation == Qt::Vertical) labels << measureID; else labels << AnalyserTemporalItem::measureDefinition(measureID).displayName();
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
            foreach (QString measureID, AnalyserTemporalItem::measureIDsForSpeaker()) {
                item = new QStandardItem(); item->setData(d->analyser->item(com->ID())->measureSpk(speakerID, measureID), Qt::DisplayRole); itemsSpk << item;
            }
            if (orientation == Qt::Vertical)
                d->modelSpk->appendRow(itemsSpk);
            else
                d->modelSpk->appendColumn(itemsSpk);
        }
    }
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
