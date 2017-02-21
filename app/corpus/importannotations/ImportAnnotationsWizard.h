#ifndef IMPORTANNOTATIONSWIZARD_H
#define IMPORTANNOTATIONSWIZARD_H

#include <QWizard>

namespace Ui {
class ImportAnnotationsWizard;
}

struct ImportAnnotationsWizardData;

class ImportAnnotationsWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ImportAnnotationsWizard(QWidget *parent = 0);
    ~ImportAnnotationsWizard();

private:
    Ui::ImportAnnotationsWizard *ui;
    ImportAnnotationsWizardData *d;
};

#endif // IMPORTANNOTATIONSWIZARD_H
