#include "WelcomeModeWidget.h"
#include "ui_WelcomeModeWidget.h"

WelcomeModeWidget::WelcomeModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WelcomeModeWidget)
{
    ui->setupUi(this);
}

WelcomeModeWidget::~WelcomeModeWidget()
{
    delete ui;
}
