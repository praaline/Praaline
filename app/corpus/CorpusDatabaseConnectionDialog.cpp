#include <QObject>
#include <QString>
#include <QFileDialog>
#include "CorpusDatabaseConnectionDialog.h"
#include "ui_CorpusDatabaseConnectionDialog.h"
#include "PraalineCore/Datastore/CorpusRepositoryDefinition.h"
using namespace Praaline::Core;

CorpusDatabaseConnectionDialog::CorpusDatabaseConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CorpusDatabaseConnectionDialog)
{
    ui->setupUi(this);
    connect(ui->commandConnect, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(ui->commandCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    // Database types
    QStringList databaseTypeNames; databaseTypeNames << tr("SQLite database") << tr("MySQL database") << tr("PostgreSQL database");
    QStringList databaseTypeDrivers; databaseTypeDrivers << "QSQLITE" << "QMYSQL" << "QPSQL";
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

CorpusRepositoryDefinition CorpusDatabaseConnectionDialog::corpusDefinition()
{
    CorpusRepositoryDefinition def;
    def.repositoryID = ui->editCorpusID->text();
    def.repositoryName = ui->editCorpusName->text();
    def.basePath = ui->editBaseFolder->text();
    def.basePathMedia = ui->editBaseFolder->text();
    def.infoDatastoreMetadata.type = DatastoreInfo::SQL;
    def.infoDatastoreMetadata.driver = ui->comboBoxDatabaseTypeMetadata->currentData().toString();
    def.infoDatastoreMetadata.hostname = ui->editHostnameMetadata->text();
    def.infoDatastoreMetadata.username = ui->editUsernameMetadata->text();
    def.infoDatastoreMetadata.password = ui->editPasswordMetadata->text();
    def.infoDatastoreMetadata.usePassword = (def.infoDatastoreMetadata.password.isEmpty()) ? false : true;
    def.infoDatastoreMetadata.datasource = ui->editDatabaseNameMetadata->text();
    def.infoDatastoreAnnotations.type = DatastoreInfo::SQL;
    def.infoDatastoreAnnotations.driver = ui->comboBoxDatabaseTypeAnnotations->currentData().toString();
    def.infoDatastoreAnnotations.hostname = ui->editHostnameAnnotations->text();
    def.infoDatastoreAnnotations.username = ui->editUsernameAnnotations->text();
    def.infoDatastoreAnnotations.password = ui->editPasswordAnnotations->text();
    def.infoDatastoreAnnotations.usePassword = (def.infoDatastoreAnnotations.password.isEmpty()) ? false : true;
    def.infoDatastoreAnnotations.datasource = ui->editDatabaseNameAnnotations->text();
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
        ui->groupBoxMetadataDb->setTitle(tr("Metadata Database Connection"));
        ui->groupBoxAnnotationsDb->setTitle(tr("Annotations Database Connection"));
        datastoreInfoChanged();
    } else {
        ui->groupBoxMetadataDb->setTitle(tr("Metadata and Annotations Database Connection"));
        ui->groupBoxAnnotationsDb->setTitle("");
        ui->groupBoxAnnotationsDb->setEnabled(true);
    }
}

