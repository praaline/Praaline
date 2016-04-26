#include <QPointer>
#include <QFileInfo>
#include <QCloseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include "pncore/corpus/corpus.h"
#include "pnlib/mediautil/SoundInfo.h"

#include "ImportCorpusItemsWizardProcessMediaPage.h"
#include "ui_ImportCorpusItemsWizardProcessMediaPage.h"

ImportCorpusItemsWizardProcessMediaPage::ImportCorpusItemsWizardProcessMediaPage(
        QMap<QPair<QString, QString>, QPointer<CorpusRecording> > &candidateRecordings,
        QWidget *parent) :
    QWizardPage(parent), ui(new Ui::ImportCorpusItemsWizardProcessMediaPage),
    m_candidateRecordings(candidateRecordings), m_stop(false)
{
    ui->setupUi(this);

    setTitle("Analyse media files");
    setSubTitle("In this first step the media file Recordings will be analysed, so that their properties can be saved into Praaline's corpus database. "
                "This information allows Praaline to detect if changes were made on them at a later stage.");
    connect(ui->commandStop, SIGNAL(clicked()), this, SLOT(stopProcess()));
}

ImportCorpusItemsWizardProcessMediaPage::~ImportCorpusItemsWizardProcessMediaPage()
{
    delete ui;
}

void ImportCorpusItemsWizardProcessMediaPage::initializePage()
{
    // Create list
    m_model = new QStandardItemModel(this);
    m_model->setColumnCount(4);

    QMap<QPair<QString, QString>, QPointer<CorpusRecording> >::iterator i;
    for (i = m_candidateRecordings.begin(); i != m_candidateRecordings.end(); ++i) {
        if (!i.value()) continue;
        QList<QStandardItem *> items;
        items << new QStandardItem(i.key().first); // Communication ID
        items << new QStandardItem(i.key().second); // Recording ID
        items << new QStandardItem(i.value()->filename());
        items << new QStandardItem("");
        m_model->appendRow(items);
    }
    m_model->setRowCount(m_candidateRecordings.count());
    QStringList headerLabels; headerLabels << "Communication ID" << "Recording ID" << "Filename" << "Status";
    m_model->setHorizontalHeaderLabels(headerLabels);
    ui->treeviewRecordings->setModel(m_model);
    for (int i = 0; i < 3; ++i) ui->treeviewRecordings->resizeColumnToContents(i);
}

void ImportCorpusItemsWizardProcessMediaPage::stopProcess()
{
    m_stop = true;
}

bool ImportCorpusItemsWizardProcessMediaPage::validatePage()
{
    int numberOfRecordings = m_model->rowCount();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(numberOfRecordings);
    ui->progressBar->setValue(0);
    m_stop = false;
    for (int i = 0; i < numberOfRecordings; ++i) {
        // this is used to break the loop when the user closes the dialog
        if (m_stop) return false;

        QString communicationID = m_model->data(m_model->index(i, 0)).toString();
        QString recordingID = m_model->data(m_model->index(i, 1)).toString();
        QString filename = m_model->data(m_model->index(i, 2)).toString();

        QPointer<CorpusRecording> rec = m_candidateRecordings.value(QPair<QString, QString>(communicationID, recordingID));
        if (!rec) {
            m_model->setItem(i, 3, new QStandardItem("Recording deleted"));
            continue;
        }
        SoundInfo info;
        bool ok = SoundInfo::getSoundInfo(filename, info);
        if (!ok) {
            m_model->setItem(i, 3, new QStandardItem("File not found!"));
            continue;
        }
        if (rec->checksumMD5() == info.checksumMD5) {
            m_model->setItem(i, 3, new QStandardItem("OK"));
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
            m_model->setItem(i, 3, new QStandardItem("UPDATED"));
        }
        ui->progressBar->setValue(i);
        if ((i % 100) == 0) QApplication::processEvents();
    }
    ui->progressBar->setValue(numberOfRecordings);
    return true;
}

