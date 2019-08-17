#include <QDebug>
#include <QPointer>
#include <QApplication>
#include <QProgressBar>
#include <QStandardItemModel>
#include <QFile>
#include <QFileInfo>

#include "GlobalProsodicProfileWidget.h"
#include "ui_GlobalProsodicProfileWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationDataTable.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginProsody {

struct GlobalProsodicProfileWidgetData {
    GlobalProsodicProfileWidgetData() :
        repository(nullptr), gridviewResults(nullptr), modelResults(nullptr)
    {}

    CorpusRepository *repository;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
    GridViewWidget *gridviewMeasureDefinitions;
    QStandardItemModel *modelMeasureDefinitions;
};

GlobalProsodicProfileWidget::GlobalProsodicProfileWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::GlobalProsodicProfileWidget), d(new GlobalProsodicProfileWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Corpora combobox
    ui->comboBoxCorpus->addItems(repository->listCorporaIDs());
    // Metadata attributes
    int i = 0;
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Communication)){
        ui->comboBoxMetadataCom->insertItem(i, attr->name(), false);
        ++i;
    }
    foreach (MetadataStructureAttribute *attr, repository->metadataStructure()->attributes(CorpusObject::Type_Speaker)){
        ui->comboBoxMetadataSpk->insertItem(i, attr->name(), false);
        ++i;
    }
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Results grid view
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);
    // Measure definitions grid view
    d->gridviewMeasureDefinitions = new GridViewWidget(this);
    d->gridviewMeasureDefinitions->tableView()->verticalHeader()->setDefaultSectionSize(20);
    d->gridviewMeasureDefinitions->tableView()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->gridLayoutMeasureDefinitions->addWidget(d->gridviewMeasureDefinitions);
    // Create measure definitions table models
    createMeasureDefinitionsTableModels();
    // First tab is active
    ui->tabWidget->setCurrentIndex(0);
}

GlobalProsodicProfileWidget::~GlobalProsodicProfileWidget()
{
    delete ui;
    delete d;
}

QStringList GlobalProsodicProfileWidget::measureIDs()
{
    return QStringList()
            << "SpeechTime" << "propphonation" << "proppause" << "SpeechRate" << "nPVI"
            << "f0_qu02" << "f0_qu05" << "f0_qu50" << "f0_qu95" << "f0_qu98"
            << "sp_qu02" << "sp_qu05" << "sp_qu50" << "sp_qu95" << "sp_qu98"
            << "PitchMeanOfST" << "PitchStdevOfST"
            << "PitchRange" << "PitchBottom" << "PitchMean" << "PitchMedian" << "PitchTop"
            << "Gliss" << "Rises" << "Falls"
            << "TrajIntra" << "TrajInter" << "TrajPhon"
            << "TrajIntraZ" << "TrajInterZ" << "TrajPhonZ";
}

// static
StatisticalMeasureDefinition GlobalProsodicProfileWidget::measureDefinition(const QString &measureID)
{
    if (measureID == "SpeechTime")      return StatisticalMeasureDefinition("SpeechTime", "Total speech time", "s", "Total speech time = internucleus time + intranucleus time + pause time");
    if (measureID == "propphonation")   return StatisticalMeasureDefinition("propphonation", "Proportion of phonation time", "%", "Proportion (%) of estimated phonation time (= internucleus time + intranucleus time) to speech time");
    if (measureID == "proppause")       return StatisticalMeasureDefinition("proppause", "Proportion of pause time", "%", "Proportion (%) of estimated pause time (= when internucleus time >= 0.3) to speech time");
    if (measureID == "SpeechRate")      return StatisticalMeasureDefinition("SpeechRate", "Speech Rate", "syll/s", "Estimated speech rate = number of nuclei / phonation time");
    if (measureID == "nPVI")            return StatisticalMeasureDefinition("nPVI", "nPVI(nucl_dur)", "", "nPVI (nucleus duration)");
    if (measureID == "f0_qu02")         return StatisticalMeasureDefinition("f0_qu02", "F0 quantile 2%", "Hz", "Quantile 2% of min and max F0 values of nuclei before stylisation");
    if (measureID == "f0_qu05")         return StatisticalMeasureDefinition("f0_qu05", "F0 quantile 5%", "Hz", "Quantile 5% of min and max F0 values of nuclei before stylisation");
    if (measureID == "f0_qu50")         return StatisticalMeasureDefinition("f0_qu50", "F0 quantile 50%", "Hz", "Quantile 50% of min and max F0 values of nuclei before stylisation");
    if (measureID == "f0_qu95")         return StatisticalMeasureDefinition("f0_qu95", "F0 quantile 95%", "Hz", "Quantile 95% of min and max F0 values of nuclei before stylisation");
    if (measureID == "f0_qu98")         return StatisticalMeasureDefinition("f0_qu98", "F0 quantile 98%", "Hz", "Quantile 98% of min and max F0 values of nuclei before stylisation");
    if (measureID == "sp_qu02")         return StatisticalMeasureDefinition("sp_qu02", "Pitch quantile 2%", "Hz", "Quantile 2% of low & high pitch values of nuclei after stylisation");
    if (measureID == "sp_qu05")         return StatisticalMeasureDefinition("sp_qu05", "Pitch quantile 5%", "Hz", "Quantile 5% of low & high pitch values of nuclei after stylisation");
    if (measureID == "sp_qu50")         return StatisticalMeasureDefinition("sp_qu50", "Pitch quantile 50%", "Hz", "Quantile 50% of low & high pitch values of nuclei after stylisation");
    if (measureID == "sp_qu95")         return StatisticalMeasureDefinition("sp_qu95", "Pitch quantile 95%", "Hz", "Quantile 95% of low & high pitch values of nuclei after stylisation");
    if (measureID == "sp_qu98")         return StatisticalMeasureDefinition("sp_qu98", "Pitch quantile 98%", "Hz", "Quantile 98% of low & high pitch values of nuclei after stylisation");
    if (measureID == "PitchMeanOfST")   return StatisticalMeasureDefinition("PitchMeanOfST", "Pitch Mean ST", "ST", "Mean of pitch values, where values are min and max pitch in ST for each syllable");
    if (measureID == "PitchStdevOfST")  return StatisticalMeasureDefinition("PitchStdevOfST", "Pitch Stdev ST", "ST", "Stdev of pitch values, where values are min and max pitch in ST for each syllable");
    if (measureID == "PitchRange")      return StatisticalMeasureDefinition("PitchRange", "Pitch Range", "ST", "Pitch range of speaker (in ST relative to 1Hz)");
    if (measureID == "PitchBottom")     return StatisticalMeasureDefinition("PitchBottom", "Pitch Bottom", "Hz", "Pitch range of speaker: bottom value");
    if (measureID == "PitchMean")       return StatisticalMeasureDefinition("PitchMean", "Pitch Mean", "Hz", "Pitch range of speaker: mean value");
    if (measureID == "PitchMedian")     return StatisticalMeasureDefinition("PitchMedian", "Pitch Median", "Hz", "Pitch range of speaker: median value");
    if (measureID == "PitchTop")        return StatisticalMeasureDefinition("PitchTop", "Pitch Top", "Hz", "Pitch range of speaker: top value");
    if (measureID == "Gliss")           return StatisticalMeasureDefinition("Gliss", "Glissando %", "%", "Proportion (%) of syllables with large pitch movement (abs(distance) >= 4ST)");
    if (measureID == "Rises")           return StatisticalMeasureDefinition("Rises", "Rises %", "%", "Proportion (%) of syllables with pitch rise (>= 4ST)");
    if (measureID == "Falls")           return StatisticalMeasureDefinition("Falls", "Falls %", "%", "Proportion (%) of syllables with pitch fall (<= -4ST)");
    if (measureID == "TrajIntra")       return StatisticalMeasureDefinition("TrajIntra", "Trajectory Intra", "ST/s", "Pitch trajectory (sum of absolute intervals) within syllabic nuclei, divided by duration (in ST/s)");
    if (measureID == "TrajInter")       return StatisticalMeasureDefinition("TrajInter", "Trajectory Inter", "ST/s", "Pitch trajectory (sum of absolute intervals) between syllabic nuclei (except pauses or speaker turns), divided by duration (in ST/s)");
    if (measureID == "TrajPhon")        return StatisticalMeasureDefinition("TrajPhon", "Trajectory Phonation", "ST/s", "Sum of TrajIntra and TrajInter, divided by phonation time (in ST/s)");
    if (measureID == "TrajIntraZ")      return StatisticalMeasureDefinition("TrajIntraZ", "Trajectory Intra Z-score", "sd/s", "As TrajIntra, but for pitch trajectory in standard deviation units on ST scale (z-score) (in sd/s)");
    if (measureID == "TrajInterZ")      return StatisticalMeasureDefinition("TrajInterZ", "Trajectory Inter Z-score", "sd/s", "As TrajInter, but for pitch trajectory in standard deviation units on ST scale (z-score) (in sd/s)");
    if (measureID == "TrajPhonZ")       return StatisticalMeasureDefinition("TrajPhonZ", "Trajectory Phonation Z-score", "sd/s", "As TrajPhon, but for pitch trajectory in standard deviation units on ST scale (z-score) (in sd/s)");
    return StatisticalMeasureDefinition(measureID, measureID, "");
}

void GlobalProsodicProfileWidget::createMeasureDefinitionsTableModels()
{
    // Measures for communications
    d->modelMeasureDefinitions = new QStandardItemModel(this);
    d->modelMeasureDefinitions->setHorizontalHeaderLabels(QStringList() <<
                                                          "Measure ID" << "Measure" << "Units" << "Description");
    foreach (QString measureID, measureIDs()) {
        QList<QStandardItem *> items;
        StatisticalMeasureDefinition def = measureDefinition(measureID);
        items << new QStandardItem(measureID);
        items << new QStandardItem(def.displayName());
        items << new QStandardItem(def.units());
        items << new QStandardItem(def.description());
        d->modelMeasureDefinitions->appendRow(items);
    }
    // Update table
    d->gridviewMeasureDefinitions->tableView()->setModel(d->modelMeasureDefinitions);
    d->gridviewMeasureDefinitions->tableView()->resizeColumnsToContents();
}

void GlobalProsodicProfileWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;

    QStandardItemModel *model = new QStandardItemModel(this);
    // Headers
    QStringList headers;
    headers << "CommunicationID" << "RecordingID" << "SpeakerID";
    for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
        if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
        MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
        headers << attr->ID();
    }
    for (int i = 0; i < ui->comboBoxMetadataSpk->count(); ++i) {
        if (!ui->comboBoxMetadataSpk->itemData(i).toBool()) continue;
        MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker).at(i);
        headers << attr->ID();
    }
    foreach (QString measureID, measureIDs()) {
        headers << measureDefinition(measureID).displayName();
    }
    model->setHorizontalHeaderLabels(headers);
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(corpus->communicationsCount());
    int i = 0;
    foreach (CorpusCommunication *com, corpus->communications()) {
        if (!com) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (!annot) continue;
                QStringList speakerIDs = d->repository->annotations()->getSpeakersInAnnotation(annot->ID());
                foreach (QString speakerID, speakerIDs) {
                    QPointer<CorpusSpeaker> spk = corpus->speaker(speakerID);
                    // Read prosogram global profile
                    QFileInfo info(rec->filePath());
                    QString prosoPath = info.absoluteDir().absolutePath() + "/prosogram/";
                    QString filenameGlobalsheet = QString("%1_%2_globalsheet.txt").arg(rec->ID()).arg(speakerID);
                    AnnotationDataTable table;
                    if (!(table.readFromFile(prosoPath + filenameGlobalsheet))) {
                        qDebug() << "Error reading: " << filenameGlobalsheet;
                        continue;
                    }
                    // Process data from table
                    for (int i = 0; i < table.getRowCount(); i++) {
                        QList<QStandardItem *> items;
                        items.append(new QStandardItem(com->ID()));
                        items.append(new QStandardItem(rec->ID()));
                        items.append(new QStandardItem(speakerID));
                        // selected metadata
                        // selected metadata attributes (Com, then Spk)
                        for (int i = 0; i < ui->comboBoxMetadataCom->count(); ++i) {
                            if (!ui->comboBoxMetadataCom->itemData(i).toBool()) continue;
                            MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Communication).at(i);
                            QStandardItem *item = new QStandardItem();
                            item->setData(com->property(attr->ID()), Qt::DisplayRole); items << item;
                        }
                        for (int i = 0; i < ui->comboBoxMetadataSpk->count(); ++i) {
                            if (!ui->comboBoxMetadataSpk->itemData(i).toBool()) continue;
                            MetadataStructureAttribute *attr = d->repository->metadataStructure()->attributes(CorpusObject::Type_Speaker).at(i);
                            QStandardItem *item = new QStandardItem();
                            item->setData(spk->property(attr->ID()), Qt::DisplayRole);
                            items << item;
                        }
                        // measures
                        foreach (QString attributeName, measureIDs()) {
                            QVariant value = table.getData(i, attributeName);
                            QStandardItem *item = new QStandardItem();
                            item->setData(value, Qt::DisplayRole);
                            items.append(item);
                        }
                        model->appendRow(items);
                    }
                } // speaker
            } // annotation
        } // recording
        i++;
        ui->progressBar->setValue(i);
        QApplication::processEvents();
    } // commmunication
    ui->progressBar->setValue(ui->progressBar->maximum());

    // Update table
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
    d->modelResults = model;
}

} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline
