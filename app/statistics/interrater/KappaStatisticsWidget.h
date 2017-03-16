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

struct KappaStatisticsWidgetData;

class KappaStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KappaStatisticsWidget(Core::CorpusRepository *repository, QWidget *parent = 0);
    ~KappaStatisticsWidget();

private slots:
    void annotationLevelChanged(const QString &text);
    void analyse();

private:
    Ui::KappaStatisticsWidget *ui;
    KappaStatisticsWidgetData *d;

    void defineClassesForCohenKappa();
    void analyseCohenKappa();
};


} // namespace StatisticsPluginInterrater
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGININTERRATER_KAPPASTATISTICSWIDGET_H
