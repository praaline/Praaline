#ifndef PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H

#include <QWidget>
#include <QChart>
QT_CHARTS_USE_NAMESPACE

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginTemporal {

namespace Ui {
class PauseLengthDistributionWidget;
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

private slots:
    void drawCharts();

private:
    Ui::PauseLengthDistributionWidget *ui;
    PauseLengthDistributionWidgetData *d;

    QChart *drawHistogram(const QString &title, QMap<QString, QList<double> > aggregates);
};


} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGINTEMPORAL_PAUSELENGTHDISTRIBUTIONWIDGET_H
