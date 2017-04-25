#ifndef PRAALINE_PLUGINS_STATISTICSPLUGINPROSODY_GLOBALPROSODICPROFILEWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGINPROSODY_GLOBALPROSODICPROFILEWIDGET_H

#include <QWidget>
#include "pncore/statistics/StatisticalMeasureDefinition.h"

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginProsody {

namespace Ui {
class GlobalProsodicProfileWidget;
}

struct GlobalProsodicProfileWidgetData;

class GlobalProsodicProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalProsodicProfileWidget(Core::CorpusRepository *repository, QWidget *parent = 0);
    ~GlobalProsodicProfileWidget();

    static QStringList measureIDs();
    static Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &measureID);

private slots:
    void analyse();

private:
    Ui::GlobalProsodicProfileWidget *ui;
    GlobalProsodicProfileWidgetData *d;

    void createMeasureDefinitionsTableModels();
};


} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline

#endif // PRAALINE_PLUGINS_STATISTICSPLUGINPROSODY_GLOBALPROSODICPROFILEWIDGET_H
