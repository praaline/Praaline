#ifndef STATISTICSPLUGINDisfluenciesWIDGET_H
#define STATISTICSPLUGINDisfluenciesWIDGET_H

#include <QWidget>

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginDisfluencies {

namespace Ui {
class AnalyserDisfluenciesWidget;
}

class AnalyserDisfluencies;
struct AnalyserDisfluenciesWidgetData;

class AnalyserDisfluenciesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyserDisfluenciesWidget(Core::CorpusRepository *repository, AnalyserDisfluencies *analyser,
                                    QWidget *parent = nullptr);
    ~AnalyserDisfluenciesWidget();

private slots:
    void madeProgress(int);
    void analyse();
    void changeDisplayedModel();
    void showAnalysisForCom();
    void showAnalysisForSpk();
    void drawChart();

private:
    Ui::AnalyserDisfluenciesWidget *ui;
    AnalyserDisfluenciesWidgetData *d;

    void buildModelForCom();
    void buildModelForSpk();
    void createMeasureDefinitionsTableModels();
};

} // namespace StatisticalPluginDisfluencies
} // namespace Plugins
} // namespace Praaline


#endif // STATISTICSPLUGINDisfluenciesWIDGET_H
