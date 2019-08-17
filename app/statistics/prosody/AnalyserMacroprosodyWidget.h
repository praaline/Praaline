#ifndef ANALYSERMACROPROSODYWIDGET_H
#define ANALYSERMACROPROSODYWIDGET_H

#include <QWidget>

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginProsody {

namespace Ui {
class AnalyserMacroprosodyWidget;
}

struct AnalyserMacroprosodyWidgetData;

class AnalyserMacroprosodyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyserMacroprosodyWidget(Core::CorpusRepository *repository, QWidget *parent = nullptr);
    ~AnalyserMacroprosodyWidget();

private slots:
    void openFileMUList();
    void analyse();

private:
    Ui::AnalyserMacroprosodyWidget *ui;
    AnalyserMacroprosodyWidgetData *d;

    void createMeasureDefinitionsTableModels();
};


} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline

#endif // ANALYSERMACROPROSODYWIDGET_H
