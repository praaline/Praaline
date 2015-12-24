#ifndef CORPUSDATABASECONNECTIONDIALOG_H
#define CORPUSDATABASECONNECTIONDIALOG_H

#include <QDialog>
#include "pncore/serialisers/corpusdefinition.h"

namespace Ui {
class CorpusDatabaseConnectionDialog;
}

class CorpusDatabaseConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CorpusDatabaseConnectionDialog(QWidget *parent = 0);
    ~CorpusDatabaseConnectionDialog();

    CorpusDefinition corpusDefinition();

private slots:
    void selectBaseFolder();
    void datastoreInfoChanged();
    void useSameDatabaseChanged();

private:
    Ui::CorpusDatabaseConnectionDialog *ui;
};

#endif // CORPUSDATABASECONNECTIONDIALOG_H
