#ifndef PRAALINE_PLUGINS_STATISTICSPLUGINPCA_PCAPLOTWIDGET_H
#define PRAALINE_PLUGINS_STATISTICSPLUGINPCA_PCAPLOTWIDGET_H

#include <QWidget>
#include "qcustomplot.h"

namespace Praaline {

namespace Core {
class CorpusRepository;
}

namespace Plugins {
namespace StatisticsPluginPCA {

namespace Ui {
class PCAPlotWidget;
}

struct PCAPlotWidgetData;

class PCAPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PCAPlotWidget(Core::CorpusRepository *repository, QWidget *parent = 0);
    ~PCAPlotWidget();

private slots:
    void filterAttributeChanged(const QString &attributeName);
    void fileterListSelectionChanged(const QString &attributeValue);
    void plotItemClick(QCPAbstractPlottable*,int,QMouseEvent*);
    void plotItemDoubleClick(QCPAbstractPlottable*,int,QMouseEvent*);

private:
    Ui::PCAPlotWidget *ui;
    PCAPlotWidgetData *d;
};


} // namespace StatisticsPluginPCA
} // namespace Plugins
} // namespace Praaline
#endif // PRAALINE_PLUGINS_STATISTICSPLUGINPCA_PCAPLOTWIDGET_H
