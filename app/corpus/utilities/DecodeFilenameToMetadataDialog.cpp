#include "DecodeFilenameToMetadataDialog.h"
#include "ui_DecodeFilenameToMetadataDialog.h"

struct DecodeFilenameToMetadataDialog {

};

DecodeFilenameToMetadataDialog::DecodeFilenameToMetadataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DecodeFilenameToMetadataDialog),
    d(new DecodeFilenameToMetadataDialog())
{
    ui->setupUi(this);
    connect(ui->commandClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->commandAdd, SIGNAL(clicked(bool)), this, SLOT(addCorrespondance()));
    connect(ui->commandRemove, SIGNAL(clicked(bool)), this, SLOT(removeCorrespondance()));
    connect(ui->commandUpdateMetadata, SIGNAL(clicked(bool)), this, SLOT(updateMetadata()));
}

DecodeFilenameToMetadataDialog::~DecodeFilenameToMetadataDialog()
{
    delete ui;
}

void DecodeFilenameToMetadataDialog::addCorrespondance()
{

}

void DecodeFilenameToMetadataDialog::removeCorrespondance()
{

}

void DecodeFilenameToMetadataDialog::updateMetadata()
{

}
