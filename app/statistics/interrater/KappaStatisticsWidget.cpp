#include "KappaStatisticsWidget.h"
#include "ui_KappaStatisticsWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusSpeaker.h"
#include "pncore/structure/MetadataStructure.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

#include "pngui/widgets/GridViewWidget.h"

#include "KappaStatisticsCalculator.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginInterrater {

struct KappaStatisticsWidgetData {
    KappaStatisticsWidgetData() :
        repository(0), gridviewResults(0), modelResults(0)
    {}
    CorpusRepository *repository;
    KappaStatisticsCalculator calculator;
    GridViewWidget *gridviewResults;
    QStandardItemModel *modelResults;
};

KappaStatisticsWidget::KappaStatisticsWidget(CorpusRepository *repository, QWidget *parent) :
    QWidget(parent), ui(new Ui::KappaStatisticsWidget), d(new KappaStatisticsWidgetData)
{
    ui->setupUi(this);
    if (!repository) return;
    d->repository = repository;
    // ================================================================================================================
    // MAIN WIDGET AND RESUTLS TAB
    // ================================================================================================================
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
    // Levels and attributes
    QStringList levelIDs = repository->annotationStructure()->levelIDs();
    ui->comboBoxAnnotationLevel->addItems(levelIDs);
    connect(ui->comboBoxAnnotationLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(annotationLevelChanged(QString)));
    annotationLevelChanged("");
    // Command Analyse
    connect(ui->commandAnalyse, SIGNAL(clicked(bool)), this, SLOT(analyse()));
    // Results grid view
    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);
    // Default
    ui->editCohenNameCat1->setText("Category 1");
    ui->editCohenNameCat2->setText("Category 2");
}

KappaStatisticsWidget::~KappaStatisticsWidget()
{
    delete ui;
}

void KappaStatisticsWidget::annotationLevelChanged(const QString &text)
{
    Q_UNUSED(text)
    if (!d->repository) return;
    QString levelID = ui->comboBoxAnnotationLevel->currentText();
    AnnotationStructureLevel *level = d->repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAnnotationAttributes->clear();
    ui->comboBoxAnnotationAttributes->insertItem(0, tr("(text)"), false);
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAnnotationAttributes->addItem(attribute->ID(), false);
    }
}

void KappaStatisticsWidget::defineClassesForCohenKappa()
{
    d->calculator.reset();
    QString separator = ui->editCohenSeparator->text().trimmed();
    if (separator.isEmpty()) separator = " ";
    QStringList labelsClass1, labelsClass2, labelsIgnore;
    foreach (QString label, ui->editCohenLabelsCat1->text().split(separator)) {
        labelsClass1 << label.trimmed();
    }
    foreach (QString label, ui->editCohenLabelsCat2->text().split(separator)) {
        labelsClass2 << label.trimmed();
    }
    foreach (QString label, ui->editCohenLabelsIgnore->text().split(separator)) {
        labelsIgnore << label.trimmed();
    }
    if (ui->editCohenNameCat1->text().isEmpty()) ui->editCohenNameCat1->setText(tr("Class 1"));
    if (ui->editCohenNameCat2->text().isEmpty()) ui->editCohenNameCat2->setText(tr("Class 2"));
    d->calculator.addClass(ui->editCohenNameCat1->text(), labelsClass1, ui->checkBoxCohenCat1Empty->isChecked());
    d->calculator.addClass(ui->editCohenNameCat2->text(), labelsClass2, ui->checkBoxCohenCat2Empty->isChecked());
    d->calculator.addClass("", labelsIgnore, ui->checkBoxCohenIgnoreEmpty->isChecked());
}

void KappaStatisticsWidget::analyse()
{
    if (ui->radioButtonCalculateCohen->isChecked()) {
        analyseCohenKappa();
    }
}

void KappaStatisticsWidget::analyseCohenKappa()
{
    if (!d->repository) return;
    // Get corpus
    QString corpusID = ui->comboBoxCorpus->currentText();
    QPointer<Corpus> corpus = d->repository->metadata()->getCorpus(corpusID);
    if (!corpus) return;
    // Level ID to analyse
    QString levelID = ui->comboBoxAnnotationLevel->currentText();
    // Set progress bar
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(corpus->communicationsCount());
    // Prepare Cohen Kappa calculator
    defineClassesForCohenKappa();
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
//        if (!com) continue;
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            QString annotationID = annot->ID();
//            tiersAll = d->repository->annotations()->getTiersAllSpeakers(annotationID, QStringList() << levelID);
//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;
//                IntervalTier *tier = tiers->getIntervalTierByName(levelID);
//                d->calculator.getCohenKappa()
//                foreach (Interval * syll, syllables) {
//                    if (syll->attribute("promise_pos").toString().isEmpty()) syll->setAttribute("promise_pos", "0");
//                    if (syll->attribute("delivery2").toString().isEmpty()) syll->setAttribute("delivery2", "0");
//                }
//                double k = agreement.getCohenKappa("0", "P", syllables, "promise_pos", "delivery2");
//                printMessage(QString("%1\t%2").arg(com->ID()).arg(k));
//            }
//            qDeleteAll(tiersAll);
//        }
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
    }

    // Finished
    ui->progressBar->setValue(ui->progressBar->maximum());
}

} // namespace StatisticsPluginInterrater
} // namespace Plugins
} // namespace Praaline
