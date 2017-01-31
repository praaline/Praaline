#include "ImportAnnotationsWizard.h"
#include "ui_ImportAnnotationsWizard.h"

ImportAnnotationsWizard::ImportAnnotationsWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::ImportAnnotationsWizard)
{
    ui->setupUi(this);
}

ImportAnnotationsWizard::~ImportAnnotationsWizard()
{
    delete ui;
}
