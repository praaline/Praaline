#include "AddCorpusCommunicationDialog.h"
#include "ui_AddCorpusCommunicationDialog.h"

struct AddCorpusCommunicationDialogData {
    int i;
};

AddCorpusCommunicationDialog::AddCorpusCommunicationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCorpusCommunicationDialog)
{
    ui->setupUi(this);
}

AddCorpusCommunicationDialog::~AddCorpusCommunicationDialog()
{
    delete ui;
}

QString AddCorpusCommunicationDialog::corpusID() const
{
    return ui->editCorpusID->text();
}

void AddCorpusCommunicationDialog::setCorpusID(const QString &corpusID)
{
    ui->editCorpusID->setText(corpusID);
}

QString AddCorpusCommunicationDialog::communicationID() const
{
    return ui->editCommunicationID->text();
}

void AddCorpusCommunicationDialog::setCommunicationID(const QString &communicationID)
{
    ui->editCommunicationID->setText(communicationID);
}

QStringList AddCorpusCommunicationDialog::mediaFilesList() const
{
    QStringList filelist;
    return filelist;
}

QStringList AddCorpusCommunicationDialog::annotationFilesList() const
{
    QStringList filelist;
    return filelist;
}
