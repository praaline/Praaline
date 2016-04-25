#ifndef STATISTICSMODEWIDGET_H
#define STATISTICSMODEWIDGET_H

#include <QWidget>

namespace Ui {
class StatisticsModeWidget;
}

struct StatisticsModeWidgetData;

class StatisticsModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsModeWidget(QWidget *parent = 0);
    ~StatisticsModeWidget();

signals:
    void activateMode();

private slots:
    void analyse();
    void actionSaveOutput();
    void actionClearOutput();

private:
    Ui::StatisticsModeWidget *ui;
    StatisticsModeWidgetData *d;
};

#endif // STATISTICSMODEWIDGET_H
