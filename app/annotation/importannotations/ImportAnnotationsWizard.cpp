#include "ImportTableDataPreviewWizardPage.h"

#include "ImportAnnotationsWizard.h"
#include "ui_ImportAnnotationsWizard.h"

struct ImportAnnotationsWizardData {
    ImportTableDataPreviewWizardPage *pageTableDataPreview;
};

ImportAnnotationsWizard::ImportAnnotationsWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::ImportAnnotationsWizard), d(new ImportAnnotationsWizardData)
{
    ui->setupUi(this);
    d->pageTableDataPreview = new ImportTableDataPreviewWizardPage(this);
    addPage(d->pageTableDataPreview);
}

ImportAnnotationsWizard::~ImportAnnotationsWizard()
{
    delete ui;
    delete d;
}
