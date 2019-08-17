#ifndef DECODEFILENAMETOMETADATADIALOG_H
#define DECODEFILENAMETOMETADATADIALOG_H

#include <QDialog>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
}
}

namespace Ui {
class DecodeFilenameToMetadataDialog;
}

struct DecodeFilenameToMetadataDialogData;

class DecodeFilenameToMetadataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DecodeFilenameToMetadataDialog(Praaline::Core::Corpus *corpus, QWidget *parent = nullptr);
    ~DecodeFilenameToMetadataDialog();

private slots:
    void operationChanged(int);
    void addCorrespondance();
    void removeCorrespondance();
    void updateMetadata();

private:
    Ui::DecodeFilenameToMetadataDialog *ui;
    DecodeFilenameToMetadataDialogData *d;
};

#endif // DECODEFILENAMETOMETADATADIALOG_H
