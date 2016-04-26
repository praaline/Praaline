#include "CreateDatasetWidget.h"
#include "ui_CreateDatasetWidget.h"

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
