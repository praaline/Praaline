#include <memory>
using namespace std;

#include <QDebug>
#include <QPointer>
#include <QFileInfo>
#include <QCloseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include "CheckMediaFilesDialog.h"
#include "ui_CheckMediaFilesDialog.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Datastore/FileDatastore.h"
using namespace Praaline::Core;

#include "PraalineMedia/SoundInfo.h"
using namespace Praaline::Media;

struct CheckMediaFilesDialogData {
    CorpusRepository *repository;
    QList<CorpusRecording *> recordings;
    QStandardItemModel *model;
    bool stop;
};

CheckMediaFilesDialog::CheckMediaFilesDialog(CorpusRepository *repository, QWidget *parent) :
    QDialog(parent), ui(new Ui::CheckMediaFilesDialog), d(new CheckMediaFilesDialogData)
{
    ui->setupUi(this);

    d->repository = repository;
    d->stop = false;

    // Get all recordings in the repository from the database
    d->recordings = d->repository->metadata()->getRecordings(MetadataDatastore::Selection("", "", ""));

    // Create a list model for the recordings
    d->model = new QStandardItemModel(this);
    d->model->setColumnCount(5);
    int countRecordings = 0;
    foreach (CorpusRecording *rec, d->recordings) {
        if (!rec) continue;
        QList<QStandardItem *> items;
        items << new QStandardItem(rec->corpusID());
        items << new QStandardItem(rec->communicationID());
        items << new QStandardItem(rec->ID());
        items << new QStandardItem(rec->filename());
        items << new QStandardItem("");
        d->model->appendRow(items);
        countRecordings++;
    }
    d->model->setRowCount(countRecordings);
    QStringList headerLabels; headerLabels << tr("Corpus ID") << tr("Communication ID") << tr("Recording ID") << tr("Filename") << tr("Status");
    d->model->setHorizontalHeaderLabels(headerLabels);
    // Show the model in the list, resize columns to contents
    ui->treeviewRecordings->setModel(d->model);
    for (int i = 0; i < 4; ++i) ui->treeviewRecordings->resizeColumnToContents(i);
    // Connect OK, Cancel and Process button signals
    connect(this, &QDialog::rejected, this, &CheckMediaFilesDialog::stop);
    connect(ui->commandClose, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(ui->commandProcess, &QAbstractButton::clicked, this, &CheckMediaFilesDialog::process);
}

CheckMediaFilesDialog::~CheckMediaFilesDialog()
{
    delete ui;
    qDeleteAll(d->recordings);
    delete d;
}

void CheckMediaFilesDialog::stop()
{
    d->stop = true;
}

void CheckMediaFilesDialog::process()
{
    int numberOfRecordings = d->model->rowCount();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(numberOfRecordings);
    ui->progressBar->setValue(0);
    d->stop = false;
    for (int i = 0; i < numberOfRecordings; ++i) {
        if (i % 10 == 0) QApplication::processEvents();
        // this is used to break the loop when the user closes the dialog
        if (d->stop) return;
        CorpusRecording *rec = d->recordings.at(i);
        if (!rec) {
            d->model->setItem(i, 4, new QStandardItem(tr("Recording deleted")));
            continue;
        }
        SoundInfo info;
        bool ok = SoundInfo::getSoundInfo(rec->filePath(), info);
        if (!ok) {
            d->model->setItem(i, 4, new QStandardItem(tr("File not found!")));
            continue;
        }
        else {
            QFileInfo fileinfo(rec->filePath());
            rec->setFormat(fileinfo.suffix().toLower());
            rec->setChannels(info.channels);
            rec->setSampleRate(info.sampleRate);
            rec->setPrecisionBits(info.precisionBits);
            rec->setDuration(info.duration);
            rec->setBitRate(info.bitRate);
            rec->setEncoding(info.encoding);
            rec->setFileSize(info.filesize);
            rec->setChecksumMD5(info.checksumMD5);
            if (rec->checksumMD5() == info.checksumMD5) {
                d->model->setItem(i, 4, new QStandardItem(tr("OK")));
            } else {
                d->model->setItem(i, 4, new QStandardItem(tr("UPDATED")));
            }
            rec->save();
        }
        ui->progressBar->setValue(i);
    }
    ui->progressBar->setValue(numberOfRecordings);
}

