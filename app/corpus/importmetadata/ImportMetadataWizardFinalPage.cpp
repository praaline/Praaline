#include "ImportMetadataWizardFinalPage.h"
#include "ui_ImportMetadataWizardFinalPage.h"

ImportMetadataWizardFinalPage::ImportMetadataWizardFinalPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ImportMetadataWizardFinalPage)
{
    ui->setupUi(this);
}

ImportMetadataWizardFinalPage::~ImportMetadataWizardFinalPage()
{
    delete ui;
}
