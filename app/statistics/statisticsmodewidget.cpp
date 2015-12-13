#include <QDebug>
#include "statisticsmodewidget.h"
#include "ui_statisticsmodewidget.h"

#include "interrateragreement.h"
#include "pngui/observers/corpusobserver.h"
#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtier.h"

StatisticsModeWidget::StatisticsModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticsModeWidget)
{
    ui->setupUi(this);
    ui->splitterLeftRight->setSizes(QList<int>() << 50 << 300);
}

StatisticsModeWidget::~StatisticsModeWidget()
{
    delete ui;
}
