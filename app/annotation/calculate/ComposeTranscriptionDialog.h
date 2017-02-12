#ifndef COMPOSETRANSCRIPTIONDIALOG_H
#define COMPOSETRANSCRIPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ComposeTranscriptionDialog;
}

class ComposeTranscriptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ComposeTranscriptionDialog(QWidget *parent = 0);
    ~ComposeTranscriptionDialog();

private:
    Ui::ComposeTranscriptionDialog *ui;
};

#endif // COMPOSETRANSCRIPTIONDIALOG_H
