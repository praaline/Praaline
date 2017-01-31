#ifndef IMPORTCORPUSITEMSWIZARD_H
#define IMPORTCORPUSITEMSWIZARD_H

#include <QWizard>
#include <QPointer>
#include "pncore/datastore/CorpusRepository.h"
using namespace Praaline::Core;

namespace Ui {
class ImportCorpusItemsWizard;
}

struct ImportCorpusItemsWizardData;

class ImportCorpusItemsWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizard(QPointer<CorpusRepository> repository, QWidget *parent = 0);
    ~ImportCorpusItemsWizard();

private:
    Ui::ImportCorpusItemsWizard *ui;
    ImportCorpusItemsWizardData *d;
};

#endif // IMPORTCORPUSITEMSWIZARD_H
