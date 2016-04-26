#include "GlobalVisualisationWidget.h"
#include "ui_GlobalVisualisationWidget.h"

GlobalVisualisationWidget::GlobalVisualisationWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GlobalVisualisationWidget)
{
    ui->setupUi(this);
}

GlobalVisualisationWidget::~GlobalVisualisationWidget()
{
    delete ui;
}
