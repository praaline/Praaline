#include <QDebug>
#include <QApplication>
#include <QProgressBar>
#include <QStandardItemModel>

#include "GlobalProsodicProfileWidget.h"
#include "ui_GlobalProsodicProfileWidget.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginProsody {

struct GlobalProsodicProfileWidgetData {
    GlobalProsodicProfileWidgetData() :
        repository(0), gridviewResults(0), modelResults(0)
    {}

    CorpusRepository *repository;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
};

GlobalProsodicProfileWidget::GlobalProsodicProfileWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::GlobalProsodicProfileWidget), d(new GlobalProsodicProfileWidgetData)
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
}

GlobalProsodicProfileWidget::~GlobalProsodicProfileWidget()
{
    delete ui;
    delete d;
}

void GlobalProsodicProfileWidget::analyse()
{

}

} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline
