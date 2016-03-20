#ifndef CHECKMEDIAFILESDIALOG_H
#define CHECKMEDIAFILESDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "pncore/corpus/corpus.h"

namespace Ui {
class CheckMediaFilesDialog;
}

class CheckMediaFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckMediaFilesDialog(Corpus *corpus, QWidget *parent = 0);
    ~CheckMediaFilesDialog();

private:
    Ui::CheckMediaFilesDialog *ui;
    Corpus *m_corpus;
    QStandardItemModel *m_model;
    bool m_stop;

private slots:
    void process();
    void stop();
};

#endif // CHECKMEDIAFILESDIALOG_H
