#include <QDebug>
#include <QApplication>
#include <QProgressBar>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QStandardItemModel>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/statistics/StatisticalMeasureDefinition.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"
#include "AnalyserMacroprosodyWidget.h"
#include "ui_AnalyserMacroprosodyWidget.h"

#include "AnalyserMacroprosody.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginProsody {

struct AnalyserMacroprosodyWidgetData {
    AnalyserMacroprosodyWidgetData() :
        repository(0), gridviewResults(0), modelResults(0)
    {}

    CorpusRepository *repository;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
    GridViewWidget *gridviewMeasureDefinitions;
    QStandardItemModel *modelMeasureDefinitions;
};

class AnalyserMacroprosodyUnit {
public:
    QString communicationID;
    QString annotationID;
    QString speakerID;
    RealTime tMin;
    RealTime tMax;
    QString label;
    QString key() { return QString("%1\t%2\t%3").arg(communicationID).arg(annotationID).arg(speakerID); }
};

AnalyserMacroprosodyWidget::AnalyserMacroprosodyWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::AnalyserMacroprosodyWidget), d(new AnalyserMacroprosodyWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // Corpora combobox
    ui->comboBoxCorpus->addItems(repository->listCorporaIDs());
    // Levels from annotation structure
    ui->comboBoxMUTier->addItems(repository->annotationStructure()->levelIDs());
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Command Open File for MU list
    connect(ui->commandOpenFileMUList, SIGNAL(clicked(bool)), this, SLOT(openFileMUList()));
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
    // Default options
    ui->optionMULIstFromTier->setChecked(true);
    ui->tabWidget->setCurrentIndex(0);
}

AnalyserMacroprosodyWidget::~AnalyserMacroprosodyWidget()
{
    delete ui;
    delete d;
}

void AnalyserMacroprosodyWidget::createMeasureDefinitionsTableModels()
{
    // Measures for communications
    d->modelMeasureDefinitions = new QStandardItemModel(this);
    d->modelMeasureDefinitions->setHorizontalHeaderLabels(QStringList() <<
                                                          "Measure ID" << "Measure" << "Units" << "Description");
    foreach (QString measureID, AnalyserMacroprosody::measureIDs("")) {
        QList<QStandardItem *> items;
        StatisticalMeasureDefinition def = AnalyserMacroprosody::measureDefinition("", measureID);
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

void AnalyserMacroprosodyWidget::openFileMUList()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Select File for Macro-Unit List"), "",
                                                    tr("Text File (*.txt);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    ui->editFilenameMUList->setText(filename);
    ui->optionMUListFromFile->setChecked(true);
}

void AnalyserMacroprosodyWidget::analyse()
{
    if (!d->repository) return;
    QString corpusID = ui->comboBoxCorpus->currentText();
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;

    // New analyser and model
    QScopedPointer<AnalyserMacroprosody> analyser(new AnalyserMacroprosody);
    QStandardItemModel *model = new QStandardItemModel(this);

    // Collect units
    QMap<QString, QList<AnalyserMacroprosodyUnit> > unitGroups;
    if (ui->optionMULIstFromTier->isChecked()) {
        QString tierNameMacroUnits = ui->comboBoxMUTier->currentText();
        foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
            if (!com) continue;
            foreach (QString annotationID, com->annotationIDs()) {
                QMap<QString, QPointer<AnnotationTierGroup> > tiersAll =
                        d->repository->annotations()->getTiersAllSpeakers(annotationID, QStringList() << tierNameMacroUnits);
                foreach (QString speakerID, tiersAll.keys()) {
                    QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                    if (!tiers) continue;
                    IntervalTier *tier_macroUnit = tiers->getIntervalTierByName(tierNameMacroUnits);
                    QList<AnalyserMacroprosodyUnit> units;
                    foreach (Interval *intv, tier_macroUnit->intervals()) {
                        if (intv->isPauseSilent() && ui->checkBoxExcludePauses->isChecked()) continue;
                        AnalyserMacroprosodyUnit unit;
                        unit.communicationID = com->ID();
                        unit.annotationID = annotationID;
                        unit.speakerID = speakerID;
                        unit.tMin = intv->tMin();
                        unit.tMax = intv->tMax();
                        unit.label = intv->text();
                        units << unit;
                    }
                    if (!units.empty()) {
                        unitGroups.insert(units.first().key(), units);
                    }
                }
                qDeleteAll(tiersAll);
            }
        }
    } else {
        // File format to use:
        // CommunicationID | AnnotationID | SpeakerID | tMin | tMax | Label
        // =============================================================================
        QFile file(ui->editFilenameMUList->text());
        if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return;
        QTextStream stream(&file);
        do {
            QString line = stream.readLine().trimmed();
            if (line.startsWith("#")) continue; // # for comments
            AnalyserMacroprosodyUnit unit;
            unit.communicationID = line.section("\t", 0, 0).trimmed();
            unit.annotationID = line.section("\t", 1, 1).trimmed();
            unit.speakerID = line.section("\t", 2, 2).trimmed();
            unit.tMin = RealTime::fromSeconds(line.section("\t", 3, 3).toDouble());
            unit.tMax = RealTime::fromSeconds(line.section("\t", 4, 4).toDouble());
            unit.label = line.section("\t", 5).trimmed();
            if (unitGroups.contains(unit.key())) {
                unitGroups[unit.key()].append(unit);
            } else {
                QList<AnalyserMacroprosodyUnit> list; list << unit;
                unitGroups.insert(unit.key(), list);
            }
        } while (!stream.atEnd());
        file.close();
    }
    // Analyse units
    ui->progressBar->setMaximum(unitGroups.count());
    int progressCount(0);
    foreach (QString groupKey, unitGroups.keys()) {
        QList<AnalyserMacroprosodyUnit> units = unitGroups.value(groupKey);
        QList<Interval *> intervals;
        foreach (AnalyserMacroprosodyUnit unit, units) {
            intervals << new Interval(unit.tMin, unit.tMax, unit.label);
        }
        if (!intervals.isEmpty()) {
            // Run analyser for each macro-unit and take each row of results into the model
            analyser->calculate(corpus, units.first().communicationID, units.first().annotationID, units.first().speakerID, intervals);
            while (analyser->model()->rowCount() > 0) {
                model->appendRow(analyser->model()->takeRow(0));
            }
            qDeleteAll(intervals);
            intervals.clear();
        }
        progressCount++;
        ui->progressBar->setValue(progressCount);
        QApplication::processEvents();
    }
    // Update table headers
    if (analyser && analyser->model()) {
        for (int i = 0; i < analyser->model()->columnCount(); ++i)
            model->setHorizontalHeaderItem(i, new QStandardItem(analyser->model()->horizontalHeaderItem(i)->text()));
    }
    // Update table
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) { d->modelResults->clear(); delete d->modelResults; }
    d->modelResults = model;
    ui->progressBar->setValue(ui->progressBar->maximum());
}

} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline
