#include <QString>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>
#include "NewCorpusRepositoryWizard.h"
#include "ui_NewCorpusRepositoryWizard.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserMetadataStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserAnnotationStructure.h"
using namespace Praaline::Core;

struct NewCorpusRepositoryWizardData {
    NewCorpusRepositoryWizardData() : newCorpusRepository(0), modelMetadataTemplates(0), modelAnnotationTemplates(0)
    {}

    CorpusRepositoryDefinition newDefinition;
    CorpusRepository *newCorpusRepository;
    // Models for metadata and annotation schema templates
    QStandardItemModel *modelMetadataTemplates;
    QStandardItemModel *modelAnnotationTemplates;
};

NewCorpusRepositoryWizard::NewCorpusRepositoryWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::NewCorpusRepositoryWizard), d(new NewCorpusRepositoryWizardData)
{
    ui->setupUi(this);
    // XML files with metadata and annotation templates
    populateTemplates();
    // Connect signals for the wizard
    connect(ui->commandSelectFolder, SIGNAL(clicked()), this, SLOT(localDbSelectFolder()));
    connect(ui->editMetadataDatabase, SIGNAL(textChanged(QString)), this, SLOT(localDbDatabaseNameChanged()));
    connect(ui->optionSameDatastore, SIGNAL(toggled(bool)), this, SLOT(localDbDatabaseNameChanged()));
}

NewCorpusRepositoryWizard::~NewCorpusRepositoryWizard()
{
    delete ui;
    delete d;
}

CorpusRepositoryDefinition NewCorpusRepositoryWizard::newDefinition()
{
    return d->newDefinition;
}

CorpusRepository *NewCorpusRepositoryWizard::newCorpusRepository()
{
    return d->newCorpusRepository;
}

void NewCorpusRepositoryWizard::populateTemplates()
{
    d->modelMetadataTemplates = new QStandardItemModel(this);
    d->modelAnnotationTemplates = new QStandardItemModel(this);
    d->modelMetadataTemplates->setColumnCount(1);
    d->modelAnnotationTemplates->setColumnCount(1);
    QStandardItem *item(0);
    // Praaline searches in the directory "templates", under the application path, for subdirectories
    // "metadata" and "annotation" containing XML files of schema templates
    QDir dirinfo;
    QFileInfoList list;
    // Metadata templates
    dirinfo.setPath(QApplication::applicationDirPath() + "/templates/metadata");
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
    dirinfo.setPath(QApplication::applicationDirPath() + "/templates/annotation");
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

bool NewCorpusRepositoryWizard::validateCurrentPage()
{
    if (currentId() == 0) {
        if (ui->editCorpusID->text().isEmpty()) {
            QMessageBox::warning(this, tr("Corpus ID cannot be empty"),
                                 tr("You must provide an ID for your corpus. "
                                    "This is a short name that will be used to identify samples belonging to this corpus in the database."));
            return false;
        }
        if (ui->optionLocalDB->isChecked()) {
            ui->editMetadataDatabase->setText(ui->editCorpusID->text());
            ui->editAnnotationDatabase->setText(ui->editCorpusID->text());
        }
    }
    else if (currentId() == 1) {
        if (ui->optionLocalDB->isChecked())
            createLocalSQLRepository();
        else if (ui->optionFiles->isChecked())
            createLocalXMLRepository();
        else if (ui->optionRemoteDB->isChecked())
            createRemoteSQLRepository();
    }
    return true;
}

void NewCorpusRepositoryWizard::localDbSelectFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->editBaseFolder->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editBaseFolder->setText(directory);
}

void NewCorpusRepositoryWizard::localDbDatabaseNameChanged()
{
    if (ui->optionSameDatastore->isChecked()) {
        ui->editAnnotationDatabase->setText(ui->editMetadataDatabase->text());
        ui->editAnnotationDatabase->setEnabled(false);
    }
    else {
        ui->editAnnotationDatabase->setEnabled(true);
    }
}

void NewCorpusRepositoryWizard::createLocalSQLRepository()
{
    QString repositoryID = ui->editCorpusID->text();
    QString repositoryName = ui->editCorpusName->text();
    QDir dir(ui->editBaseFolder->text());
    QString basePath = dir.absolutePath() + "/";
    QString databaseNameMetadata = ui->editMetadataDatabase->text();
    QString databaseNameAnnotations = ui->editAnnotationDatabase->text();
    QString errorMessages;
    if      (databaseNameMetadata.isEmpty())    databaseNameMetadata = databaseNameAnnotations;
    else if (databaseNameAnnotations.isEmpty()) databaseNameAnnotations = databaseNameMetadata;
    if (databaseNameMetadata.isEmpty()) {
        databaseNameMetadata = databaseNameAnnotations = repositoryID;
    }

    d->newDefinition.repositoryID = repositoryID;
    d->newDefinition.repositoryName = repositoryName;
    d->newDefinition.infoDatastoreMetadata = DatastoreInfo(
                DatastoreInfo::SQL, "QSQLITE", "",
                basePath + databaseNameMetadata + ".db", "", "");
    d->newDefinition.infoDatastoreAnnotations = DatastoreInfo(
                DatastoreInfo::SQL, "QSQLITE", "",
                basePath + databaseNameAnnotations + ".db", "", "");
    d->newDefinition.basePath = basePath;
    d->newCorpusRepository = CorpusRepository::create(d->newDefinition, errorMessages);
    if (!d->newCorpusRepository) {
        QMessageBox::warning(this, tr("Error creating corpus repository"), QString("%1\n%2")
                             .arg(tr("There was an error while creating the new repository."))
                             .arg(errorMessages));
        return;
    }
    d->newDefinition.save(basePath + repositoryID + ".PraalineRepository");
    applyTemplates();
}

void NewCorpusRepositoryWizard::createRemoteSQLRepository()
{

}

void NewCorpusRepositoryWizard::createLocalXMLRepository()
{

}

void NewCorpusRepositoryWizard::applyTemplates()
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
