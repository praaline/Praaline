#include "ImportMetadataWizardColumnsPage.h"
#include "ui_ImportMetadataWizardColumnsPage.h"

ImportMetadataWizardColumnsPage::ImportMetadataWizardColumnsPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ImportMetadataWizardColumnsPage)
{
    ui->setupUi(this);
}

ImportMetadataWizardColumnsPage::~ImportMetadataWizardColumnsPage()
{
    delete ui;
}
