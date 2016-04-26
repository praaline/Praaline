#include "AddNewCorpusItemDialog.h"
#include "ui_AddNewCorpusItemDialog.h"

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
