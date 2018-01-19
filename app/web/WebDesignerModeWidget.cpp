#include "WebDesignerModeWidget.h"
#include "ui_WebDesignerModeWidget.h"

WebDesignerModeWidget::WebDesignerModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WebDesignerModeWidget)
{
    ui->setupUi(this);
}

WebDesignerModeWidget::~WebDesignerModeWidget()
{
    delete ui;
}
