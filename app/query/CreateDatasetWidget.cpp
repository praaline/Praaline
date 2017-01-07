#include <QProgressBar>
#include <QCommandLinkButton>

#include "CreateDatasetWidget.h"
#include "ui_CreateDatasetWidget.h"

#include "dataseteditor/DatasetEditorWidget.h"

struct CreateDatasetWidgetData {
    DatasetEditorWidget *editor;
    QProgressBar *progressbar;
    QCommandLinkButton *commandCreateDataset;
};

CreateDatasetWidget::CreateDatasetWidget(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::CreateDatasetWidget), d(new CreateDatasetWidgetData)
{
    ui->setupUi(this);

    d->editor = new DatasetEditorWidget(this);
    d->progressbar = new QProgressBar(this);
    d->commandCreateDataset = new QCommandLinkButton(tr("Create Dataset"), this);

    ui->gridLayoutDatasetEditor->addWidget(d->editor, 0, 0, 1, 5);
    ui->gridLayoutDatasetEditor->addWidget(d->commandCreateDataset, 1, 0, 1, 1);
    ui->gridLayoutDatasetEditor->addWidget(d->progressbar, 1, 1, 1, 4);

    ui->splitter->setSizes(QList<int>() << 300 << 100);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 1);
}

CreateDatasetWidget::~CreateDatasetWidget()
{
    delete ui;
    delete d;
}
