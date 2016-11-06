#include "ExportMetadataWizard.h"
#include "ui_ExportMetadataWizard.h"

#include "pncore/corpus/Corpus.h"

struct ExportMetadataWizardData {
    ExportMetadataWizardData() : corpus(0) {}

    QPointer<Corpus> corpus;
};

ExportMetadataWizard::ExportMetadataWizard(Corpus *corpus, QWidget *parent) :
    QWizard(parent), ui(new Ui::ExportMetadataWizard), d(new ExportMetadataWizardData)
{
    ui->setupUi(this);
    d->corpus = corpus;
}

ExportMetadataWizard::~ExportMetadataWizard()
{
    delete ui;
    delete d;
}

