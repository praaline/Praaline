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

class AnalyserTemporal;
struct AnalyserTemporalWidgetData;

class AnalyserTemporalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyserTemporalWidget(Core::CorpusRepository *repository, AnalyserTemporal *analyser,
                                    QWidget *parent = nullptr);
    ~AnalyserTemporalWidget();

private slots:
    void madeProgress(int);
    void analyse();
    void changeDisplayedModel();
    void showAnalysisForCom();
    void showAnalysisForSpk();
    void drawChart();

private:
    Ui::AnalyserTemporalWidget *ui;
    AnalyserTemporalWidgetData *d;

    void buildModelForCom();
    void buildModelForSpk();
    void createMeasureDefinitionsTableModels();
};

} // namespace StatisticalPluginTemporal
} // namespace Plugins
} // namespace Praaline


#endif // STATISTICSPLUGINTEMPORALWIDGET_H
