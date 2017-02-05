#ifndef PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H

#include <QWidget>

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

namespace Ui {
class PauseLengthDistributionWIdget;
}

class PauseLengthDistributionWIdget : public QWidget
{
    Q_OBJECT

public:
    explicit PauseLengthDistributionWIdget(QWidget *parent = 0);
    ~PauseLengthDistributionWIdget();

private:
    Ui::PauseLengthDistributionWIdget *ui;
};


} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H
