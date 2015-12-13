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
};

#endif // GLOBALVISUALISATIONWIDGET_H
