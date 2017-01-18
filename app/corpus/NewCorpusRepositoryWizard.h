#ifndef NEWCORPUSREPOSITORYWIZARD_H
#define NEWCORPUSREPOSITORYWIZARD_H

#include <QWizard>
#include "pncore/datastore/CorpusRepositoryDefinition.h"
#include "pncore/datastore/CorpusRepository.h"

namespace Ui {
class NewCorpusRepositoryWizard;
}

struct NewCorpusRepositoryWizardData;

class NewCorpusRepositoryWizard : public QWizard
{
    Q_OBJECT

public:
    explicit NewCorpusRepositoryWizard(QWidget *parent = 0);
    ~NewCorpusRepositoryWizard();

    bool validateCurrentPage();

    Praaline::Core::CorpusRepositoryDefinition newDefinition();
    Praaline::Core::CorpusRepository *newCorpusRepository();

private slots:
    void localDbSelectFolder();
    void localDbDatabaseNameChanged();

private:
    Ui::NewCorpusRepositoryWizard *ui;
    NewCorpusRepositoryWizardData *d;

    void populateTemplates();
    void createLocalSQLRepository();
    void createRemoteSQLRepository();
    void createLocalXMLRepository();
    void applyTemplates();
};

#endif // NEWCORPUSREPOSITORYWIZARD_H
