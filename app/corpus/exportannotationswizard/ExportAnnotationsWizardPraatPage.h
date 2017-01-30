#ifndef EXPORTANNOTATIONSWIZARDPRAATPAGE_H
#define EXPORTANNOTATIONSWIZARDPRAATPAGE_H

#include <QPointer>
#include <QWizardPage>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ExportAnnotationsWizardPraatPage;
}

struct ExportAnnotationsWizardPraatPageData;

class ExportAnnotationsWizardPraatPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ExportAnnotationsWizardPraatPage(QWidget *parent = 0);
    ~ExportAnnotationsWizardPraatPage();

private:
    Ui::ExportAnnotationsWizardPraatPage *ui;
    ExportAnnotationsWizardPraatPageData *d;
};

#endif // EXPORTANNOTATIONSWIZARDPRAATPAGE_H
