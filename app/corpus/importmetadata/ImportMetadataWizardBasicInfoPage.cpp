#include "ImportMetadataWizardBasicInfoPage.h"
#include "ui_ImportMetadataWizardBasicInfoPage.h"

ImportMetadataWizardBasicInfoPage::ImportMetadataWizardBasicInfoPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::ImportMetadataWizardBasicInfoPage)
{
    ui->setupUi(this);
}

ImportMetadataWizardBasicInfoPage::~ImportMetadataWizardBasicInfoPage()
{
    delete ui;
}
