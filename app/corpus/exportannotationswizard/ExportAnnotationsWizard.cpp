#include "ExportAnnotationsWizard.h"
#include "ui_exportannotationswizard.h"
#include "ExportAnnotationsWizardPraatPage.h"

struct ExportAnnotationsWizardData {
    ExportAnnotationsWizardData() :
        corpus(0)
    {}

    // Pages
    ExportAnnotationsWizardPraatPage *pagePraat;
    // State
    QPointer<Corpus> corpus;
};


ExportAnnotationsWizard::ExportAnnotationsWizard(QPointer<Corpus> corpus, QWidget *parent) :
    QWizard(parent), ui(new Ui::ExportAnnotationsWizard), d(new ExportAnnotationsWizardData)
{
    ui->setupUi(this);

    d->corpus = corpus;
    d->pagePraat = new ExportAnnotationsWizardPraatPage(d->corpus, this);

    addPage(d->pagePraat);
}

ExportAnnotationsWizard::~ExportAnnotationsWizard()
{
    delete ui;
    delete d;
}
