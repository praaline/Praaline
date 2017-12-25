#ifndef GLOBALVISUALISATIONWIDGET_H
#define GLOBALVISUALISATIONWIDGET_H

#include <QMainWindow>

namespace Ui {
class GlobalVisualisationWidget;
}

class GlobalVisualisationWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit GlobalVisualisationWidget(QWidget *parent = 0);
    ~GlobalVisualisationWidget();

private:
    Ui::GlobalVisualisationWidget *ui;
    void test1();
    void test2();
    void test3();
};

#endif // GLOBALVISUALISATIONWIDGET_H
