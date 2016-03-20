#include "ImportMetadataWizardBasicInfoPage.h"
#include "ui_importmetadatawizardbasicinfopage.h"

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
