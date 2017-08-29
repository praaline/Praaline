#include "GridViewFindDialog.h"
#include "ui_GridViewFindDialog.h"

GridViewFindDialog::GridViewFindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GridViewFindDialog)
{
    ui->setupUi(this);

    ui->comboBoxSearchScope->addItems(QStringList() << "Rows" << "Columns" << "Entire table");
    setDefaultSearchScope(GridViewFindDialog::EntireTable);

    connect(ui->commandFindNext, SIGNAL(clicked(bool)), this, SLOT(findNextClicked()));
    connect(ui->commandClose, SIGNAL(clicked(bool)), this, SLOT(close()));
}

GridViewFindDialog::~GridViewFindDialog()
{
    delete ui;
}

void GridViewFindDialog::findNextClicked()
{
    emit findNext();
}

QString GridViewFindDialog::findWhat() const
{
    return ui->editFindWhat->text();
}

GridViewFindDialog::SearchScope GridViewFindDialog::searchScope() const
{
    if (ui->comboBoxSearchScope->currentText() == "Rows")
        return GridViewFindDialog::Rows;
    else if (ui->comboBoxSearchScope->currentText() == "Columns")
        return GridViewFindDialog::Columns;
    else
        return GridViewFindDialog::EntireTable;
}

void GridViewFindDialog::setDefaultSearchScope(GridViewFindDialog::SearchScope scope)
{
    if (scope == GridViewFindDialog::Rows)
        ui->comboBoxSearchScope->setCurrentText("Rows");
    else if (scope == GridViewFindDialog::Columns)
        ui->comboBoxSearchScope->setCurrentText("Columns");
    else
        ui->comboBoxSearchScope->setCurrentText("Entire table");
}

bool GridViewFindDialog::matchWholeWordsOnly() const
{
    return ui->optionMatchWholeWords->isChecked();
}

bool GridViewFindDialog::matchCase() const
{
    return ui->optionMatchCase->isChecked();
}

bool GridViewFindDialog::useRegularExpressions() const
{
    return ui->optionRegEx->isChecked();
}

bool GridViewFindDialog::directionIsDown() const
{
    return ui->optionSearchDown->isChecked();
}

bool GridViewFindDialog::wrapAround() const
{
    return ui->optionWrapAround->isChecked();
}
