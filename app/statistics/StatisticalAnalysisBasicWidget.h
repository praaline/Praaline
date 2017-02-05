#ifndef STATISTICALANALYSISBASICWIDGET_H
#define STATISTICALANALYSISBASICWIDGET_H

#include <QWidget>

namespace Ui {
class StatisticalAnalysisBasicWidget;
}

class StatisticalAnalysisBasicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatisticalAnalysisBasicWidget(QWidget *parent = 0);
    ~StatisticalAnalysisBasicWidget();

private:
    Ui::StatisticalAnalysisBasicWidget *ui;
};

#endif // STATISTICALANALYSISBASICWIDGET_H
