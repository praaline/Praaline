#ifndef TIDYUPANNOTATIONSDIALOG_H
#define TIDYUPANNOTATIONSDIALOG_H

#include <QDialog>

namespace Ui {
class TidyUpAnnotationsDialog;
}

struct TidyUpAnnotationsDialogData;

class TidyUpAnnotationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TidyUpAnnotationsDialog(QWidget *parent = 0);
    ~TidyUpAnnotationsDialog();

private slots:
    void corpusRepositoryChanged(const QString &repositoryID);
    void annotationLevelChanged(int index);
    void applyChanges();

private:
    Ui::TidyUpAnnotationsDialog *ui;
    TidyUpAnnotationsDialogData *d;
};

#endif // TIDYUPANNOTATIONSDIALOG_H
