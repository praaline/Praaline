#include <QDebug>
#include <QPointer>
#include <QApplication>
#include <QProgressBar>
#include <QStandardItemModel>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Corpus/CorpusSpeaker.h"
#include "PraalineCore/Structure/MetadataStructure.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "StatisticsPluginBasicWidget.h"
#include "ui_StatisticsPluginBasicWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginBasic {

struct StatisticsPluginBasicWidgetData {
    StatisticsPluginBasicWidgetData() :
        repository(0), modelResults(0), gridviewResults(0)
    {}

    // Data
    CorpusRepository *repository;
    QStringList levelIDs;
    QStringList attributeIDs;
    QMap<QString, QHash<QString, long long> > dataCounts;
    // Presentation
    QStandardItemModel *modelResults;
    GridViewWidget *gridviewResults;
};

StatisticsPluginBasicWidget::StatisticsPluginBasicWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::StatisticsPluginBasicWidget), d(new StatisticsPluginBasicWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Corpora combobox
    ui->comboBoxCorpus->addItems(repository->listCorporaIDs());
    // Metadata attributes
    int i = 0;
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Communication)){
        ui->comboBoxMetadata->insertItem(i, attr->name(), false);
        ++i;
    }
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)){
        ui->comboBoxMetadata->insertItem(i, attr->name(), false);
        ++i;
    }
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Results grid view
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);
    // Default
    ui->optionOrientationVertical->setChecked(true);
    connect(ui->optionOrientationVertical, SIGNAL(clicked(bool)), this, SLOT(updateTable()));
    connect(ui->optionOrientationHorizontal, SIGNAL(clicked(bool)), this, SLOT(updateTable()));
}

StatisticsPluginBasicWidget::~StatisticsPluginBasicWidget()
{
    delete ui;
    delete d;
}

void StatisticsPluginBasicWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;

    d->levelIDs = d->repository->annotationStructure()->levelIDs();
    d->attributeIDs.clear();
    d->attributeIDs << "annotationID" << "speakerID";
    // Exclude pauses from counts
    QStringList valuesForSilentPause;
    valuesForSilentPause << "" << "_" << "#";

    foreach (QString levelID, d->levelIDs) {
        QList<QPair<QList<QVariant>, long long> > counts = d->repository->annotations()->countItems(levelID, d->attributeIDs, true, valuesForSilentPause);
        QPair<QList<QVariant>, long long> count;
        foreach (count, counts) {
            // ID = Annotation ID \tab Speaker ID
            QString id = count.first.at(0).toString() + "\t" + count.first.at(1).toString();
            d->dataCounts[id].insert(levelID, count.second);
        }
    }
    updateTable();
}

void StatisticsPluginBasicWidget::updateTable()
{
    // Select orientation
    Qt::Orientation orientation = Qt::Vertical;
    if (ui->optionOrientationHorizontal->isChecked()) orientation = Qt::Horizontal;
    // Create new model
    QStandardItemModel *model = new QStandardItemModel(this);
    // Build header
    QStringList header;

    header << "Annotation ID" << "Speaker ID " << d->levelIDs;
    if (orientation == Qt::Vertical) {
        model->setHorizontalHeaderLabels(header);
        model->setColumnCount(d->levelIDs.count() + 2);
    } else {
        QList<QStandardItem *> headerItems;
        foreach (QString headerField, header) headerItems << new QStandardItem(headerField);
        model->appendColumn(headerItems);
        model->setRowCount(d->levelIDs.count() + 2);
    }
    // Build table model
    foreach (QString id, d->dataCounts.keys()) {
        QList<QStandardItem *> items;
        foreach (QString idpart, id.split("\t"))
            items << new QStandardItem(idpart);
        foreach (QString levelID, d->levelIDs) {
            QStandardItem *item = new QStandardItem();
            item->setData(d->dataCounts.value(id).value(levelID), Qt::DisplayRole);
            items << item;
        }
        if (orientation == Qt::Vertical)
            model->appendRow(items);
        else
            model->appendColumn(items);
    }
    // Show table model
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
    d->modelResults = model;
}

} // namespace StatisticsPluginBasic
} // namespace Plugins
} // namespace Praaline
