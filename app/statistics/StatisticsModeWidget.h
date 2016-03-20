#ifndef STATISTICSMODEWIDGET_H
#define STATISTICSMODEWIDGET_H

#include <QWidget>

namespace Ui {
class StatisticsModeWidget;
}

class StatisticsModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticsModeWidget(QWidget *parent = 0);
    ~StatisticsModeWidget();

signals:
    void activateMode();

private slots:

private:
    Ui::StatisticsModeWidget *ui;
};

#endif // STATISTICSMODEWIDGET_H
