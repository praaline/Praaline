#ifndef CREATECORPUSCOMMUNICATIONDIALOG_H
#define CREATECORPUSCOMMUNICATIONDIALOG_H

#include <QDialog>

namespace Ui {
class CreateCorpusCommunicationDialog;
}

class CreateCorpusCommunicationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateCorpusCommunicationDialog(QWidget *parent = nullptr);
    ~CreateCorpusCommunicationDialog();

private:
    Ui::CreateCorpusCommunicationDialog *ui;
};

#endif // CREATECORPUSCOMMUNICATIONDIALOG_H
