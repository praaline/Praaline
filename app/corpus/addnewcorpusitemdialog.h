#ifndef ADDNEWCORPUSITEMDIALOG_H
#define ADDNEWCORPUSITEMDIALOG_H

#include <QDialog>

namespace Ui {
class AddNewCorpusItemDialog;
}

class AddNewCorpusItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddNewCorpusItemDialog(QWidget *parent = 0);
    ~AddNewCorpusItemDialog();

private:
    Ui::AddNewCorpusItemDialog *ui;
};

#endif // ADDNEWCORPUSITEMDIALOG_H
