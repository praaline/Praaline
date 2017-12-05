#include <QString>
#include <QPointer>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "DecodeFilenameToMetadataDialog.h"
#include "ui_DecodeFilenameToMetadataDialog.h"

struct DecodeFilenameToMetadataDialogData {
    QPointer<Corpus> corpus;
};

DecodeFilenameToMetadataDialog::DecodeFilenameToMetadataDialog(QPointer<Corpus> corpus, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DecodeFilenameToMetadataDialog),
    d(new DecodeFilenameToMetadataDialogData())
{
    ui->setupUi(this);
    d->corpus = corpus;
    connect(ui->commandClose, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->commandAdd, SIGNAL(clicked(bool)), this, SLOT(addCorrespondance()));
    connect(ui->commandRemove, SIGNAL(clicked(bool)), this, SLOT(removeCorrespondance()));
    connect(ui->commandUpdateMetadata, SIGNAL(clicked(bool)), this, SLOT(updateMetadata()));
    connect(ui->comboBoxOperation, SIGNAL(currentIndexChanged(int)), this, SLOT(operationChanged(int)));
    // Load metadata attributes in current corpus

}

DecodeFilenameToMetadataDialog::~DecodeFilenameToMetadataDialog()
{
    delete ui;
    delete d;
}

void DecodeFilenameToMetadataDialog::operationChanged(int operation)
{

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
