#ifndef IMPORTMETADATAWIZARDCOLUMNSPAGE_H
#define IMPORTMETADATAWIZARDCOLUMNSPAGE_H

#include <QWizardPage>

namespace Ui {
class ImportMetadataWizardColumnsPage;
}

class ImportMetadataWizardColumnsPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportMetadataWizardColumnsPage(QWidget *parent = nullptr);
    ~ImportMetadataWizardColumnsPage();

private:
    Ui::ImportMetadataWizardColumnsPage *ui;
};

#endif // IMPORTMETADATAWIZARDCOLUMNSPAGE_H
