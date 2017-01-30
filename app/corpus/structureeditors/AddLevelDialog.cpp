#include "AddLevelDialog.h"
#include "ui_AddLevelDialog.h"

AddLevelDialog::AddLevelDialog(AnnotationStructure *structure, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddLevelDialog),
    m_annotationStructure(structure)
{
    ui->setupUi(this);
    QStringList leveltypes, datatypes, levelIDs;

    leveltypes << "Independent Points" << "Independent Intervals" << "Grouping Intervals"
               << "Sequences" << "Tree" << "Relations";
    ui->comboLevelType->addItems(leveltypes);
    datatypes << "Text" << "Number (real)" << "Number (integer)" << "Yes/No (boolean)" << "Date/Time";
    ui->comboDatatype->addItems(datatypes);
    ui->comboParentLevelID->addItem("");
    if (structure) {
        foreach (AnnotationStructureLevel *level, structure->levels()) levelIDs << level->ID();
        ui->comboParentLevelID->addItems(levelIDs);
    }
    ui->spinDatalength->setValue(1024);

    connect(ui->comboLevelType, SIGNAL(currentTextChanged(QString)), this, SLOT(levelTypeChanged(QString)));
    connect(ui->comboDatatype, SIGNAL(currentTextChanged(QString)), this, SLOT(datatypeChanged(QString)));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

AddLevelDialog::~AddLevelDialog()
{
    delete ui;
}

QString AddLevelDialog::levelID() const
{
    return ui->editLevelID->text();
}

AnnotationStructureLevel::LevelType AddLevelDialog::levelType() const
{
    QString lt = ui->comboLevelType->currentText();
    if      (lt == "Independent Points")    return AnnotationStructureLevel::IndependentPointsLevel;
    else if (lt == "Independent Intervals") return AnnotationStructureLevel::IndependentIntervalsLevel;
    else if (lt == "Grouping Intervals")    return AnnotationStructureLevel::GroupingLevel;
    else if (lt == "Sequences")             return AnnotationStructureLevel::SequencesLevel;
    else if (lt == "Tree")                  return AnnotationStructureLevel::TreeLevel;
    else if (lt == "Relations")             return AnnotationStructureLevel::RelationsLevel;
    return AnnotationStructureLevel::IndependentIntervalsLevel;
}

QString AddLevelDialog::parentLevelID() const
{
    return ui->comboParentLevelID->currentText();
}

QString AddLevelDialog::datatype() const
{
    if (ui->comboDatatype->currentText() == "Text")                     return "varchar";
    else if (ui->comboDatatype->currentText() == "Number (real)")       return "double";
    else if (ui->comboDatatype->currentText() == "Number (integer)")    return "integer";
    else if (ui->comboDatatype->currentData() == "Yes/No (boolean)")    return "bool";
    else if (ui->comboDatatype->currentData() == "Date/Time")           return "datetime";
    return "varchar";
}

int AddLevelDialog::datalength() const
{
    if (datatype() == "varchar")
        return ui->spinDatalength->value();
    return 0;
}

void AddLevelDialog::levelTypeChanged(QString text)
{
    Q_UNUSED(text)
    if (ui->comboLevelType->currentText() == "Independent")
        ui->comboParentLevelID->setCurrentText("");
}

void AddLevelDialog::datatypeChanged(QString text)
{
    Q_UNUSED(text)
    if (ui->comboDatatype->currentText() != "Text")
        ui->spinDatalength->setEnabled(false);
    else
        ui->spinDatalength->setEnabled(true);
}

