#ifndef PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H

#include <QWidget>

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginTemporal {

namespace Ui {
class PauseLengthDistributionWIdget;
}

class AnalyserTemporal;
struct PauseLengthDistributionWidgetData;

class PauseLengthDistributionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PauseLengthDistributionWidget(Core::CorpusRepository *repository, AnalyserTemporal *analyser,
                                           QWidget *parent = 0);
    ~PauseLengthDistributionWidget();

private:
    Ui::PauseLengthDistributionWIdget *ui;
    PauseLengthDistributionWidgetData *d;
};


} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H
