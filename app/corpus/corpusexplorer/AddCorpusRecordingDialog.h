#ifndef ADDCORPUSRECORDINGDIALOG_H
#define ADDCORPUSRECORDINGDIALOG_H

#include <QDialog>

namespace Ui {
class AddCorpusRecordingDialog;
}

class AddCorpusRecordingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCorpusRecordingDialog(QWidget *parent = nullptr);
    ~AddCorpusRecordingDialog();

private:
    Ui::AddCorpusRecordingDialog *ui;
};

#endif // ADDCORPUSRECORDINGDIALOG_H
