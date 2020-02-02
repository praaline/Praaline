#ifndef CORPUSREPOSITORYPROPERTIESDIALOG_H
#define CORPUSREPOSITORYPROPERTIESDIALOG_H

#include <QDialog>
#include "PraalineCore/Datastore/CorpusRepositoryDefinition.h"

namespace Ui {
class CorpusRepositoryPropertiesDialog;
}

struct CorpusRepositoryPropertiesDialogData;

class CorpusRepositoryPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CorpusRepositoryPropertiesDialog(const Praaline::Core::CorpusRepositoryDefinition &repositoryDef,
                                              QWidget *parent = nullptr);
    ~CorpusRepositoryPropertiesDialog();

    Praaline::Core::CorpusRepositoryDefinition &repositoryDef() const;

private slots:
    void accept();
    void selectBasePathMedia();

private:
    Ui::CorpusRepositoryPropertiesDialog *ui;
    CorpusRepositoryPropertiesDialogData *d;

    void updateFields();
};

#endif // CORPUSREPOSITORYPROPERTIESDIALOG_H
