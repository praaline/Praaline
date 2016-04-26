#include "ScriptingModeWidget.h"
#include "ui_ScriptingModeWidget.h"

ScriptingModeWidget::ScriptingModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptingModeWidget)
{
    ui->setupUi(this);
}

ScriptingModeWidget::~ScriptingModeWidget()
{
    delete ui;
}
