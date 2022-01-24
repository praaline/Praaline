#ifndef CHECKMEDIAFILESDIALOG_H
#define CHECKMEDIAFILESDIALOG_H

#include <QDialog>
#include <QStandardItemModel>

namespace Praaline {
namespace Core {
class CorpusRepository;
}
}

namespace Ui {
class CheckMediaFilesDialog;
}

struct CheckMediaFilesDialogData;

class CheckMediaFilesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckMediaFilesDialog(Praaline::Core::CorpusRepository *repository, QWidget *parent = nullptr);
    ~CheckMediaFilesDialog();

private:
    Ui::CheckMediaFilesDialog *ui;
    CheckMediaFilesDialogData *d;

private slots:
    void process();
    void stop();
};

#endif // CHECKMEDIAFILESDIALOG_H
