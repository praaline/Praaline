#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

#include "MergeCorporaDialog.h"
#include "ui_MergeCorporaDialog.h"

struct MergeCorporaDialogData {
    MergeCorporaDialogData() : corpusA(0), corpusB(0) {}

    QPointer<Corpus> corpusA;
    QPointer<Corpus> corpusB;
};

MergeCorporaDialog::MergeCorporaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MergeCorporaDialog), d(new MergeCorporaDialogData)
{
    ui->setupUi(this);
}

MergeCorporaDialog::~MergeCorporaDialog()
{
    delete ui;
    delete d;
}

void MergeCorporaDialog::browseForCorpusA()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Corpus A"), "",
                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    Corpus *corpus = openCorpusFile(filename);
    if (corpus) d->corpusA = corpus;
}

void MergeCorporaDialog::browseForCorpusB()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Corpus B"), "",
                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    Corpus *corpus = openCorpusFile(filename);
    if (corpus) d->corpusB = corpus;
}

void MergeCorporaDialog::browseForDestinationFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"), ui->editDestinationFolder->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editDestinationFolder->setText(directory);
}

Corpus *MergeCorporaDialog::openCorpusFile(const QString &filename)
{
//    // Open the corpus and register it in the global object pool
//    CorpusDefinition definition;
//    if (!definition.load(filename)) {
//        QMessageBox::warning(this, tr("Cannot open corpus"),
//                             QString(tr("Cannot open corpus definition file (%1). Is it a valid Praaline XML corpus definition?")).arg(filename),
//                             QMessageBox::Ok);
//        return 0;
//    }
//    // Ask for password, if needed
//    QString password;
//    if (definition.datastoreMetadata.usePassword == true || definition.datastoreAnnotations.usePassword) {
//        password = QInputDialog::getText(this, tr("Enter password"), tr("Enter password to connect to this corpus"),
//                                         QLineEdit::Password);
//        if (definition.datastoreMetadata.usePassword) definition.datastoreMetadata.password = password;
//        if (definition.datastoreAnnotations.usePassword) definition.datastoreAnnotations.password = password;
//    }
//    // Attempt to open corpus
//    QString errorMessages;
//    Corpus *corpus = Corpus::open(definition, errorMessages);
//    if (!errorMessages.isEmpty()) {
//        QMessageBox::warning(this, tr("Error opening corpus"), errorMessages, QMessageBox::Ok);
//        return 0;
//    }
//    return corpus;
}


void MergeCorporaDialog::mergeCorpora()
{
//    if (!d->corpusA) {
//        return;
//    }
//    if (!d->corpusB) {
//        return;
//    }
//    QString destinationCorpusName = ui->editDestinationCorpusName->text();
//    if (destinationCorpusName.isEmpty()) {
//        return;
//    }
//    QString destinationCorpusFolder = ui->editDestinationFolder->text();
//    if (destinationCorpusFolder.isEmpty()) {
//        return;
//    }
//    // Create destination corpus
//    // Local DB corpus, using ID = name = metadata = annotation database name, stored in outputPath
//    QString errorMessages;
//    QDir dir(destinationCorpusFolder);
//    QString baseFolder = dir.absolutePath() + "/";
//    CorpusDefinition destinationCorpusDefinition;
//    destinationCorpusDefinition.corpusID = destinationCorpusName;
//    destinationCorpusDefinition.corpusName = destinationCorpusName;
//    destinationCorpusDefinition.datastoreMetadata = DatastoreInfo(DatastoreInfo::SQL, "QSQLITE", "",
//                                                                  baseFolder + destinationCorpusName + ".db", "", "");
//    destinationCorpusDefinition.datastoreAnnotations = DatastoreInfo(DatastoreInfo::SQL, "QSQLITE", "",
//                                                                     baseFolder  + destinationCorpusName + ".db", "", "");
//    destinationCorpusDefinition.baseMediaPath = baseFolder;

//    Corpus *destination = Corpus::create(destinationCorpusDefinition, errorMessages);
//    if (!destination) {
//        return;
//    }
//    destinationCorpusDefinition.save(baseFolder + destinationCorpusName + ".corpus");

//    // Import corpus A and corpus B into destination corpus
//    destination->importCorpus(d->corpusA, ui->editPrefixA->text());
//    destination->importCorpus(d->corpusB, ui->editPrefixB->text());

//    destination->save();
}
