#ifndef ADVANCEDQUERIESWIDGET_H
#define ADVANCEDQUERIESWIDGET_H

#include <QMainWindow>

namespace Ui {
class AdvancedQueriesWidget;
}

class AdvancedQueriesWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdvancedQueriesWidget(QWidget *parent = 0);
    ~AdvancedQueriesWidget();

private:
    Ui::AdvancedQueriesWidget *ui;
};

#endif // ADVANCEDQUERIESWIDGET_H
