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
//    QScopedPointer<AnalyserTemporal>analyser(new AnalyserTemporal);

//    if (ui->optionCommunications->isChecked()) model = modelCom; else model = modelSpk;
//    d->gridviewResults->tableView()->setModel(model);
//    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
//    d->modelResults = model;

}
} // namespace StatisticsPluginBasic
} // namespace Plugins
} // namespace Praaline
