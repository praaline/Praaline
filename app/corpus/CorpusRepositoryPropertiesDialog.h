#ifndef CORPUSREPOSITORYPROPERTIESDIALOG_H
#define CORPUSREPOSITORYPROPERTIESDIALOG_H

#include <QDialog>
#include "pncore/datastore/CorpusRepositoryDefinition.h"

namespace Ui {
class CorpusRepositoryPropertiesDialog;
}

struct CorpusRepositoryPropertiesDialogData;

class CorpusRepositoryPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CorpusRepositoryPropertiesDialog(const Praaline::Core::CorpusRepositoryDefinition &repositoryDef,
                                              QWidget *parent = 0);
    ~CorpusRepositoryPropertiesDialog();

    Praaline::Core::CorpusRepositoryDefinition &repositoryDef() const;

private slots:
    void accept();

private:
    Ui::CorpusRepositoryPropertiesDialog *ui;
    CorpusRepositoryPropertiesDialogData *d;

    void updateFields();
};

#endif // CORPUSREPOSITORYPROPERTIESDIALOG_H
