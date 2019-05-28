#include "AddCorpusRecordingDialog.h"
#include "ui_AddCorpusRecordingDialog.h"

AddCorpusRecordingDialog::AddCorpusRecordingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCorpusRecordingDialog)
{
    ui->setupUi(this);
}

AddCorpusRecordingDialog::~AddCorpusRecordingDialog()
{
    delete ui;
}
