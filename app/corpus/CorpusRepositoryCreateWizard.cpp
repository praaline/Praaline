#include <QString>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserMetadataStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserAnnotationStructure.h"
using namespace Praaline::Core;

#include "CorpusRepositoryCreateWizard.h"
#include "ui_CorpusRepositoryCreateWizard.h"

struct CorpusRepositoryCreateWizardData {
    CorpusRepositoryCreateWizardData() :
        newCorpusRepository(nullptr), modelMetadataTemplates(nullptr), modelAnnotationTemplates(nullptr)
    {}

    CorpusRepositoryDefinition newDefinition;
    CorpusRepository *newCorpusRepository;
    // Models for metadata and annotation schema templates
    QStandardItemModel *modelMetadataTemplates;
    QStandardItemModel *modelAnnotationTemplates;
};

CorpusRepositoryCreateWizard::CorpusRepositoryCreateWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::CorpusRepositoryCreateWizard), d(new CorpusRepositoryCreateWizardData)
{
    ui->setupUi(this);
    // Default values
    ui->editBaseFolder->setText(QDir::homePath());
    // XML files with metadata and annotation templates
    populateTemplates();
    // Connect signals for the wizard
    connect(ui->commandSelectFolder, SIGNAL(clicked()), this, SLOT(selectMediaBaseFolder()));
    connect(ui->editDatabaseNameMetadata, SIGNAL(textChanged(QString)), this, SLOT(databaseNameChanged()));
    connect(ui->optionSameDatastore, SIGNAL(toggled(bool)), this, SLOT(databaseNameChanged()));
}

CorpusRepositoryCreateWizard::~CorpusRepositoryCreateWizard()
{
    delete ui;
    delete d;
}

CorpusRepositoryDefinition CorpusRepositoryCreateWizard::newDefinition()
{
    return d->newDefinition;
}

CorpusRepository *CorpusRepositoryCreateWizard::newCorpusRepository()
{
    return d->newCorpusRepository;
}

void CorpusRepositoryCreateWizard::populateTemplates()
{
    d->modelMetadataTemplates = new QStandardItemModel(this);
    d->modelAnnotationTemplates = new QStandardItemModel(this);
    d->modelMetadataTemplates->setColumnCount(1);
    d->modelAnnotationTemplates->setColumnCount(1);
    QStandardItem *item(nullptr);
    // Praaline searches in the directory "templates", under the application path, for subdirectories
    // "metadata" and "annotation" containing XML files of schema templates
    QDir dirinfo;
    QFileInfoList list;
    // Metadata templates
    dirinfo.setPath(QDir::homePath() + "/Praaline/templates/metadata");
    dirinfo.setNameFilters(QStringList() << "*.xml");
    list << dirinfo.entryInfoList();
    int i = 0;
    foreach (QFileInfo info, list) {
        item = new QStandardItem(info.baseName());
        item->setData(info.absoluteFilePath());
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        d->modelMetadataTemplates->setItem(i, 0, item);
        ++i;
    }
    ui->treeviewMetadataTemplates->setModel(d->modelMetadataTemplates);
    ui->treeviewMetadataTemplates->setHeaderHidden(true);
    // Annotation templates
    dirinfo.setPath(QDir::homePath() + "/Praaline/templates/annotation");
    dirinfo.setNameFilters(QStringList() << "*.xml");
    list << dirinfo.entryInfoList();
    list.clear();
    i = 0;
    foreach (QFileInfo info, list) {
        item = new QStandardItem(info.baseName());
        item->setData(info.absoluteFilePath());
        item->setCheckable(true);
        item->setCheckState(Qt::Unchecked);
        d->modelAnnotationTemplates->setItem(i, 0, item);
        ++i;
    }
    ui->treeviewAnnotationTemplates->setModel(d->modelAnnotationTemplates);
    ui->treeviewAnnotationTemplates->setHeaderHidden(true);
}

bool CorpusRepositoryCreateWizard::validateCurrentPage()
{
    if (currentId() == 0) {
        if (ui->editCorpusRepositoryID->text().isEmpty()) {
            QMessageBox::warning(this, tr("Corpus Repository ID cannot be empty"),
                                 tr("You must provide an ID for your corpus repository. "
                                    "This is a short name that will be used to identify the repository."));
            return false;
        }
        if (ui->optionLocalDB->isChecked()) {
            // LOCAL DATABASE
            // Database types
            QStringList databaseTypeNames; databaseTypeNames << tr("SQLite database");
            QStringList databaseTypeDrivers; databaseTypeDrivers << "QSQLITE";
            ui->comboBoxDatabaseTypeMetadata->clear();
            ui->comboBoxDatabaseTypeAnnotations->clear();
            for (int i = 0; i < databaseTypeDrivers.count(); ++i) {
                ui->comboBoxDatabaseTypeMetadata->addItem(databaseTypeNames.at(i), databaseTypeDrivers.at(i));
                ui->comboBoxDatabaseTypeAnnotations->addItem(databaseTypeNames.at(i), databaseTypeDrivers.at(i));
            }
            ui->comboBoxDatabaseTypeMetadata->setCurrentIndex(0);
            // Disable server information
            ui->editHostnameMetadata->clear();      ui->editHostnameMetadata->setEnabled(false);
            ui->editUsernameMetadata->clear();      ui->editUsernameMetadata->setEnabled(false);
            ui->editPasswordMetadata->clear();      ui->editPasswordMetadata->setEnabled(false);
            ui->editHostnameAnnotations->clear();   ui->editHostnameAnnotations->setEnabled(false);
            ui->editUsernameAnnotations->clear();   ui->editUsernameAnnotations->setEnabled(false);
            ui->editPasswordAnnotations->clear();   ui->editPasswordAnnotations->setEnabled(false);
            // Default name
            ui->editDatabaseNameMetadata->setText(ui->editCorpusRepositoryID->text());
            ui->editDatabaseNameAnnotations->setText(ui->editCorpusRepositoryID->text());
        } else {
            // REMOTE DATABASE
            // Database types
            QStringList databaseTypeNames; databaseTypeNames << tr("MySQL database") << tr("PostgreSQL database");
            QStringList databaseTypeDrivers; databaseTypeDrivers << "QMYSQL" << "QPSQL";
            ui->comboBoxDatabaseTypeMetadata->clear();
            ui->comboBoxDatabaseTypeAnnotations->clear();
            for (int i = 0; i < databaseTypeDrivers.count(); ++i) {
                ui->comboBoxDatabaseTypeMetadata->addItem(databaseTypeNames.at(i), databaseTypeDrivers.at(i));
                ui->comboBoxDatabaseTypeAnnotations->addItem(databaseTypeNames.at(i), databaseTypeDrivers.at(i));
            }
            ui->comboBoxDatabaseTypeMetadata->setCurrentIndex(0);
            // Enable server information
            ui->editHostnameMetadata->clear();      ui->editHostnameMetadata->setEnabled(true);
            ui->editUsernameMetadata->clear();      ui->editUsernameMetadata->setEnabled(true);
            ui->editPasswordMetadata->clear();      ui->editPasswordMetadata->setEnabled(true);
            ui->editHostnameAnnotations->clear();   ui->editHostnameAnnotations->setEnabled(true);
            ui->editUsernameAnnotations->clear();   ui->editUsernameAnnotations->setEnabled(true);
            ui->editPasswordAnnotations->clear();   ui->editPasswordAnnotations->setEnabled(true);
            // Default name
            ui->editDatabaseNameMetadata->setText(ui->editCorpusRepositoryID->text());
            ui->editDatabaseNameAnnotations->setText(ui->editCorpusRepositoryID->text());
        }
    }
    else if (currentId() == 2) {
        if (ui->optionLocalDB->isChecked())
            createLocalSQLRepository();
        else if (ui->optionRemoteDB->isChecked())
            createRemoteSQLRepository();
        else if (ui->optionFiles->isChecked())
            createLocalXMLRepository();
    }
    return true;
}

void CorpusRepositoryCreateWizard::selectMediaBaseFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->editBaseFolder->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editBaseFolder->setText(directory);
}

void CorpusRepositoryCreateWizard::databaseNameChanged()
{
    if (ui->optionSameDatastore->isChecked()) {
        ui->editDatabaseNameAnnotations->setText(ui->editDatabaseNameMetadata->text());
        ui->groupBoxAnnotationsDb->setEnabled(false);
    }
    else {
        ui->groupBoxAnnotationsDb->setEnabled(true);
    }
}

void CorpusRepositoryCreateWizard::createLocalSQLRepository()
{
    QString errorMessages;
    QString repositoryID = ui->editCorpusRepositoryID->text();
    QDir dir(ui->editBaseFolder->text());
    QString basePath = dir.absolutePath() + "/";
    QString databaseNameMetadata = ui->editDatabaseNameMetadata->text();
    QString databaseNameAnnotations = ui->editDatabaseNameAnnotations->text();
    if      (databaseNameMetadata.isEmpty())    databaseNameMetadata = databaseNameAnnotations;
    else if (databaseNameAnnotations.isEmpty()) databaseNameAnnotations = databaseNameMetadata;
    if (databaseNameMetadata.isEmpty()) {
        databaseNameMetadata = databaseNameAnnotations = repositoryID;
    }

    d->newDefinition.repositoryID = repositoryID;
    d->newDefinition.repositoryName = ui->editCorpusRepositoryDescription->text();
    d->newDefinition.basePath = basePath;
    d->newDefinition.basePathMedia = basePath;
    d->newDefinition.infoDatastoreMetadata = DatastoreInfo(
                DatastoreInfo::SQL, "QSQLITE", "",
                basePath + databaseNameMetadata + ".db", "", "");
    d->newDefinition.infoDatastoreAnnotations = DatastoreInfo(
                DatastoreInfo::SQL, "QSQLITE", "",
                basePath + databaseNameAnnotations + ".db", "", "");
    d->newDefinition.filenameDefinition = basePath + repositoryID + ".PraalineRepository";
    d->newCorpusRepository = CorpusRepository::create(d->newDefinition, errorMessages);
    if (!d->newCorpusRepository) {
        QMessageBox::warning(this, tr("Error creating corpus repository"), QString("%1\n%2")
                             .arg(tr("There was an error while creating the new repository."))
                             .arg(errorMessages));
        return;
    }
    d->newDefinition.save(d->newDefinition.filenameDefinition);
    applyTemplates();
}

void CorpusRepositoryCreateWizard::createRemoteSQLRepository()
{
    QString errorMessages;
    QString repositoryID = ui->editCorpusRepositoryID->text();
    d->newDefinition.repositoryID = repositoryID;
    d->newDefinition.repositoryName = ui->editCorpusRepositoryDescription->text();
    QDir dir(ui->editBaseFolder->text());
    QString basePath = dir.absolutePath() + "/";
    d->newDefinition.basePath = basePath;
    d->newDefinition.basePathMedia = basePath;
    d->newDefinition.infoDatastoreMetadata.type = DatastoreInfo::SQL;
    d->newDefinition.infoDatastoreMetadata.driver = ui->comboBoxDatabaseTypeMetadata->currentData().toString();
    d->newDefinition.infoDatastoreMetadata.hostname = ui->editHostnameMetadata->text();
    d->newDefinition.infoDatastoreMetadata.username = ui->editUsernameMetadata->text();
    d->newDefinition.infoDatastoreMetadata.password = ui->editPasswordMetadata->text();
    d->newDefinition.infoDatastoreMetadata.usePassword = (d->newDefinition.infoDatastoreMetadata.password.isEmpty()) ? false : true;
    d->newDefinition.infoDatastoreMetadata.datasource = ui->editDatabaseNameMetadata->text();
    if (ui->optionSameDatastore->isChecked()) {
        d->newDefinition.infoDatastoreAnnotations.type = d->newDefinition.infoDatastoreMetadata.type;
        d->newDefinition.infoDatastoreAnnotations.driver = d->newDefinition.infoDatastoreMetadata.driver;
        d->newDefinition.infoDatastoreAnnotations.hostname = d->newDefinition.infoDatastoreMetadata.hostname;
        d->newDefinition.infoDatastoreAnnotations.username = d->newDefinition.infoDatastoreMetadata.username;
        d->newDefinition.infoDatastoreAnnotations.password = d->newDefinition.infoDatastoreMetadata.password;
        d->newDefinition.infoDatastoreAnnotations.usePassword = d->newDefinition.infoDatastoreMetadata.usePassword;
        d->newDefinition.infoDatastoreAnnotations.datasource = d->newDefinition.infoDatastoreMetadata.datasource;
    } else {
        d->newDefinition.infoDatastoreAnnotations.type = DatastoreInfo::SQL;
        d->newDefinition.infoDatastoreAnnotations.driver = ui->comboBoxDatabaseTypeAnnotations->currentData().toString();
        d->newDefinition.infoDatastoreAnnotations.hostname = ui->editHostnameAnnotations->text();
        d->newDefinition.infoDatastoreAnnotations.username = ui->editUsernameAnnotations->text();
        d->newDefinition.infoDatastoreAnnotations.password = ui->editPasswordAnnotations->text();
        d->newDefinition.infoDatastoreAnnotations.usePassword = (d->newDefinition.infoDatastoreAnnotations.password.isEmpty()) ? false : true;
        d->newDefinition.infoDatastoreAnnotations.datasource = ui->editDatabaseNameAnnotations->text();
    }
    // Try saving, connecting and creating repository
    d->newDefinition.filenameDefinition = basePath + repositoryID + ".PraalineRepository";
    d->newCorpusRepository = CorpusRepository::create(d->newDefinition, errorMessages);
    if (!d->newCorpusRepository) {
        QMessageBox::warning(this, tr("Error creating corpus repository"), QString("%1\n%2")
                             .arg(tr("There was an error while creating the new repository."))
                             .arg(errorMessages));
        return;
    }
    d->newDefinition.save(d->newDefinition.filenameDefinition);
    applyTemplates();
}

void CorpusRepositoryCreateWizard::createLocalXMLRepository()
{

}

void CorpusRepositoryCreateWizard::applyTemplates()
{
    if (!d->newCorpusRepository) return;
    // Metadata
    for (int i = 0; i < d->modelMetadataTemplates->rowCount(); ++i) {
        QStandardItem *item = d->modelMetadataTemplates->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString filenameXML = item->data().toString();
            MetadataStructure *metadataStructure = XMLSerialiserMetadataStructure::read(filenameXML);
            if (metadataStructure) {
                d->newCorpusRepository->importMetadataStructure(metadataStructure);
            }
        }
    }
    // Annotation
    for (int i = 0; i < d->modelAnnotationTemplates->rowCount(); ++i) {
        QStandardItem *item = d->modelAnnotationTemplates->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString filenameXML = item->data().toString();
            AnnotationStructure *annotationStructure = XMLSerialiserAnnotationStructure::read(filenameXML);
            if (annotationStructure) {
                d->newCorpusRepository->importAnnotationStructure(annotationStructure);
            }
        }
    }
}
