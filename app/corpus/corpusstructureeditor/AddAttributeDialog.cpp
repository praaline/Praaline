#include "AddAttributeDialog.h"
#include "ui_addattributedialog.h"

AddAttributeDialog::AddAttributeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAttributeDialog)
{
    ui->setupUi(this);
    QStringList datatypes;
    datatypes << "Text" << "Number (real)" << "Number (integer)" << "Yes/No (boolean)" << "Date/Time";
    ui->comboDatatype->addItems(datatypes);
    ui->spinDatalength->setValue(256);

    connect(ui->comboDatatype, SIGNAL(currentTextChanged(QString)), this, SLOT(datatypeChanged(QString)));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AddAttributeDialog::~AddAttributeDialog()
{
    delete ui;
}

QString AddAttributeDialog::attributeID() const
{
    return ui->editAttributeID->text();
}

QString AddAttributeDialog::datatype() const
{
    if (ui->comboDatatype->currentText() == "Text")                     return "varchar";
    else if (ui->comboDatatype->currentText() == "Number (real)")       return "double";
    else if (ui->comboDatatype->currentText() == "Number (integer)")    return "int";
    else if (ui->comboDatatype->currentText() == "Yes/No (boolean)")    return "bool";
    else if (ui->comboDatatype->currentText() == "Date/Time")           return "datetime";
    return "varchar";
}

int AddAttributeDialog::datalength() const
{
    if (datatype() == "varchar")
        return ui->spinDatalength->value();
    return 0;
}

void AddAttributeDialog::datatypeChanged(QString text)
{
    Q_UNUSED(text)
    if (ui->comboDatatype->currentText() != "Text")
        ui->spinDatalength->setEnabled(false);
    else
        ui->spinDatalength->setEnabled(true);
}
