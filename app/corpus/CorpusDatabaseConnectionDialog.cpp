#include <QObject>
#include <QString>
#include <QFileDialog>
#include "CorpusDatabaseConnectionDialog.h"
#include "ui_CorpusDatabaseConnectionDialog.h"
#include "pncore/serialisers/corpusdefinition.h"

CorpusDatabaseConnectionDialog::CorpusDatabaseConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CorpusDatabaseConnectionDialog)
{
    ui->setupUi(this);
    connect(ui->commandConnect, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(ui->commandCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    // Database types
    QStringList databaseTypeNames; databaseTypeNames << "MySQL database" << "PostgreSQL database";
    QStringList databaseTypeDrivers; databaseTypeDrivers << "QMYSQL" << "QPSQL";
    for (int i = 0; i < databaseTypeDrivers.count(); ++i) {
        ui->comboBoxDatabaseTypeMetadata->addItem(databaseTypeNames.at(i), databaseTypeDrivers.at(i));
        ui->comboBoxDatabaseTypeAnnotations->addItem(databaseTypeNames.at(i), databaseTypeDrivers.at(i));
    }
    ui->comboBoxDatabaseTypeMetadata->setCurrentIndex(0);
    ui->comboBoxDatabaseTypeAnnotations->setCurrentIndex(0);
    //
    connect(ui->commandSelectFolder, SIGNAL(clicked(bool)), this, SLOT(selectBaseFolder()));
    connect(ui->comboBoxDatabaseTypeMetadata, SIGNAL(currentIndexChanged(int)), this, SLOT(datastoreInfoChanged()));
    connect(ui->editHostnameMetadata, SIGNAL(textChanged(QString)), this, SLOT(datastoreInfoChanged()));
    connect(ui->editUsernameMetadata, SIGNAL(textChanged(QString)), this, SLOT(datastoreInfoChanged()));
    connect(ui->editPasswordMetadata, SIGNAL(textChanged(QString)), this, SLOT(datastoreInfoChanged()));
    connect(ui->editDatabaseNameMetadata, SIGNAL(textChanged(QString)), this, SLOT(datastoreInfoChanged()));
    connect(ui->optionUseSameDatabase, SIGNAL(stateChanged(int)), this, SLOT(useSameDatabaseChanged()));
}

CorpusDatabaseConnectionDialog::~CorpusDatabaseConnectionDialog()
{
    delete ui;
}

CorpusDefinition CorpusDatabaseConnectionDialog::corpusDefinition()
{
    CorpusDefinition def;
    def.corpusID = ui->editCorpusID->text();
    def.corpusName = ui->editCorpusName->text();
    def.basePath = def.baseMediaPath = ui->editBaseFolder->text();
    def.datastoreMetadata.type = DatastoreInfo::SQL;
    def.datastoreMetadata.driver = ui->comboBoxDatabaseTypeMetadata->currentData().toString();
    def.datastoreMetadata.hostname = ui->editHostnameMetadata->text();
    def.datastoreMetadata.username = ui->editUsernameMetadata->text();
    def.datastoreMetadata.password = ui->editPasswordMetadata->text();
    def.datastoreMetadata.usePassword = (def.datastoreMetadata.password.isEmpty()) ? false : true;
    def.datastoreMetadata.datasource = ui->editDatabaseNameMetadata->text();
    def.datastoreAnnotations.type = DatastoreInfo::SQL;
    def.datastoreAnnotations.driver = ui->comboBoxDatabaseTypeAnnotations->currentData().toString();
    def.datastoreAnnotations.hostname = ui->editHostnameAnnotations->text();
    def.datastoreAnnotations.username = ui->editUsernameAnnotations->text();
    def.datastoreAnnotations.password = ui->editPasswordAnnotations->text();
    def.datastoreAnnotations.usePassword = (def.datastoreAnnotations.password.isEmpty()) ? false : true;
    def.datastoreAnnotations.datasource = ui->editDatabaseNameAnnotations->text();
    return def;
}

void CorpusDatabaseConnectionDialog::selectBaseFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->editBaseFolder->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editBaseFolder->setText(directory);
}

void CorpusDatabaseConnectionDialog::datastoreInfoChanged()
{
    if (ui->optionUseSameDatabase->isChecked()) {
        ui->comboBoxDatabaseTypeAnnotations->setCurrentIndex(ui->comboBoxDatabaseTypeMetadata->currentIndex());
        ui->editHostnameAnnotations->setText(ui->editHostnameMetadata->text());
        ui->editUsernameAnnotations->setText(ui->editUsernameMetadata->text());
        ui->editPasswordAnnotations->setText(ui->editPasswordMetadata->text());
        ui->editDatabaseNameAnnotations->setText(ui->editDatabaseNameMetadata->text());
    }
}

void CorpusDatabaseConnectionDialog::useSameDatabaseChanged()
{
    if (ui->optionUseSameDatabase->isChecked()) {
        ui->groupBoxAnnotationsDb->setEnabled(false);
        datastoreInfoChanged();
    } else {
        ui->groupBoxAnnotationsDb->setEnabled(true);
    }
}

