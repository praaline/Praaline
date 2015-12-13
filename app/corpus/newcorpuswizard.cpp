#include <QFileDialog>
#include "newcorpuswizard.h"
#include "ui_newcorpuswizard.h"
#include "pncore/serialisers/datastoreinfo.h"

NewCorpusWizard::NewCorpusWizard(QWidget *parent) :
    QWizard(parent), ui(new Ui::NewCorpusWizard), m_newCorpus(0)
{
    ui->setupUi(this);
    connect(ui->commandSelectFolder, SIGNAL(clicked()), this, SLOT(localDbSelectFolder()));
    connect(ui->editMetadataDatabase, SIGNAL(textChanged(QString)), this, SLOT(localDbDatabaseNameChanged()));
    connect(ui->optionSameDatastore, SIGNAL(toggled(bool)), this, SLOT(localDbDatabaseNameChanged()));
}

NewCorpusWizard::~NewCorpusWizard()
{
    delete ui;
}

bool NewCorpusWizard::validateCurrentPage()
{
    if (currentId() == 0) {
        if (ui->optionLocalDB->isChecked()) {
            ui->editMetadataDatabase->setText(ui->editCorpusID->text());
            ui->editAnnotationDatabase->setText(ui->editCorpusID->text());
        }
    }
    else if (currentId() == 1) {
        if (ui->optionLocalDB->isChecked())
            createLocalDbCorpus();
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

    m_newDefinition.corpusID = corpusID;
    m_newDefinition.corpusName = corpusName;
    m_newDefinition.datastoreMetadata = DatastoreInfo(DatastoreInfo::SQL, "QSQLITE", "",
                                                      baseFolder  + databaseNameMetadata + ".db", "", "");
    m_newDefinition.datastoreAnnotations = DatastoreInfo(DatastoreInfo::SQL, "QSQLITE", "",
                                                         baseFolder  + databaseNameAnnotations + ".db", "", "");
    m_newDefinition.baseMediaPath = baseFolder;
    m_newCorpus = Corpus::create(m_newDefinition, errorMessages);
    if (!m_newCorpus) {
        return;
    }
    m_newDefinition.save(baseFolder + corpusID + ".corpus");
}

void NewCorpusWizard::createRemoteDbCorpus()
{

}

void NewCorpusWizard::createFilesCorpus()
{

}
