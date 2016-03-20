#ifndef SPLITCOMMUNICATIONSDIALOG_H
#define SPLITCOMMUNICATIONSDIALOG_H

#include <QDialog>
#include <QPointer>
#include "pncore/corpus/corpus.h"

namespace Ui {
class SplitCommunicationsDialog;
}

struct SplitCommunicationsDialogData;

class SplitCommunicationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SplitCommunicationsDialog(QPointer<Corpus> corpus, QWidget *parent = 0);
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
