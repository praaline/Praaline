#ifndef CREATESEQUENCEANNOTATIONDIALOG_H
#define CREATESEQUENCEANNOTATIONDIALOG_H

#include <QDialog>

namespace Ui {
class CreateSequenceAnnotationDialog;
}

struct CreateSequenceAnnotationDialogData;

class CreateSequenceAnnotationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateSequenceAnnotationDialog(QWidget *parent = 0);
    ~CreateSequenceAnnotationDialog();

private:
    Ui::CreateSequenceAnnotationDialog *ui;
    CreateSequenceAnnotationDialogData *d;
};

#endif // CREATESEQUENCEANNOTATIONDIALOG_H
