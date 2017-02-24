#ifndef ADDCALCULATEDANNOTATIONDIALOG_H
#define ADDCALCULATEDANNOTATIONDIALOG_H

#include <QDialog>

namespace Ui {
class AddCalculatedAnnotationDialog;
}

struct AddCalculatedAnnotationDialogData;

class AddCalculatedAnnotationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCalculatedAnnotationDialog(QWidget *parent = 0);
    ~AddCalculatedAnnotationDialog();

private slots:
    void corpusRepositoryChanged(const QString &repositoryID);
    void annotationLevelBaseChanged(int index);
    void annotationLevelDerivedChanged(int index);
    void applyMeasure();

private:
    Ui::AddCalculatedAnnotationDialog *ui;
    AddCalculatedAnnotationDialogData *d;
};

#endif // ADDCALCULATEDANNOTATIONDIALOG_H
