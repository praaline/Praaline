#include "AddCalculatedAnnotationDialog.h"
#include "ui_AddCalculatedAnnotationDialog.h"

AddCalculatedAnnotationDialog::AddCalculatedAnnotationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCalculatedAnnotationDialog)
{
    ui->setupUi(this);
}

AddCalculatedAnnotationDialog::~AddCalculatedAnnotationDialog()
{
    delete ui;
}
