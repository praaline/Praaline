#ifndef ADDCALCULATEDANNOTATIONDIALOG_H
#define ADDCALCULATEDANNOTATIONDIALOG_H

#include <QDialog>

namespace Ui {
class AddCalculatedAnnotationDialog;
}

class AddCalculatedAnnotationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCalculatedAnnotationDialog(QWidget *parent = 0);
    ~AddCalculatedAnnotationDialog();

private:
    Ui::AddCalculatedAnnotationDialog *ui;
};

#endif // ADDCALCULATEDANNOTATIONDIALOG_H
