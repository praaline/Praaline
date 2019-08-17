#ifndef PRAALINE_PLUGINS_STATISTICSPLUGINBASIC_STATISTICSPLUGINBASICWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGINBASIC_STATISTICSPLUGINBASICWIDGET_H

#include <QWidget>

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginBasic {

namespace Ui {
class StatisticsPluginBasicWidget;
}

struct StatisticsPluginBasicWidgetData;

class StatisticsPluginBasicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsPluginBasicWidget(Core::CorpusRepository *repository, QWidget *parent = nullptr);
    ~StatisticsPluginBasicWidget();

private slots:
    void analyse();
    void updateTable();

private:
    Ui::StatisticsPluginBasicWidget *ui;
    StatisticsPluginBasicWidgetData *d;
};


} // namespace StatisticsPluginBasic
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGINBASIC_STATISTICSPLUGINBASICWIDGET_H
