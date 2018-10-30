#include "CreateCorpusCommunicationDialog.h"
#include "ui_CreateCorpusCommunicationDialog.h"

CreateCorpusCommunicationDialog::CreateCorpusCommunicationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateCorpusCommunicationDialog)
{
    ui->setupUi(this);
}

CreateCorpusCommunicationDialog::~CreateCorpusCommunicationDialog()
{
    delete ui;
}
