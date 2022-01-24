#ifndef IMPORTCORPUSITEMSWIZARD_H
#define IMPORTCORPUSITEMSWIZARD_H

#include <QWizard>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusRepository;
}
}

namespace Ui {
class ImportCorpusItemsWizard;
}

struct ImportCorpusItemsWizardData;

class ImportCorpusItemsWizard : public QWizard
{
    Q_OBJECT

public:
    explicit ImportCorpusItemsWizard(Praaline::Core::CorpusRepository *repository, QWidget *parent = nullptr);
    ~ImportCorpusItemsWizard();

private:
    Ui::ImportCorpusItemsWizard *ui;
    ImportCorpusItemsWizardData *d;
};

#endif // IMPORTCORPUSITEMSWIZARD_H
