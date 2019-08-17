#ifndef SPLITCOMMUNICATIONSDIALOG_H
#define SPLITCOMMUNICATIONSDIALOG_H

#include <QDialog>

namespace Praaline {
namespace Core {
class Corpus;
}
}

namespace Ui {
class SplitCommunicationsDialog;
}

struct SplitCommunicationsDialogData;

class SplitCommunicationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SplitCommunicationsDialog(Praaline::Core::Corpus *corpus, QWidget *parent = nullptr);
    ~SplitCommunicationsDialog();

private:
    Ui::SplitCommunicationsDialog *ui;
    SplitCommunicationsDialogData *d;

private slots:
    void annotationLevelChanged();
    void annotationAttributeChanged();
    void doSplit();
};

#endif // SPLITCOMMUNICATIONSDIALOG_H
