#include "ExportMetadataWizard.h"
#include "ui_exportmetadatawizard.h"

#include "pncore/corpus/corpus.h"

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

