#include <QDebug>
#include <QApplication>
#include <QProgressBar>
#include <QStandardItemModel>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "StatisticsPluginBasicWidget.h"
#include "ui_StatisticsPluginBasicWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginBasic {

struct StatisticsPluginBasicWidgetData {
    StatisticsPluginBasicWidgetData() :
        repository(0), gridviewResults(0), modelResults(0)
    {}

    CorpusRepository *repository;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
};

StatisticsPluginBasicWidget::StatisticsPluginBasicWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::StatisticsPluginBasicWidget), d(new StatisticsPluginBasicWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
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

    QStringList levelIDs = d->repository->annotationStructure()->levelIDs();
    QStringList attributeIDs; attributeIDs << "annotationID" << "speakerID";
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << "Annotation ID" << "Speaker ID " << levelIDs);
    model->setColumnCount(levelIDs.count() + 2);
    QMap<QString, QHash<QString, long long> > table;
    foreach (QString levelID, levelIDs) {
        QList<QPair<QList<QVariant>, long long> > counts = d->repository->annotations()->countItems(levelID, attributeIDs);
        QPair<QList<QVariant>, long long> count;
        foreach (count, counts) {
            QString id = count.first.at(0).toString() + "\t" + count.first.at(1).toString();
            table[id].insert(levelID, count.second);
        }
    }
    // build table model
    foreach (QString id, table.keys()) {
        QList<QStandardItem *> items;
        items << new QStandardItem(id.section("\t", 0, 0));
        items << new QStandardItem(id.section("\t", 1, 1));
        foreach (QString levelID, levelIDs) {
            QStandardItem *item = new QStandardItem();
            item->setData(table.value(id).value(levelID), Qt::DisplayRole);
            items << item;
        }
        model->appendRow(items);
    }
    // show table model
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
    d->modelResults = model;

}

} // namespace StatisticsPluginBasic
} // namespace Plugins
} // namespace Praaline
