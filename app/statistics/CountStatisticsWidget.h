#ifndef COUNTSTATISTICSWIDGET_H
#define COUNTSTATISTICSWIDGET_H

#include <QWidget>

namespace Ui {
class CountStatisticsWidget;
}

struct CountStatisticsWidgetData;

class CountStatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CountStatisticsWidget(QWidget *parent = 0);
    ~CountStatisticsWidget();

private:
    Ui::CountStatisticsWidget *ui;
    CountStatisticsWidgetData *d;
};

#endif // COUNTSTATISTICSWIDGET_H
