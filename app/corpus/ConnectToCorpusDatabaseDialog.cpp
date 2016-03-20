#include "ConnectToCorpusDatabaseDialog.h"
#include "ui_connecttocorpusdatabasedialog.h"

ConnectToCorpusDatabaseDialog::ConnectToCorpusDatabaseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectToCorpusDatabaseDialog)
{
    ui->setupUi(this);
}

ConnectToCorpusDatabaseDialog::~ConnectToCorpusDatabaseDialog()
{
    delete ui;
}
