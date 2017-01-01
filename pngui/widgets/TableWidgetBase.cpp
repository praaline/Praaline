#include "TableWidgetBase.h"
#include "ui_TableWidgetBase.h"

TableWidgetBase::TableWidgetBase(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TableWidgetBase)
{
    ui->setupUi(this);
}

TableWidgetBase::~TableWidgetBase()
{
    delete ui;
}
