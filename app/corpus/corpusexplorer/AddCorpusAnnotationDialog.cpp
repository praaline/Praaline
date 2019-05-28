#include "AddCorpusAnnotationDialog.h"
#include "ui_AddCorpusAnnotationDialog.h"

AddCorpusAnnotationDialog::AddCorpusAnnotationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCorpusAnnotationDialog)
{
    ui->setupUi(this);
}

AddCorpusAnnotationDialog::~AddCorpusAnnotationDialog()
{
    delete ui;
}
