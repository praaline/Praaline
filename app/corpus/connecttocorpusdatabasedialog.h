#ifndef CONNECTTOCORPUSDATABASEDIALOG_H
#define CONNECTTOCORPUSDATABASEDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectToCorpusDatabaseDialog;
}

class ConnectToCorpusDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectToCorpusDatabaseDialog(QWidget *parent = 0);
    ~ConnectToCorpusDatabaseDialog();

private:
    Ui::ConnectToCorpusDatabaseDialog *ui;
};

#endif // CONNECTTOCORPUSDATABASEDIALOG_H
