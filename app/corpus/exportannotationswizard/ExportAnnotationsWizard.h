#ifndef EXPORTANNOTATIONSWIZARD_H
#define EXPORTANNOTATIONSWIZARD_H

#include <QPointer>
#include <QWizard>
#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class ExportAnnotationsWizard;
}

struct ExportAnnotationsWizardData;

class ExportAnnotationsWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ExportAnnotationsWizard(QPointer<Corpus> corpus, QWidget *parent = 0);
    ~ExportAnnotationsWizard();

private:
    Ui::ExportAnnotationsWizard *ui;
    ExportAnnotationsWizardData *d;
};

#endif // EXPORTANNOTATIONSWIZARD_H
