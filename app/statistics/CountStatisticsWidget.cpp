#include "CountStatisticsWidget.h"
#include "ui_CountStatisticsWidget.h"

struct CountStatisticsWidgetData {

};

CountStatisticsWidget::CountStatisticsWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::CountStatisticsWidget), d(new CountStatisticsWidgetData())
{
    ui->setupUi(this);
}

CountStatisticsWidget::~CountStatisticsWidget()
{
    delete ui;
    delete d;
}

