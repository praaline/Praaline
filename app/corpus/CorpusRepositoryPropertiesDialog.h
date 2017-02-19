#ifndef CORPUSREPOSITORYPROPERTIESDIALOG_H
#define CORPUSREPOSITORYPROPERTIESDIALOG_H

#include <QDialog>
#include "pncore/datastore/CorpusRepository.h"

namespace Ui {
class CorpusRepositoryPropertiesDialog;
}

class CorpusRepositoryPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CorpusRepositoryPropertiesDialog(Praaline::Core::CorpusRepository *repository, QWidget *parent = 0);
    ~CorpusRepositoryPropertiesDialog();

private:
    Ui::CorpusRepositoryPropertiesDialog *ui;
};

#endif // CORPUSREPOSITORYPROPERTIESDIALOG_H
