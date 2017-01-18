#include "ExtractSoundBitesWidget.h"
#include "ui_ExtractSoundBitesWidget.h"

#include <QString>
#include <QList>
#include <QPointer>
#include <QFileDialog>
#include <QStandardItem>
#include <QStandardItemModel>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusRecording.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/FileDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "pnlib/mediautil/AudioSegmenter.h"
#include "pnlib/mediautil/SoundInfo.h"
#include "CorpusRepositoriesManager.h"


struct ExtractSoundBitesWidgetData {
    ExtractSoundBitesWidgetData() : manualSelectionModel(0), repositoryCopyOver(0) {}

    GridViewWidget *manualSelectionTable;
    QStandardItemModel *manualSelectionModel;
    CorpusRepositoriesManager *corpusRepositoriesManager;
    QPointer<CorpusRepository> repositoryCopyOver;
};


ExtractSoundBitesWidget::ExtractSoundBitesWidget(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::ExtractSoundBitesWidget), d(new ExtractSoundBitesWidgetData)
{
    ui->setupUi(this);
    // Controls
    connect(ui->commandBrowseForOutputFolder, SIGNAL(clicked(bool)), this, SLOT(browseForFolder()));
    connect(ui->commandExtract, SIGNAL(clicked(bool)), this, SLOT(extractSoundBites()));
    ui->optionSamplingRateOriginal->setChecked(true);

    // Corpus Repositories Manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    connect(d->corpusRepositoriesManager, SIGNAL(activeCorpusRepositoryChanged(QString)), this, SLOT(activeCorpusChanged(QString)));

    // Manual selection model - table
    d->manualSelectionTable = new GridViewWidget(this);
    ui->gridLayoutManualSelection->addWidget(d->manualSelectionTable);
    d->manualSelectionModel = new QStandardItemModel(200, 5, this);
    d->manualSelectionModel->setHorizontalHeaderLabels(
                QStringList() << tr("Sound Bite Name") << tr("Communication ID") << tr("Recording ID")
                << tr("Start Time (s)") << tr("End Time (s)"));
    d->manualSelectionTable->tableView()->setModel(d->manualSelectionModel);
}

ExtractSoundBitesWidget::~ExtractSoundBitesWidget()
{
    delete ui;
    delete d;
}

void ExtractSoundBitesWidget::activeCorpusRepositoryChanged(const QString &repositoryID)
{
    Q_UNUSED(repositoryID)
    ui->comboBoxLevelsToCopy->clear();
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    int i = 0;
    foreach (AnnotationStructureLevel *level, repository->annotationStructure()->levels()) {
        ui->comboBoxLevelsToCopy->insertItem(i, level->name(), true);
        ++i;
    }
}

void ExtractSoundBitesWidget::browseForFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"), ui->editOutputFolder->text(),
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directory.isEmpty())
        ui->editOutputFolder->setText(directory);
}

void ExtractSoundBitesWidget::extractSoundBites()
{
    if (!d->manualSelectionModel) return;
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;

    QString outputPath = ui->editOutputFolder->text();
    if (outputPath.isEmpty()) {
        outputPath = repository->files()->basePath();
        ui->editOutputFolder->setText(outputPath);
    }
    RealTime paddingLeft = RealTime::fromMilliseconds(ui->spinBoxPaddingBeforeStart->value());
    RealTime paddingRight = RealTime::fromMilliseconds(ui->spinBoxPaddingAfterEnd->value());
    bool normalise = ui->checkNormalise->isChecked();
    int resampleRate = ui->spinBoxSamplingRate->value();
    if (ui->optionSamplingRateOriginal->isChecked()) resampleRate = 0;

    // Count how many sound bites we'll produce
    int totalBites = 0;
    for (int row = 0; row < d->manualSelectionModel->rowCount(); ++row) {
        QString communicationID = d->manualSelectionModel->data(d->manualSelectionModel->index(row, 1)).toString();
        if (!communicationID.isEmpty()) totalBites++;
    }
    if (totalBites == 0) return;

    // If we'll be copying over annotations, prepare the target corpus
    if (ui->checkCreateCorpus) {
        prepareCopyOverCorpus(repository, outputPath);
    }

//    // Prepare the progress bar
//    ui->progressBar->setValue(0);
//    ui->progressBar->setMaximum(totalBites);
//    int progress = 0;
//    // Process the table and create sound bites
//    for (int row = 0; row < d->manualSelectionModel->rowCount(); ++row) {
//        QString stimulusName = d->manualSelectionModel->data(d->manualSelectionModel->index(row, 0)).toString();
//        QString communicationID = d->manualSelectionModel->data(d->manualSelectionModel->index(row, 1)).toString();
//        QString recordingID = d->manualSelectionModel->data(d->manualSelectionModel->index(row, 2)).toString();
//        if (recordingID.isEmpty()) recordingID = communicationID;
//        QString timeStartStr = d->manualSelectionModel->data(d->manualSelectionModel->index(row, 3)).toString();
//        double timeStartSec = timeStartStr.replace(",", ".").toDouble();
//        QString timeEndStr = d->manualSelectionModel->data(d->manualSelectionModel->index(row, 4)).toString();
//        double timeEndSec = timeEndStr.replace(",", ".").toDouble();
//        if (stimulusName.isEmpty())
//            stimulusName = QString("%1_%2_%3").arg(communicationID).arg(timeStartSec).arg(timeEndSec);

//        QPointer<CorpusCommunication> com = corpus->communication(communicationID);
//        if (!com) continue;
//        QPointer<CorpusRecording> rec = com->recording(recordingID);
//        if (!rec) continue;
//        QString originalMediaFile = rec->filePath();

//        QList<Interval *> intv;
//        RealTime start = RealTime::fromSeconds(timeStartSec) - paddingLeft;
//        if (start < RealTime(0, 0)) start = RealTime(0, 0);
//        RealTime end = RealTime::fromSeconds(timeEndSec) + paddingRight;
//        intv << new Interval(start, end, stimulusName);

//        AudioSegmenter::segment(originalMediaFile, outputPath, intv, "", resampleRate, normalise);

//        // Add to copy-over corpus and add annotations, if needed
//        if (ui->checkCreateCorpus && d->corpusCopyOver) {
//            // Create Communication
//            QPointer<CorpusCommunication> stimCom = d->corpusCopyOver->communication(stimulusName);
//            if (!stimCom) {
//                stimCom = new CorpusCommunication(stimulusName);
//                stimCom->setName(stimulusName);
//                d->corpusCopyOver->addCommunication(stimCom);
//            }
//            // Carry over participations and speakers
//            foreach (QPointer<CorpusParticipation> participation, corpus->participationsForCommunication(communicationID)) {
//                if (!d->corpusCopyOver->hasSpeaker(participation->speakerID())) {
//                    d->corpusCopyOver->addSpeaker(new CorpusSpeaker(participation->speakerID()));
//                }
//                d->corpusCopyOver->addParticipation(stimCom->ID(), participation->speakerID(), participation->role());
//            }
//            // Update recording
//            QPointer<CorpusRecording> stimRec = stimCom->recording(stimulusName);
//            if (!stimRec) {
//                stimRec = new CorpusRecording(stimulusName);
//                stimRec->setName(stimulusName);
//                stimRec->setFilename(QString("%1.wav").arg(stimulusName));
//                SoundInfo info;
//                if (SoundInfo::getSoundInfo(d->corpusCopyOver->baseMediaPath() + "/" + stimRec->filename(), info)) {
//                    stimRec->setFormat("wav");
//                    stimRec->setChannels(info.channels);
//                    stimRec->setSampleRate(info.sampleRate);
//                    stimRec->setPrecisionBits(info.precisionBits);
//                    stimRec->setDuration(info.duration);
//                    stimRec->setBitRate(info.bitRate);
//                    stimRec->setEncoding(info.encoding);
//                    stimRec->setFileSize(info.filesize);
//                    stimRec->setChecksumMD5(info.checksumMD5);
//                }
//                stimCom->addRecording(stimRec);
//            }
//            // Carry over annotations
//            QPointer<CorpusAnnotation> annotSource = com->annotation(recordingID);
//            if (annotSource) {
//                QPointer<CorpusAnnotation> stimAnnot = stimCom->annotation(stimulusName);
//                if (!stimAnnot) {
//                    stimAnnot = new CorpusAnnotation(stimulusName);
//                    stimAnnot->setName(stimulusName);
//                    stimCom->addAnnotation(stimAnnot);
//                }
//                // in any case (replaces annotations already in the destination corpus)
//                carryOverAnnotations(corpus, annotSource, stimAnnot, start, end);
//            }
//        }

//        progress++;
//        ui->progressBar->setValue(progress);
//        QApplication::processEvents();
//    }
//    ui->progressBar->setValue(ui->progressBar->maximum());
}

void ExtractSoundBitesWidget::prepareCopyOverCorpus(QPointer<CorpusRepository> repositorySource, const QString &outputPath)
{
//    // Local DB corpus, using ID = name = metadata = annotation database name, stored in outputPath
//    QString errorMessages;
//    QString corpusName = ui->editNewCorpusName->text();
//    QDir dir(outputPath);
//    QString baseFolder = dir.absolutePath() + "/";

//    CorpusDefinition destCorpusDef;
//    if (destCorpusDef.load(baseFolder + corpusName + ".corpus")) {
//        d->corpusCopyOver = Corpus::open(destCorpusDef, errorMessages);
//        if (!d->corpusCopyOver) return;
//    }
//    else {

//        destCorpusDef.corpusID = corpusName;
//        destCorpusDef.corpusName = corpusName;
//        destCorpusDef.datastoreMetadata = DatastoreInfo(DatastoreInfo::SQL, "QSQLITE", "",
//                                                       baseFolder + corpusName + ".db", "", "");
//        destCorpusDef.datastoreAnnotations = DatastoreInfo(DatastoreInfo::SQL, "QSQLITE", "",
//                                                          baseFolder  + corpusName + ".db", "", "");
//        destCorpusDef.baseMediaPath = baseFolder;

//        d->corpusCopyOver = Corpus::create(destCorpusDef, errorMessages);
//        if (!d->corpusCopyOver) return;
//        destCorpusDef.save(baseFolder + corpusName + ".corpus");
//    }
//    // Copy over the metadata and annotation structure
//    d->corpusCopyOver->importMetadataStructure(corpusSource->metadataStructure());
//    d->corpusCopyOver->importAnnotationStructure(corpusSource->annotationStructure());
//    d->corpusCopyOver->save();
}

void ExtractSoundBitesWidget::carryOverAnnotations(QPointer<CorpusRepository> repositorySource,
                                                   QPointer<CorpusAnnotation> annotSource, QPointer<CorpusAnnotation> annotDestination,
                                                   RealTime start, RealTime end)
{
    if (!d->repositoryCopyOver) return;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersSourceAll = repositorySource->annotations()->getTiersAllSpeakers(annotSource->ID());
    foreach (QString speakerID, tiersSourceAll.keys()) {
        QPointer<AnnotationTierGroup> tiersSource = tiersSourceAll.value(speakerID);
        if (!tiersSource) continue;
        QPointer<AnnotationTierGroup> tiersDest = new AnnotationTierGroup();
        for (int i = 0; i < ui->comboBoxLevelsToCopy->count(); ++i) {
            if (ui->comboBoxLevelsToCopy->itemData(i).toBool() == true) {
                AnnotationStructureLevel *level = repositorySource->annotationStructure()->levels().at(i);
                if (!level) continue;
                // TODO: ensure all types of levels are copied over!!!
                if (level->levelType() != AnnotationStructureLevel::IndependentIntervalsLevel &&
                    level->levelType() != AnnotationStructureLevel::GroupingLevel) continue;
                IntervalTier *tierIntvSource = tiersSource->getIntervalTierByName(level->ID());
                if (tierIntvSource) {
                    IntervalTier *tierIntvDest = tierIntvSource->getIntervalTierSubset(start, end);
                    if (tierIntvDest->count() == 1 && tierIntvDest->first()->isPauseSilent()) continue;
                    tiersDest->addTier(tierIntvDest);
                }
                PointTier *tierPointSource = tiersSource->getPointTierByName(level->ID());
                if (tierPointSource) {

                }
            }
        }
        d->repositoryCopyOver->annotations()->saveTiers(annotDestination->ID(), speakerID, tiersDest);
        delete tiersDest;
    }
    qDeleteAll(tiersSourceAll);
}


