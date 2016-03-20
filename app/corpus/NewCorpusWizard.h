#ifndef NEWCORPUSWIZARD_H
#define NEWCORPUSWIZARD_H

#include <QWizard>
#include "pncore/corpus/corpus.h"

namespace Ui {
class NewCorpusWizard;
}

class NewCorpusWizard : public QWizard
{
    Q_OBJECT

public:
    explicit NewCorpusWizard(QWidget *parent = 0);
    ~NewCorpusWizard();

    bool validateCurrentPage();

    CorpusDefinition newDefinition() { return m_newDefinition; }
    Corpus *newCorpus() { return m_newCorpus; }

private slots:
    void localDbSelectFolder();
    void localDbDatabaseNameChanged();

private:
    Ui::NewCorpusWizard *ui;

    void createLocalDbCorpus();
    void createRemoteDbCorpus();
    void createFilesCorpus();

    CorpusDefinition m_newDefinition;
    Corpus *m_newCorpus;
};

#endif // NEWCORPUSWIZARD_H
