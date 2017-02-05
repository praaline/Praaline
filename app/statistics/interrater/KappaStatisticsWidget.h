#ifndef PRAALINE_PLUGINS_STATISTICSPLUGININTERRATER_KAPPASTATISTICSWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGININTERRATER_KAPPASTATISTICSWIDGET_H

#include <QWidget>

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginInterrater {

namespace Ui {
class KappaStatisticsWidget;
}

class KappaStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KappaStatisticsWidget(Core::CorpusRepository *repository, QWidget *parent = 0);
    ~KappaStatisticsWidget();

private:
    Ui::KappaStatisticsWidget *ui;
};


} // namespace StatisticsPluginInterrater
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGININTERRATER_KAPPASTATISTICSWIDGET_H
