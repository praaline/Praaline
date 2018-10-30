#include "CorpusExplorerTreeWidget.h"
#include "ui_CorpusExplorerTreeWidget.h"

CorpusExplorerTreeWidget::CorpusExplorerTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusExplorerTreeWidget)
{
    ui->setupUi(this);
}

CorpusExplorerTreeWidget::~CorpusExplorerTreeWidget()
{
    delete ui;
}
