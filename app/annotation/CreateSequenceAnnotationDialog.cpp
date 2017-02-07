#include "CreateSequenceAnnotationDialog.h"
#include "ui_CreateSequenceAnnotationDialog.h"

CreateSequenceAnnotationDialog::CreateSequenceAnnotationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateSequenceAnnotationDialog)
{
    ui->setupUi(this);
}

CreateSequenceAnnotationDialog::~CreateSequenceAnnotationDialog()
{
    delete ui;
}
