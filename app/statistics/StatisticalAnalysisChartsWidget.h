#ifndef STATISTICALANALYSISCHARTSWIDGET_H
#define STATISTICALANALYSISCHARTSWIDGET_H

#include <QWidget>
#include "StatisticalAnalyserBase.h"

namespace Ui {
class StatisticalAnalysisChartsWidget;
}

namespace Praaline {
namespace Core {
class CorpusRepository;
}
}

struct StatisticalAnalysisChartsWidgetData;

class StatisticalAnalysisChartsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticalAnalysisChartsWidget(Praaline::Core::CorpusRepository *repository, StatisticalAnalyserBase *analyser,
                                             QWidget *parent = nullptr);
    ~StatisticalAnalysisChartsWidget();

    void showMeasuresForCom();
    void showMeasuresForSpk();

private slots:
    void drawChart();

private:
    Ui::StatisticalAnalysisChartsWidget *ui;
    StatisticalAnalysisChartsWidgetData *d;
};

#endif // STATISTICALANALYSISCHARTSWIDGET_H
