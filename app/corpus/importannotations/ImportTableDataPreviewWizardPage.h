#ifndef IMPORTTABLEDATAPREVIEWWIZARDPAGE_H
#define IMPORTTABLEDATAPREVIEWWIZARDPAGE_H

#include <QWizardPage>

namespace Ui {
class ImportTableDataPreviewWizardPage;
}

struct ImportTableDataPreviewWizardPageData;

class ImportTableDataPreviewWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ImportTableDataPreviewWizardPage(QWidget *parent = nullptr);
    ~ImportTableDataPreviewWizardPage();

private:
    Ui::ImportTableDataPreviewWizardPage *ui;
    ImportTableDataPreviewWizardPageData *d;
};

#endif // IMPORTTABLEDATAPREVIEWWIZARDPAGE_H
