#include "createdatasetwidget.h"
#include "ui_createdatasetwidget.h"

CreateDatasetWidget::CreateDatasetWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CreateDatasetWidget)
{
    ui->setupUi(this);
}

CreateDatasetWidget::~CreateDatasetWidget()
{
    delete ui;
}
