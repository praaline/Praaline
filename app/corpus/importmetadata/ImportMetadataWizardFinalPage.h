#ifndef IMPORTMETADATAWIZARDFINALPAGE_H
#define IMPORTMETADATAWIZARDFINALPAGE_H

#include <QWizardPage>

namespace Ui {
class ImportMetadataWizardFinalPage;
}

class ImportMetadataWizardFinalPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportMetadataWizardFinalPage(QWidget *parent = nullptr);
    ~ImportMetadataWizardFinalPage();

private:
    Ui::ImportMetadataWizardFinalPage *ui;
};

#endif // IMPORTMETADATAWIZARDFINALPAGE_H
