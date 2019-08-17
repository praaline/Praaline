#ifndef MERGECORPORADIALOG_H
#define MERGECORPORADIALOG_H

#include <QString>
#include <QDialog>

namespace Praaline {
namespace Core {
class Corpus;
}
}

namespace Ui {
class MergeCorporaDialog;
}

struct MergeCorporaDialogData;

class MergeCorporaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MergeCorporaDialog(QWidget *parent = nullptr);
    ~MergeCorporaDialog();

private slots:
    void browseForCorpusA();
    void browseForCorpusB();
    void browseForDestinationFolder();
    void mergeCorpora();

private:
    Praaline::Core::Corpus *openCorpusFile(const QString &filename);

    Ui::MergeCorporaDialog *ui;
    MergeCorporaDialogData *d;
};

#endif // MERGECORPORADIALOG_H
