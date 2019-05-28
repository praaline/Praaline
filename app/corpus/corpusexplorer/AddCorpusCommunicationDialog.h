#ifndef ADDCORPUSCOMMUNICATIONDIALOG_H
#define ADDCORPUSCOMMUNICATIONDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class AddCorpusCommunicationDialog;
}

struct AddCorpusCommunicationDialogData;

class AddCorpusCommunicationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCorpusCommunicationDialog(QWidget *parent = nullptr);
    ~AddCorpusCommunicationDialog();

    QString corpusID() const;
    void setCorpusID(const QString &corpusID);

    QString communicationID() const;
    void setCommunicationID(const QString &communicationID);

    QStringList mediaFilesList() const;
    QStringList annotationFilesList() const;


private:
    Ui::AddCorpusCommunicationDialog *ui;
    AddCorpusCommunicationDialogData *d;
};

#endif // ADDCORPUSCOMMUNICATIONDIALOG_H
