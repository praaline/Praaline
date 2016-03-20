#include "AnnotationDataBrowserWidget.h"
#include "ui_AnnotationDataBrowserWidget.h"

AnnotationDataBrowserWidget::AnnotationDataBrowserWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AnnotationDataBrowserWidget)
{
    ui->setupUi(this);
}

AnnotationDataBrowserWidget::~AnnotationDataBrowserWidget()
{
    delete ui;
}
