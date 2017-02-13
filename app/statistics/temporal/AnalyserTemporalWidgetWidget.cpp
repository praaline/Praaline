#include <QApplication>
#include <QProgressBar>
#include <QStandardItem>
#include <QStandardItemModel>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

#include "AnalyserTemporal.h"
#include "AnalyserTemporalItem.h"
#include "AnalyserTemporalWidgetWidget.h"
#include "ui_AnalyserTemporalWidgetWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct AnalyserTemporalWidgetData {
    AnalyserTemporalWidgetData() :
        repository(0), analyser(0), gridviewResults(0), modelCom(0), modelSpk(0)
    {}

    CorpusRepository *repository;
    AnalyserTemporal *analyser;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelCom;
    QStandardItemModel *modelSpk;
};

AnalyserTemporalWidget::AnalyserTemporalWidget(CorpusRepository *repository, AnalyserTemporal *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::AnalyserTemporalWidget), d(new AnalyserTemporalWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Analyser
    d->analyser = analyser;
    connect(d->analyser, SIGNAL(madeProgress(int)), this, SLOT(madeProgress(int)));
    // Corpora combobox
    ui->comboBoxCorpus->addItems(repository->listCorporaIDs());
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Results grid view
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);
    // Default
    ui->optionCommunications->setChecked(true);
    // Change display
    connect(ui->optionCommunications, SIGNAL(toggled(bool)), this, SLOT(changeDisplayedModel()));
    connect(ui->optionSpeakers, SIGNAL(toggled(bool)), this, SLOT(changeDisplayedModel()));
}

AnalyserTemporalWidget::~AnalyserTemporalWidget()
{
    delete ui;
    delete d;
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
    d->analyser->analyse();
    ui->progressBar->setValue(ui->progressBar->maximum());

    changeDisplayedModel();
}

void AnalyserTemporalWidget::changeDisplayedModel()
{
    if (ui->optionCommunications->isChecked())
        showAnalysisForCom();
    else
        showAnalysisForSpk();
}

void AnalyserTemporalWidget::showAnalysisForCom()
{
    if (!d->repository) return;
    if (!d->analyser->corpus()) return;
    if (!d->modelCom) buildModelForCom();
    // Update table
    d->gridviewResults->tableView()->setModel(d->modelCom);
}

void AnalyserTemporalWidget::showAnalysisForSpk()
{
    if (!d->repository) return;
    if (!d->analyser->corpus()) return;
    if (!d->modelSpk) buildModelForSpk();
    // Update table
    d->gridviewResults->tableView()->setModel(d->modelSpk);
}

void AnalyserTemporalWidget::buildModelForCom()
{
    if (d->modelCom) delete d->modelCom;
    d->modelCom = new QStandardItemModel(this);
    if (!d->analyser->corpus()) return;
    // Create model headers
    QStringList labels;
    labels << "CommunicationID";
    foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication))
        labels << attr->ID();
    foreach (QString measureID, AnalyserTemporalItem::measureIDsForCommunication()) labels << measureID;
    d->modelCom->setHorizontalHeaderLabels(labels);
    // Data
    foreach (QPointer<CorpusCommunication> com, d->analyser->corpus()->communications()) {
        if (!com) continue;
        if (!d->analyser->item(com->ID())) continue;

        QList<QStandardItem *> itemsCom;
        QStandardItem *item;
        item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsCom << item;
        // properties
        foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
            item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsCom << item;
        }
        // measures
        foreach (QString measureID, AnalyserTemporalItem::measureIDsForCommunication()) {
            // analyser->measureDefinitionForCommunication(measureID).displayNameUnit()
            item = new QStandardItem(); item->setData(d->analyser->item(com->ID())->measureCom(measureID), Qt::DisplayRole); itemsCom << item;
        }
        d->modelCom->appendRow(itemsCom);
    }
}

void AnalyserTemporalWidget::buildModelForSpk()
{
    if (d->modelSpk) delete d->modelSpk;
    d->modelSpk = new QStandardItemModel(this);
    if (!d->analyser->corpus()) return;
    // Create model headers
    QStringList labels;
    labels << "CommunicationID" << "SpeakerID";
    foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
        labels << attr->ID();
    }
    foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
        labels << attr->ID();
    }
    foreach (QString measureID, AnalyserTemporalItem::measureIDsForSpeaker()) labels << measureID;
    d->modelSpk->setHorizontalHeaderLabels(labels);
    foreach (QPointer<CorpusCommunication> com, d->analyser->corpus()->communications()) {
        if (!com) continue;
        if (!d->analyser->item(com->ID())) continue;
        foreach (QString speakerID, d->analyser->item(com->ID())->speakerIDs()) {
            QList<QStandardItem *> itemsSpk;
            QStandardItem *item;
            item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsSpk << item;
            item = new QStandardItem(); item->setData(speakerID, Qt::DisplayRole); itemsSpk << item;
            // properties
            foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
                item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsSpk << item;
            }
            foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
                item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsSpk << item;
            }
            // measures
            foreach (QString measureID, AnalyserTemporalItem::measureIDsForSpeaker()) {
                // analyser->measureDefinitionForSpeaker(measureID).displayNameUnit();
                item = new QStandardItem(); item->setData(d->analyser->item(com->ID())->measureSpk(speakerID, measureID), Qt::DisplayRole); itemsSpk << item;
            }
            d->modelSpk->appendRow(itemsSpk);
        }
    }
}


} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
