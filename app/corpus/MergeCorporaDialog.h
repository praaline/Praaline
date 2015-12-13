#ifndef MERGECORPORADIALOG_H
#define MERGECORPORADIALOG_H

#include <QString>
#include <QDialog>

class Corpus;

namespace Ui {
class MergeCorporaDialog;
}

struct MergeCorporaDialogData;

class MergeCorporaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MergeCorporaDialog(QWidget *parent = 0);
    ~MergeCorporaDialog();

private slots:
    void browseForCorpusA();
    void browseForCorpusB();
    void browseForDestinationFolder();
    void mergeCorpora();

private:
    Corpus *openCorpusFile(const QString &filename);

    Ui::MergeCorporaDialog *ui;
    MergeCorporaDialogData *d;
};

#endif // MERGECORPORADIALOG_H
