#ifndef NEWCORPUSWIZARD_H
#define NEWCORPUSWIZARD_H

#include <QWizard>
#include "pncore/corpus/corpus.h"

namespace Ui {
class NewCorpusWizard;
}

struct NewCorpusWizardData;

class NewCorpusWizard : public QWizard
{
    Q_OBJECT

public:
    explicit NewCorpusWizard(QWidget *parent = 0);
    ~NewCorpusWizard();

    bool validateCurrentPage();

    CorpusDefinition newDefinition();
    Corpus *newCorpus();

private slots:
    void localDbSelectFolder();
    void localDbDatabaseNameChanged();

private:
    Ui::NewCorpusWizard *ui;
    NewCorpusWizardData *d;

    void populateTemplates();
    void createLocalDbCorpus();
    void createRemoteDbCorpus();
    void createFilesCorpus();
    void applyTemplates();
};

#endif // NEWCORPUSWIZARD_H
