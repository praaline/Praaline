#ifndef VISUALISATIONMODEWIDGET_H
#define VISUALISATIONMODEWIDGET_H

#include <QWidget>

namespace Ui {
class VisualisationModeWidget;
}

struct VisualisationModeWidgetData;

class VisualisationModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualisationModeWidget(QWidget *parent = nullptr);
    ~VisualisationModeWidget();

signals:
    void activateMode();

private slots:
    void showTimelineVisualisation();
    void showGlobalVisualisation();

private:
    Ui::VisualisationModeWidget *ui;
    VisualisationModeWidgetData *d;

    void setupActions();

};

#endif // VISUALISATIONMODEWIDGET_H
