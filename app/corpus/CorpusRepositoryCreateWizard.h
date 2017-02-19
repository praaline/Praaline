#ifndef NEWCORPUSREPOSITORYWIZARD_H
#define NEWCORPUSREPOSITORYWIZARD_H

#include <QWizard>
#include "pncore/datastore/CorpusRepositoryDefinition.h"
#include "pncore/datastore/CorpusRepository.h"

namespace Ui {
class CorpusRepositoryCreateWizard;
}

struct CorpusRepositoryCreateWizardData;

class CorpusRepositoryCreateWizard : public QWizard
{
    Q_OBJECT

public:
    explicit CorpusRepositoryCreateWizard(QWidget *parent = 0);
    ~CorpusRepositoryCreateWizard();

    bool validateCurrentPage();

    Praaline::Core::CorpusRepositoryDefinition newDefinition();
    Praaline::Core::CorpusRepository *newCorpusRepository();

private slots:
    void selectMediaBaseFolder();
    void databaseNameChanged();

private:
    Ui::CorpusRepositoryCreateWizard *ui;
    CorpusRepositoryCreateWizardData *d;

    void populateTemplates();
    void createLocalSQLRepository();
    void createRemoteSQLRepository();
    void createLocalXMLRepository();
    void applyTemplates();
};

#endif // NEWCORPUSREPOSITORYWIZARD_H
