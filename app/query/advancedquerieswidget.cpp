#include "advancedquerieswidget.h"
#include "ui_advancedquerieswidget.h"

AdvancedQueriesWidget::AdvancedQueriesWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AdvancedQueriesWidget)
{
    ui->setupUi(this);
}

AdvancedQueriesWidget::~AdvancedQueriesWidget()
{
    delete ui;
}
