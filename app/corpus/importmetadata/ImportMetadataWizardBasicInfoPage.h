#ifndef IMPORTMETADATAWIZARDBASICINFOPAGE_H
#define IMPORTMETADATAWIZARDBASICINFOPAGE_H

#include <QWizardPage>

namespace Ui {
class ImportMetadataWizardBasicInfoPage;
}

class ImportMetadataWizardBasicInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportMetadataWizardBasicInfoPage(QWidget *parent = nullptr);
    ~ImportMetadataWizardBasicInfoPage();

private:
    Ui::ImportMetadataWizardBasicInfoPage *ui;
};

#endif // IMPORTMETADATAWIZARDBASICINFOPAGE_H
