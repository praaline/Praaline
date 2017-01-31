#include "ExportMetadataWizard.h"
#include "ui_ExportMetadataWizard.h"

#include "pncore/corpus/Corpus.h"

struct ExportMetadataWizardData {
    ExportMetadataWizardData() {}
};

ExportMetadataWizard::ExportMetadataWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::ExportMetadataWizard), d(new ExportMetadataWizardData)
{
    ui->setupUi(this);
}

ExportMetadataWizard::~ExportMetadataWizard()
{
    delete ui;
    delete d;
}

