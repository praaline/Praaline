#ifndef UPDATEPRAALINEDIALOG_H
#define UPDATEPRAALINEDIALOG_H

#include <QDialog>

namespace Ui {
class UpdatePraalineDialog;
}

struct UpdatePraalineDialogData;

class UpdatePraalineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdatePraalineDialog(QWidget *parent = 0);
    ~UpdatePraalineDialog();


private slots:
    void checkForUpdates();
    void updateChangelog(const QString &url);

private:
    Ui::UpdatePraalineDialog *ui;
    UpdatePraalineDialogData *d;
};

#endif // UPDATEPRAALINEDIALOG_H
