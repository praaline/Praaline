#ifndef STATISTICSPLUGINTEMPORALWIDGET_H
#define STATISTICSPLUGINTEMPORALWIDGET_H

#include <QWidget>

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginTemporal {

namespace Ui {
class AnalyserTemporalWidget;
}

struct AnalyserTemporalWidgetData;

class AnalyserTemporalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyserTemporalWidget(Core::CorpusRepository *repository, QWidget *parent = 0);
    ~AnalyserTemporalWidget();

private slots:
    void analyse();

private:
    Ui::AnalyserTemporalWidget *ui;
    AnalyserTemporalWidgetData *d;
};

} // namespace StatisticalPluginTemporal
} // namespace Plugins
} // namespace Praaline


#endif // STATISTICSPLUGINTEMPORALWIDGET_H
