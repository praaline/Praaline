#ifndef CREATEDATASETWIDGET_H
#define CREATEDATASETWIDGET_H

#include <QMainWindow>

namespace Ui {
class CreateDatasetWidget;
}

class CreateDatasetWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit CreateDatasetWidget(QWidget *parent = 0);
    ~CreateDatasetWidget();

private:
    Ui::CreateDatasetWidget *ui;
};

#endif // CREATEDATASETWIDGET_H
