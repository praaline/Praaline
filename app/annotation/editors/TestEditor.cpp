#include "TestEditor.h"
#include "ui_TestEditor.h"

TestEditor::TestEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestEditor)
{
    ui->setupUi(this);
}

TestEditor::~TestEditor()
{
    delete ui;
}
