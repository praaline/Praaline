#ifndef MERGECOMMUNICATIONSDIALOG_H
#define MERGECOMMUNICATIONSDIALOG_H

#include <QDialog>

namespace Praaline {
namespace Core {
class Corpus;
}
}

namespace Ui {
class MergeCommunicationsDialog;
}

class MergeCommunicationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MergeCommunicationsDialog(Praaline::Core::Corpus *corpus, QWidget *parent = nullptr);
    ~MergeCommunicationsDialog();

private:
    Ui::MergeCommunicationsDialog *ui;
};

#endif // MERGECOMMUNICATIONSDIALOG_H
