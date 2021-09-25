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
#include "PraalineCore/Datastore/FileDatastore.h"
using namespace Praaline::Core;

#include "PraalineMedia/SoundInfo.h"
using namespace Praaline::Media;

struct CheckMediaFilesDialogData {
    Corpus *corpus;
    QStandardItemModel *model;
    bool stop;
};

CheckMediaFilesDialog::CheckMediaFilesDialog(Corpus *corpus, QWidget *parent) :
    QDialog(parent), ui(new Ui::CheckMediaFilesDialog), d(new CheckMediaFilesDialogData)
{
    ui->setupUi(this);

    d->corpus = corpus;
    d->stop = false;

    // d->corpus->repository()->metadata()

    // Create list
    d->model = new QStandardItemModel(this);
    d->model->setColumnCount(4);
    int countRecordings = 0;
    foreach (CorpusCommunication *com, corpus->communications()) {
        if (!com) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            QList<QStandardItem *> items;
            items << new QStandardItem(rec->communicationID());
            items << new QStandardItem(rec->ID());
            items << new QStandardItem(rec->filename());
            items << new QStandardItem("");
            d->model->appendRow(items);
            countRecordings++;
        }
    }
    d->model->setRowCount(countRecordings);
    QStringList headerLabels; headerLabels << tr("Communication ID") << tr("Recording ID") << tr("Filename") << tr("Status");
    d->model->setHorizontalHeaderLabels(headerLabels);
    ui->treeviewRecordings->setModel(d->model);
    for (int i = 0; i < 3; ++i) ui->treeviewRecordings->resizeColumnToContents(i);
    connect(this, &QDialog::rejected, this, &CheckMediaFilesDialog::stop);
    connect(ui->commandClose, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(ui->commandProcess, &QAbstractButton::clicked, this, &CheckMediaFilesDialog::process);
}

CheckMediaFilesDialog::~CheckMediaFilesDialog()
{
    delete ui;
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

        QString communicationID = d->model->data(d->model->index(i, 0)).toString();
        QString recordingID = d->model->data(d->model->index(i, 1)).toString();
        QString filename = d->model->data(d->model->index(i, 2)).toString();
        CorpusCommunication *com = d->corpus->communication(communicationID);
        if (!com) {
            d->model->setItem(i, 3, new QStandardItem(tr("Communication deleted")));
            continue;
        }
        CorpusRecording *rec = com->recording(recordingID);
        if (!rec) {
            d->model->setItem(i, 3, new QStandardItem(tr("Recording deleted")));
            continue;
        }
        SoundInfo info;
        bool ok = SoundInfo::getSoundInfo(rec->filePath(), info);
        if (!ok) {
            d->model->setItem(i, 3, new QStandardItem(tr("File not found!")));
            continue;
        }
        else {
            QFileInfo fileinfo(filename);
            rec->setFormat(fileinfo.suffix().toLower());
            rec->setChannels(info.channels);
            rec->setSampleRate(info.sampleRate);
            rec->setPrecisionBits(info.precisionBits);
            rec->setDuration(info.duration);
            rec->setBitRate(info.bitRate);
            rec->setEncoding(info.encoding);
            rec->setFileSize(info.filesize);
            rec->setChecksumMD5(info.checksumMD5);
            qDebug() << rec->duration().toDouble();
            d->model->setItem(i, 3, new QStandardItem(tr("UPDATED")));
            if (rec->checksumMD5() == info.checksumMD5) {
                d->model->setItem(i, 3, new QStandardItem(tr("OK")));
            }
        }
        ui->progressBar->setValue(i);
    }
    ui->progressBar->setValue(numberOfRecordings);
}
