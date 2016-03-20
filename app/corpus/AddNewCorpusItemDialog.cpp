#include "AddNewCorpusItemDialog.h"
#include "ui_addnewcorpusitemdialog.h"

AddNewCorpusItemDialog::AddNewCorpusItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddNewCorpusItemDialog)
{
    ui->setupUi(this);
}

AddNewCorpusItemDialog::~AddNewCorpusItemDialog()
{
    delete ui;
}
