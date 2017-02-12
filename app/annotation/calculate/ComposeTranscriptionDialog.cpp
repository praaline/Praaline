#include "ComposeTranscriptionDialog.h"
#include "ui_ComposeTranscriptionDialog.h"

ComposeTranscriptionDialog::ComposeTranscriptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComposeTranscriptionDialog)
{
    ui->setupUi(this);
}

ComposeTranscriptionDialog::~ComposeTranscriptionDialog()
{
    delete ui;
}
