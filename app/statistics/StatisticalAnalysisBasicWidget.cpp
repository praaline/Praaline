#include "StatisticalAnalysisBasicWidget.h"
#include "ui_StatisticalAnalysisBasicWidget.h"

StatisticalAnalysisBasicWidget::StatisticalAnalysisBasicWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticalAnalysisBasicWidget)
{
    ui->setupUi(this);
}

StatisticalAnalysisBasicWidget::~StatisticalAnalysisBasicWidget()
{
    delete ui;
}
