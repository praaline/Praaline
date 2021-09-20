#include "pngui/widgets/ImportDataPreviewWidget.h"

#include "ImportTableDataPreviewWizardPage.h"
#include "ui_ImportTableDataPreviewWizardPage.h"

struct ImportTableDataPreviewWizardPageData {
    ImportDataPreviewWidget *previewWidget;
};

ImportTableDataPreviewWizardPage::ImportTableDataPreviewWizardPage(QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportTableDataPreviewWizardPage), d(new ImportTableDataPreviewWizardPageData)
{
    ui->setupUi(this);
    d->previewWidget = new ImportDataPreviewWidget(this);
    ui->gridLayout->addWidget(d->previewWidget, 0, 0);
}

ImportTableDataPreviewWizardPage::~ImportTableDataPreviewWizardPage()
{
    delete ui;
    delete d;
}
