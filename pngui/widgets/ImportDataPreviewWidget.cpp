#include "ImportDataPreviewWidget.h"
#include "ui_ImportDataPreviewWidget.h"

struct ImportDataPreviewWidgetData {

};

ImportDataPreviewWidget::ImportDataPreviewWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::ImportDataPreviewWidget), d(new ImportDataPreviewWidgetData)
{
    ui->setupUi(this);
}

ImportDataPreviewWidget::~ImportDataPreviewWidget()
{
    delete ui;
    delete d;
}
