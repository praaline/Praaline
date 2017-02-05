#include "PauseLengthDistributionWIdget.h"
#include "ui_PauseLengthDistributionWIdget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

PauseLengthDistributionWIdget::PauseLengthDistributionWIdget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PauseLengthDistributionWIdget)
{
    ui->setupUi(this);
}

PauseLengthDistributionWIdget::~PauseLengthDistributionWIdget()
{
    delete ui;
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
