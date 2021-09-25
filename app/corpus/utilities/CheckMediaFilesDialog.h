#ifndef CHECKMEDIAFILESDIALOG_H
#define CHECKMEDIAFILESDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include "PraalineCore/Corpus/Corpus.h"
using namespace Praaline::Core;

namespace Ui {
class CheckMediaFilesDialog;
}

struct CheckMediaFilesDialogData;

class CheckMediaFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckMediaFilesDialog(Corpus *corpus, QWidget *parent = nullptr);
    ~CheckMediaFilesDialog();

private:
    Ui::CheckMediaFilesDialog *ui;
    CheckMediaFilesDialogData *d;

private slots:
    void process();
    void stop();
};

#endif // CHECKMEDIAFILESDIALOG_H
