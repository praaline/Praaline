#include "KappaStatisticsWidget.h"
#include "ui_KappaStatisticsWidget.h"

#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginInterrater {

KappaStatisticsWidget::KappaStatisticsWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KappaStatisticsWidget)
{
    ui->setupUi(this);
}

KappaStatisticsWidget::~KappaStatisticsWidget()
{
    delete ui;
}

} // namespace StatisticsPluginInterrater
} // namespace Plugins
} // namespace Praaline
