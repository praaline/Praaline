#ifndef EXPORTANNOTATIONSWIZARDPRAATPAGE_H
#define EXPORTANNOTATIONSWIZARDPRAATPAGE_H

#include <QPointer>
#include <QWizardPage>
#include "pncore/corpus/corpus.h"

namespace Ui {
class ExportAnnotationsWizardPraatPage;
}

struct ExportAnnotationsWizardPraatPageData;

class ExportAnnotationsWizardPraatPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ExportAnnotationsWizardPraatPage(QPointer<Corpus> corpus, QWidget *parent = 0);
    ~ExportAnnotationsWizardPraatPage();

private:
    Ui::ExportAnnotationsWizardPraatPage *ui;
    ExportAnnotationsWizardPraatPageData *d;
};

#endif // EXPORTANNOTATIONSWIZARDPRAATPAGE_H
