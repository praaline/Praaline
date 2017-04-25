#include "LanguageModelBuilderWidget.h"
#include "ui_LanguageModelBuilderWidget.h"

LanguageModelBuilderWidget::LanguageModelBuilderWidget(QWidget *parent) :
    ASRModuleWidgetBase(parent), ui(new Ui::LanguageModelBuilderWidget)
{
    ui->setupUi(this);
}

LanguageModelBuilderWidget::~LanguageModelBuilderWidget()
{
    delete ui;
}
