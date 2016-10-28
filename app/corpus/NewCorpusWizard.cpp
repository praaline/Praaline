#include <QString>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDebug>
#include "NewCorpusWizard.h"
#include "ui_NewCorpusWizard.h"
#include "pncore/serialisers/datastoreinfo.h"
#include "pncore/structure/metadatastructure.h"
#include "pncore/structure/annotationstructure.h"
#include "pncore/serialisers/xml/xmlserialisermetadatastructure.h"
#include "pncore/serialisers/xml/xmlserialiserannotationstructure.h"

struct NewCorpusWizardData {
    NewCorpusWizardData() : newCorpus(0), modelMetadataTemplates(0), modelAnnotationTemplates(0)
    {}

    CorpusDefinition newDefinition;
    Corpus *newCorpus;
    // Models for metadata and annotation schema templates
    QStandardItemModel *modelMetadataTemplates;
    QStandardItemModel *modelAnnotationTemplates;
};

NewCorpusWizard::NewCorpusWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::NewCorpusWizard), d(new NewCorpusWizardData)
{
    ui->setupUi(this);
    // XML files with metadata and annotation templates
    populateTemplates();
    // Connect signals for the wizard
    connect(ui->commandSelectFolder, SIGNAL(clicked()), this, SLOT(localDbSelectFolder()));
    connect(ui->editMetadataDatabase, SIGNAL(textChanged(QString)), this, SLOT(localDbDatabaseNameChanged()));
    connect(ui->optionSameDatastore, SIGNAL(toggled(bool)), this, SLOT(localDbDatabaseNameChanged()));
}

NewCorpusWizard::~NewCorpusWizard()
{
    delete ui;
    delete d;
}

CorpusDefinition NewCorpusWizard::newDefinition()
{
    return d->newDefinition;
}

Corpus *NewCorpusWizard::newCorpus()
{
    return d->newCorpus;
}

void NewCorpusWizard::populateTemplates()
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

bool NewCorpusWizard::validateCurrentPage()
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
        if (ui->optionLocalDB->isChecked()) {
            createLocalDbCorpus();
        }
        else if (ui->optionRemoteDB->isChecked())
            createRemoteDbCorpus();
        else if (ui->optionFiles->isChecked())
            createFilesCorpus();
    }
    return true;
}

void NewCorpusWizard::localDbSelectFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Directory"), ui->editBaseFolder->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editBaseFolder->setText(directory);
}

void NewCorpusWizard::localDbDatabaseNameChanged()
{
    if (ui->optionSameDatastore->isChecked()) {
        ui->editAnnotationDatabase->setText(ui->editMetadataDatabase->text());
        ui->editAnnotationDatabase->setEnabled(false);
    }
    else {
        ui->editAnnotationDatabase->setEnabled(true);
    }
}

void NewCorpusWizard::createLocalDbCorpus()
{
    QString corpusID = ui->editCorpusID->text();
    QString corpusName = ui->editCorpusName->text();
    QDir dir(ui->editBaseFolder->text());
    QString baseFolder = dir.absolutePath() + "/";
    QString databaseNameMetadata = ui->editMetadataDatabase->text();
    QString databaseNameAnnotations = ui->editAnnotationDatabase->text();
    QString errorMessages;
    if      (databaseNameMetadata.isEmpty())    databaseNameMetadata = databaseNameAnnotations;
    else if (databaseNameAnnotations.isEmpty()) databaseNameAnnotations = databaseNameMetadata;
    if (databaseNameMetadata.isEmpty()) {
        databaseNameMetadata = databaseNameAnnotations = corpusID;
    }

    d->newDefinition.corpusID = corpusID;
    d->newDefinition.corpusName = corpusName;
    d->newDefinition.datastoreMetadata = DatastoreInfo(
                DatastoreInfo::SQL, "QSQLITE", "",
                baseFolder  + databaseNameMetadata + ".db", "", "");
    d->newDefinition.datastoreAnnotations = DatastoreInfo(
                DatastoreInfo::SQL, "QSQLITE", "",
                baseFolder  + databaseNameAnnotations + ".db", "", "");
    d->newDefinition.baseMediaPath = baseFolder;
    d->newCorpus = Corpus::create(d->newDefinition, errorMessages);
    if (!d->newCorpus) {
        QMessageBox::warning(this, tr("Error creating corpus"), QString("%1\n%2")
                             .arg(tr("There was an error while creating the new corpus."))
                             .arg(errorMessages));
        return;
    }
    d->newDefinition.save(baseFolder + corpusID + ".corpus");
    applyTemplates();
}

void NewCorpusWizard::createRemoteDbCorpus()
{

}

void NewCorpusWizard::createFilesCorpus()
{

}

void NewCorpusWizard::applyTemplates()
{
    if (!d->newCorpus) return;
    // Metadata
    for (int i = 0; i < d->modelMetadataTemplates->rowCount(); ++i) {
        QStandardItem *item = d->modelMetadataTemplates->item(i, 0);
        if (item->checkState() == Qt::Checked) {
            QString filenameXML = item->data().toString();
            MetadataStructure *metadataStructure = XMLSerialiserMetadataStructure::read(filenameXML);
            if (metadataStructure) {
                d->newCorpus->importMetadataStructure(metadataStructure);
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
                d->newCorpus->importAnnotationStructure(annotationStructure);
            }
        }
    }
}
