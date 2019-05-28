#ifndef ADDCORPUSANNOTATIONDIALOG_H
#define ADDCORPUSANNOTATIONDIALOG_H

#include <QDialog>

namespace Ui {
class AddCorpusAnnotationDialog;
}

class AddCorpusAnnotationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCorpusAnnotationDialog(QWidget *parent = nullptr);
    ~AddCorpusAnnotationDialog();

private:
    Ui::AddCorpusAnnotationDialog *ui;
};

#endif // ADDCORPUSANNOTATIONDIALOG_H
