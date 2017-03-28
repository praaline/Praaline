#include "TranscriptionAnnotationWidget.h"
#include "ui_TranscriptionAnnotationWidget.h"

TranscriptionAnnotationWidget::TranscriptionAnnotationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TranscriptionAnnotationWidget)
{
    ui->setupUi(this);
}

TranscriptionAnnotationWidget::~TranscriptionAnnotationWidget()
{
    delete ui;
}
