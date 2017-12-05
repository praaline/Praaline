#ifndef DECODEFILENAMETOMETADATADIALOG_H
#define DECODEFILENAMETOMETADATADIALOG_H

#include <QDialog>

namespace Ui {
class DecodeFilenameToMetadataDialog;
}

struct DecodeFilenameToMetadataDialogData;

class DecodeFilenameToMetadataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DecodeFilenameToMetadataDialog(QWidget *parent = 0);
    ~DecodeFilenameToMetadataDialog();

private slots:
    void addCorrespondance();
    void removeCorrespondance();
    void updateMetadata();

private:
    Ui::DecodeFilenameToMetadataDialog *ui;
    DecodeFilenameToMetadataDialogData *d;
};

#endif // DECODEFILENAMETOMETADATADIALOG_H
