#include <QApplication>
#include <QProgressBar>
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
#include "AnalyserTemporalWidgetWidget.h"
#include "ui_AnalyserTemporalWidgetWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct AnalyserTemporalWidgetData {
    AnalyserTemporalWidgetData() :
        repository(0), gridviewResults(0), modelResults(0)
    {}

    CorpusRepository *repository;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
};

AnalyserTemporalWidget::AnalyserTemporalWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::AnalyserTemporalWidget), d(new AnalyserTemporalWidgetData)
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

AnalyserTemporalWidget::~AnalyserTemporalWidget()
{
    delete ui;
    delete d;
}

void AnalyserTemporalWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;
    QScopedPointer<AnalyserTemporal>analyser(new AnalyserTemporal);
    // Pause lists
//    QFile filePauseListSIL(corpus->basePath() + "/pauselist_SIL.txt");
//    if ( !filePauseListSIL.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
//    QTextStream pauseListSIL(&filePauseListSIL);
//    pauseListSIL.setCodec("UTF-8");
//    QFile filePauseListFIL(corpus->basePath() + "/pauselist_FIL.txt");
//    if ( !filePauseListFIL.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
//    QTextStream pauseListFIL(&filePauseListFIL);
//    pauseListFIL.setCodec("UTF-8");

    // Models available
    QStandardItemModel *modelCom = new QStandardItemModel(this);
    QStandardItemModel *modelSpk = new QStandardItemModel(this);
    // Create model headers
    QStringList labels;
    labels << "CommunicationID";
    foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication))
        labels << attr->ID();
    foreach (QString measureID, analyser->measureIDsForCommunication()) labels << measureID;
    modelCom->setHorizontalHeaderLabels(labels);
    labels.clear();
    labels << "CommunicationID" << "SpeakerID";
//    pauseListSIL << "CommunicationID\tSpeakerID\t";
//    pauseListFIL << "CommunicationID\tSpeakerID\t";
    foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
        labels << attr->ID();
//        pauseListSIL << attr->ID() << "\t";
//        pauseListFIL << attr->ID() << "\t";
    }
    foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)) {
        labels << attr->ID();
    }
    foreach (QString measureID, analyser->measureIDsForSpeaker()) labels << measureID;
    modelSpk->setHorizontalHeaderLabels(labels);
//    pauseListSIL << "Duration\tDurationRel1\tDurationRel2\tDurationRel3\tDurationRel4\tDurationRel5\n";
//    pauseListFIL << "Duration\n";
    QTextStream SIL;
    QTextStream FIL;
    // Analyse communications / and then speakers
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(corpus->communicationsCount());
    int i = 1;
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        analyser->calculate(com, SIL, FIL); // ,pauseListSIL, pauseListFIL);
        QList<QStandardItem *> itemsCom;
        QStandardItem *item;
        item = new QStandardItem(); item->setData(com->ID(), Qt::DisplayRole); itemsCom << item;
        // properties
        foreach (QPointer<MetadataStructureAttribute> attr, d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication)) {
            item = new QStandardItem(); item->setData(com->property(attr->ID()), Qt::DisplayRole); itemsCom << item;
        }
        // measures
        foreach (QString measureID, analyser->measureIDsForCommunication()) {
            // analyser->measureDefinitionForCommunication(measureID).displayNameUnit()
            item = new QStandardItem(); item->setData(analyser->measureCom(measureID), Qt::DisplayRole); itemsCom << item;
        }
        modelCom->appendRow(itemsCom);

        foreach (QString speakerID, analyser->speakerIDs()) {
            QList<QStandardItem *> itemsSpk;
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
            foreach (QString measureID, analyser->measureIDsForSpeaker()) {
                // analyser->measureDefinitionForSpeaker(measureID).displayNameUnit();
                item = new QStandardItem(); item->setData(analyser->measureSpk(speakerID, measureID), Qt::DisplayRole); itemsSpk << item;
            }
            modelSpk->appendRow(itemsSpk);
        }
        ui->progressBar->setValue(i);
        QApplication::processEvents();
    }
    ui->progressBar->setValue(ui->progressBar->maximum());
    QApplication::processEvents();

    // Update table headers
    //    for (int i = 0; i < modelCom->columnCount(); ++i)
    //        model->setHorizontalHeaderItem(i, new QStandardItem(analyser->model()->horizontalHeaderItem(i)->text()));
    // Update table
    QStandardItemModel *model(0);
    if (ui->optionCommunications->isChecked()) model = modelCom; else model = modelSpk;
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
    d->modelResults = model;

//    filePauseListSIL.close();
//    filePauseListFIL.close();
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
