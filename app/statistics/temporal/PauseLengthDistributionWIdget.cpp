#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

#include "AnalyserTemporal.h"

#include "PauseLengthDistributionWIdget.h"
#include "ui_PauseLengthDistributionWIdget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct PauseLengthDistributionWidgetData {
    PauseLengthDistributionWidgetData() :
        repository(0), analyser(0)
    {}

    CorpusRepository *repository;
    AnalyserTemporal *analyser;
};

PauseLengthDistributionWidget::PauseLengthDistributionWidget(CorpusRepository *repository, AnalyserTemporal *analyser, QWidget *parent) :
    QWidget(parent), ui(new Ui::PauseLengthDistributionWIdget), d(new PauseLengthDistributionWidgetData)
{
    ui->setupUi(this);
    d->repository = repository;
    d->analyser = analyser;
}

PauseLengthDistributionWidget::~PauseLengthDistributionWidget()
{
    delete ui;
    delete d;
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
