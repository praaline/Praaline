#include "ExportAnnotationsWizard.h"
#include "ui_ExportAnnotationsWizard.h"
#include "ExportAnnotationsWizardPraatPage.h"

struct ExportAnnotationsWizardData {
    ExportAnnotationsWizardData()
    {}

    // Pages
    ExportAnnotationsWizardPraatPage *pagePraat;
    // State
};


ExportAnnotationsWizard::ExportAnnotationsWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::ExportAnnotationsWizard), d(new ExportAnnotationsWizardData)
{
    ui->setupUi(this);

    d->pagePraat = new ExportAnnotationsWizardPraatPage(this);

    addPage(d->pagePraat);
}

ExportAnnotationsWizard::~ExportAnnotationsWizard()
{
    delete ui;
    delete d;
}
