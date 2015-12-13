#include "transcriberwidget.h"
#include "ui_transcriberwidget.h"

struct TranscriberWidgetData {
    TranscriberWidgetData() {}

};

TranscriberWidget::TranscriberWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TranscriberWidget), d(new TranscriberWidgetData)
{
    ui->setupUi(this);
}

TranscriberWidget::~TranscriberWidget()
{
    delete ui;
    delete d;
}

