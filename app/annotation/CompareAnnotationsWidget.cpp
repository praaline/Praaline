#include <QString>
#include <QList>
#include <QPointer>
#include <QSet>
#include <QMap>
#include <QAction>
#include <QMessageBox>

#include "CompareAnnotationsWidget.h"
#include "ui_CompareAnnotationsWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/MetadataDatastore.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/diff/DiffIntervals.h"
using namespace Praaline::Core;

#include "pngui/model/diff/DiffSESForIntervalsTableModel.h"
#include "pngui/widgets/CheckBoxList.h"
#include "pngui/widgets/GridViewWidget.h"

#include "pngui/xlsx/xlsxdocument.h"
#include "pngui/xlsx/xlsxformat.h"

#include "pngui/PraalineUserInterfaceOptions.h"
#include "CorpusRepositoriesManager.h"

struct CompareAnnotationsWidgetData {
    CompareAnnotationsWidgetData()
        : modelResults(0), outputPath("/home/george/comparator/")
    {}

    QToolBar *toolbarMain;
    QAction *actionCompare;
    QAction *actionExport;

    // Results grid
    GridViewWidget *gridviewResults;
    dtl::Ses<Interval *>::sesElemVec sesSequence;
    DiffSESforIntervalsTableModel *modelResults;

    // Corpus Manager
    CorpusRepositoriesManager *corpusRepositoriesManager;
    // Path for output files
    QString outputPath;
};

CompareAnnotationsWidget::CompareAnnotationsWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CompareAnnotationsWidget), d(new CompareAnnotationsWidgetData)
{
    ui->setupUi(this);

    // Setup user interfacce
    d->toolbarMain = new QToolBar(tr("Compare Annotations"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    this->addToolBar(d->toolbarMain);

    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    ui->gridLayoutResults->addWidget(d->gridviewResults);

    setupActions();

    // Corpus Repositories Manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    // Repository
    connect(d->corpusRepositoriesManager, SIGNAL(corpusRepositoryAdded(QString)), this, SLOT(corpusRepositoryAdded(QString)));
    connect(d->corpusRepositoriesManager, SIGNAL(corpusRepositoryRemoved(QString)), this, SLOT(corpusRepositoryRemoved(QString)));
    // Compare Corpora
    connect(ui->comboBoxCCRepositoryLeft, SIGNAL(currentTextChanged(QString)), this, SLOT(ccRepositoryLeftChanged(QString)));
    connect(ui->comboBoxCCRepositoryRight, SIGNAL(currentTextChanged(QString)), this, SLOT(ccRepositoryRightChanged(QString)));
    // Compare Annotations
    connect(ui->comboBoxCARepositoryLeft, SIGNAL(currentTextChanged(QString)), this, SLOT(caRepositoryLeftChanged(QString)));
    connect(ui->comboBoxCARepositoryRight, SIGNAL(currentTextChanged(QString)), this, SLOT(caRepositoryRightChanged(QString)));
    connect(ui->comboBoxLevelLeft, SIGNAL(currentTextChanged(QString)), this, SLOT(caLevelLeftChanged(QString)));
    connect(ui->comboBoxLevelRight, SIGNAL(currentTextChanged(QString)), this, SLOT(caLevelRightChanged(QString)));
    connect(ui->comboBoxAnnotationLeft, SIGNAL(currentTextChanged(QString)), this, SLOT(caAnnotationLeftChanged(QString)));
    connect(ui->comboBoxAnnotationRight, SIGNAL(currentTextChanged(QString)), this, SLOT(caAnnotationRightChanged(QString)));
    // Compare commands
    connect(ui->commandCompareCorpora, SIGNAL(clicked()), this, SLOT(compareCorpora()));
    connect(ui->commandCompareAnnotations, SIGNAL(clicked()), this, SLOT(compareAnnotations()));
}

CompareAnnotationsWidget::~CompareAnnotationsWidget()
{
    delete ui;
    delete d;
}

void CompareAnnotationsWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    d->actionExport = new QAction(QIcon(":/icons/actions/action_export.png"), tr("Export"), this);
    connect(d->actionExport, SIGNAL(triggered()), SLOT(exportResults()));
    command = ACTION_MANAGER->registerAction("Annotation.CompareAnnotations.Export", d->actionExport, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionExport);

}

void CompareAnnotationsWidget::corpusRepositoryAdded(const QString &repositoryID)
{
    if (!d->corpusRepositoriesManager) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    ui->comboBoxCCRepositoryLeft->addItem(repository->ID(), repositoryID);
    ui->comboBoxCCRepositoryRight->addItem(repository->ID(), repositoryID);
    ui->comboBoxCARepositoryLeft->addItem(repository->ID(), repositoryID);
    ui->comboBoxCARepositoryRight->addItem(repository->ID(), repositoryID);
}

void CompareAnnotationsWidget::corpusRepositoryRemoved(const QString &repositoryID)
{
    if (!d->corpusRepositoriesManager) return;
    int index = ui->comboBoxCCRepositoryLeft->findData(repositoryID);
    if (index >= 0) {
        if (index == ui->comboBoxCCRepositoryLeft->currentIndex()) ui->comboBoxCCCorpusLeft->clear();
        ui->comboBoxCCRepositoryLeft->removeItem(index);
    }
    index = ui->comboBoxCCRepositoryRight->findData(repositoryID);
    if (index >= 0) {
        if (index == ui->comboBoxCCRepositoryRight->currentIndex()) ui->comboBoxCCCorpusRight->clear();
        ui->comboBoxCCRepositoryRight->removeItem(index);
    }
    index = ui->comboBoxCARepositoryLeft->findData(repositoryID);
    if (index >= 0) {
        if (index == ui->comboBoxCARepositoryLeft->currentIndex()) ui->comboBoxCACorpusLeft->clear();
        ui->comboBoxCARepositoryLeft->removeItem(index);
    }
    index = ui->comboBoxCARepositoryRight->findData(repositoryID);
    if (index >= 0) {
        if (index == ui->comboBoxCARepositoryRight->currentIndex()) ui->comboBoxCACorpusRight->clear();
        ui->comboBoxCARepositoryRight->removeItem(index);
    }
}

void CompareAnnotationsWidget::ccRepositoryLeftChanged(const QString &repositoryID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    ui->comboBoxCCCorpusLeft->clear();
    ui->comboBoxCCCorpusLeft->addItems(d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID));
}

void CompareAnnotationsWidget::ccRepositoryRightChanged(const QString &repositoryID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    ui->comboBoxCCCorpusRight->clear();
    ui->comboBoxCCCorpusRight->addItems(d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID));
}

void CompareAnnotationsWidget::caRepositoryLeftChanged(const QString &repositoryID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    ui->comboBoxCACorpusLeft->clear();
    ui->comboBoxCACorpusLeft->addItems(d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID));
    ui->comboBoxLevelLeft->clear();
    ui->comboBoxAttributeLeft->clear();
    foreach (AnnotationStructureLevel *level, repository->annotationStructure()->levels()) {
        ui->comboBoxLevelLeft->addItem(level->name(), level->ID());
    }
    caCorpusLeftChanged(ui->comboBoxCACorpusLeft->currentText());
}

void CompareAnnotationsWidget::caRepositoryRightChanged(const QString &repositoryID)
{
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    ui->comboBoxCACorpusRight->clear();
    ui->comboBoxCACorpusRight->addItems(d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID));
    ui->comboBoxLevelRight->clear();
    ui->comboBoxAttributeRight->clear();
    foreach (AnnotationStructureLevel *level, repository->annotationStructure()->levels()) {
        ui->comboBoxLevelRight->addItem(level->name(), level->ID());
    }
    caCorpusRightChanged(ui->comboBoxCACorpusRight->currentText());
}

void CompareAnnotationsWidget::caCorpusLeftChanged(const QString &corpusID)
{
    ui->comboBoxAnnotationLeft->clear();
    ui->comboBoxSpeakerLeft->clear();
    QPointer<Corpus> corpus = d->corpusRepositoriesManager->openCorpus(corpusID, ui->comboBoxCARepositoryLeft->currentText());
    if (!corpus) return;
    QPair<QString, QString> pair;
    foreach (pair, corpus->getCommunicationsAnnotationsIDs()) {
        ui->comboBoxAnnotationLeft->addItem(pair.first + " :: " + pair.second, pair.second);
    }
}

void CompareAnnotationsWidget::caCorpusRightChanged(const QString &corpusID)
{
    ui->comboBoxAnnotationRight->clear();
    ui->comboBoxSpeakerRight->clear();
    QPointer<Corpus> corpus = d->corpusRepositoriesManager->openCorpus(corpusID, ui->comboBoxCARepositoryRight->currentText());
    if (!corpus) return;
    QPair<QString, QString> pair;
    foreach (pair, corpus->getCommunicationsAnnotationsIDs()) {
        ui->comboBoxAnnotationRight->addItem(pair.first + " :: " + pair.second, pair.second);
    }
}

void CompareAnnotationsWidget::caLevelLeftChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(ui->comboBoxCARepositoryLeft->currentText());
    if (!repository) return;
    QString levelID = ui->comboBoxLevelLeft->currentData().toString();
    AnnotationStructureLevel *level = repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAttributeLeft->clear();
    ui->comboBoxAttributeLeft->addItem("", "");
    int i = 0;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAttributeLeft->addItem(attribute->name(), attribute->ID());
        ui->comboBoxExtraAttributesLeft->insertItem(i, attribute->name(), false);
        i++;
    }
}

void CompareAnnotationsWidget::caLevelRightChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(ui->comboBoxCARepositoryRight->currentText());
    if (!repository) return;
    QString levelID = ui->comboBoxLevelRight->currentData().toString();
    AnnotationStructureLevel *level = repository->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxAttributeRight->clear();
    ui->comboBoxAttributeRight->addItem("", "");
    int i = 0;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxAttributeRight->addItem(attribute->name(), attribute->ID());
        ui->comboBoxExtraAttributesRight->insertItem(i, attribute->name(), false);
        i++;
    }
}

void CompareAnnotationsWidget::caAnnotationLeftChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(ui->comboBoxCARepositoryLeft->currentText());
    if (!repository) return;
    QString annotationID = ui->comboBoxAnnotationLeft->currentData().toString();
    ui->comboBoxSpeakerLeft->clear();
    ui->comboBoxSpeakerLeft->addItems(repository->annotations()->getSpeakersInAnnotation(annotationID));
}

void CompareAnnotationsWidget::caAnnotationRightChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(ui->comboBoxCARepositoryRight->currentText());
    if (!repository) return;
    QString annotationID = ui->comboBoxAnnotationRight->currentData().toString();
    ui->comboBoxSpeakerRight->clear();
    ui->comboBoxSpeakerRight->addItems(repository->annotations()->getSpeakersInAnnotation(annotationID));
}

void CompareAnnotationsWidget::compareAnnotations()
{
    QPointer<CorpusRepository> repository_left = d->corpusRepositoriesManager->corpusRepositoryByID(ui->comboBoxCARepositoryLeft->currentText());
    if (!repository_left) return;
    QPointer<CorpusRepository> repository_right = d->corpusRepositoriesManager->corpusRepositoryByID(ui->comboBoxCARepositoryRight->currentText());
    if (!repository_right) return;

    QString annotationID_left, speakerID_left, levelID_left, attributeID_left;
    QString annotationID_right, speakerID_right, levelID_right, attributeID_right;
    annotationID_left = ui->comboBoxAnnotationLeft->currentData().toString();
    speakerID_left = ui->comboBoxSpeakerLeft->currentText();
    levelID_left = ui->comboBoxLevelLeft->currentData().toString();
    attributeID_left = ui->comboBoxAttributeLeft->currentData().toString();
    annotationID_right = ui->comboBoxAnnotationRight->currentData().toString();
    speakerID_right = ui->comboBoxSpeakerRight->currentText();
    levelID_right = ui->comboBoxLevelRight->currentData().toString();
    attributeID_right = ui->comboBoxAttributeRight->currentData().toString();

    AnnotationTierGroup *group_left = repository_left->annotations()->getTiers(
                annotationID_left, speakerID_left, QStringList() << levelID_left);
    IntervalTier *tier_left = group_left->getIntervalTierByName(levelID_left);
    if (!tier_left) {
        QMessageBox::warning(this, tr("Cannot find left data"),
                             QString(tr("No data for left pane: annotation %1, speaker %2 and annotation level %3."))
                             .arg(annotationID_left).arg(speakerID_left).arg(levelID_left), QMessageBox::Ok);
        return;
    }
    AnnotationTierGroup *group_right = repository_right->annotations()->getTiers(
                annotationID_right, speakerID_right, QStringList() << levelID_right);
    IntervalTier *tier_right = group_right->getIntervalTierByName(levelID_right);
    if (!tier_right) {
        QMessageBox::warning(this, tr("Cannot find right data"),
                             QString(tr("No data for right pane: annotation %1, speaker %2 and annotation level %3."))
                             .arg(annotationID_right).arg(speakerID_right).arg(levelID_right), QMessageBox::Ok);
        return;
    }

    // Extra attributes
    QStringList extraAttributes_left, extraAttributes_right;
    for (int i = 0; i < ui->comboBoxExtraAttributesLeft->count(); ++i) {
        if (ui->comboBoxExtraAttributesLeft->itemData(i).toBool() == true) {
            AnnotationStructureLevel *level = repository_left->annotationStructure()->level(levelID_left);
            if (level && i < level->attributesCount()) extraAttributes_left << level->attributes().at(i)->ID();
        }
    }
    for (int i = 0; i < ui->comboBoxExtraAttributesRight->count(); ++i) {
        if (ui->comboBoxExtraAttributesRight->itemData(i).toBool() == true) {
            AnnotationStructureLevel *level = repository_right->annotationStructure()->level(levelID_right);
            if (level && i < level->attributesCount()) extraAttributes_right << level->attributes().at(i)->ID();
        }
    }

    d->sesSequence = DiffIntervals::intervalDiff(tier_left->intervals(), tier_right->intervals(), false,
                                                 attributeID_left, attributeID_right).getSes().getSequence();

    DiffSESforIntervalsTableModel *model = new DiffSESforIntervalsTableModel(
                d->sesSequence, tier_left->intervals(), tier_right->intervals(),
                attributeID_left, attributeID_right, extraAttributes_left, extraAttributes_right, this);
    d->gridviewResults->tableView()->setModel(model);
    if (d->modelResults) delete d->modelResults;
    d->modelResults = model;
}

void CompareAnnotationsWidget::exportResults()
{
    if (!d->modelResults) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Results"),
                                                    tr("praaline_compare.txt"), tr("Tab-separated Text File (*.txt);;All Files (*)"),
                                                    &selectedFilter, options);
    if (fileName.isEmpty()) return;
    d->gridviewResults->exportToTabSeparated(fileName);
}

void CompareAnnotationsWidget::saveChanges()
{

}

void CompareAnnotationsWidget::compareCorpora()
{
    // Open repositories
    QString repositoryID_left = ui->comboBoxCCRepositoryLeft->currentText();
    QString repositoryID_right = ui->comboBoxCCRepositoryRight->currentText();
    QPointer<CorpusRepository> repository_left = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID_left);
    if (!repository_left) return;
    QPointer<CorpusRepository> repository_right = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID_right);
    if (!repository_right) return;
    // Open corpora
    QString corpusID_left = ui->comboBoxCCCorpusLeft->currentText();
    QString corpusID_right = ui->comboBoxCCCorpusRight->currentText();
    QPointer<Corpus> corpus_left = repository_left->metadata()->getCorpus(corpusID_left);
    if (!corpus_left) return;
    QPointer<Corpus> corpus_right = repository_right->metadata()->getCorpus(corpusID_right);
    if (!corpus_right) return;

    // Find common levels
    QList<QString> levelIDs_common;
    levelIDs_common << QSet<QString>::fromList(repository_left->annotationStructure()->levelIDs())
                       .intersect(QSet<QString>::fromList(repository_right->annotationStructure()->levelIDs())).toList();
    qSort(levelIDs_common);

    // Selected levels
    QStringList levelIDs; levelIDs << "phone" << "syll" << "tok_min" << "tok_mwu" << "sequence" << "rection" << "bdu" << "boundary";

    // Labels header
    QString header = "CommunicationID\tAnnotationID\tSpeakerID\t";
    foreach (QString levelID, levelIDs) header = header.append(levelID).append("\t");
    if (!header.isEmpty()) header.chop(1);
    ui->editCompareCorporaMessages->appendPlainText(header);

    // Loop through communicationIDs existing in both corpora
    QList<QString> communicationIDs_common = QSet<QString>::fromList(corpus_left->communicationIDs()).intersect(
                QSet<QString>::fromList(corpus_right->communicationIDs())).toList();
    qSort(communicationIDs_common);
    foreach (QString communicationID, communicationIDs_common) {
        CorpusCommunication *com_left = corpus_left->communication(communicationID);
        if (!com_left) continue;
        CorpusCommunication *com_right = corpus_right->communication(communicationID);
        if (!com_right) continue;
        // Loop through annotationIDs existing in both communications
        QList<QString> annotationIDs_common = QSet<QString>::fromList(com_left->annotationIDs()).intersect(
                    QSet<QString>::fromList(com_right->annotationIDs())).toList();
        qSort(annotationIDs_common);
        foreach (QString annotationID, annotationIDs_common) {
            SpeakerAnnotationTierGroupMap tiersAll_left, tiersAll_right;
            tiersAll_left = repository_left->annotations()->getTiersAllSpeakers(annotationID);
            tiersAll_right = repository_right->annotations()->getTiersAllSpeakers(annotationID);
            // Find common speakerIDs
            QList<QString> speakerIDs_common = QSet<QString>::fromList(tiersAll_left.keys()).intersect(
                        QSet<QString>::fromList(tiersAll_right.keys())).toList();
            qSort(speakerIDs_common);
            foreach (QString speakerID, speakerIDs_common) {
                QMap<QString, QPointer<DiffSESforIntervalsTableModel> > models;
                QMap<QString, dtl::Ses<Interval *>::sesElemVec> sesSequences;
                foreach (QString levelID, levelIDs_common) {
                    // Load annotations
                    IntervalTier *tier_left = tiersAll_left.value(speakerID)->getIntervalTierByName(levelID);
                    if (!tier_left) continue;
                    IntervalTier *tier_right = tiersAll_right.value(speakerID)->getIntervalTierByName(levelID);
                    if (!tier_right) continue;
                    // Apply diff
                    sesSequences.insert(levelID, DiffIntervals::intervalDiff(tier_left->intervals(), tier_right->intervals(), false).getSes().getSequence());
                    // Get model of diff sequence and add it
                    models.insert(levelID, new DiffSESforIntervalsTableModel(
                                      sesSequences[levelID], tier_left->intervals(), tier_right->intervals(),
                                      "", "", QStringList(), QStringList(), this));
                }
                // Export diff tables
                exportDiffTableCombinedExcel(communicationID, annotationID, speakerID, models, levelIDs);
                qDeleteAll(models);
                sesSequences.clear();
            } // speaker ID
            qDeleteAll(tiersAll_left);
            qDeleteAll(tiersAll_right);
        } // annotationID
    } // communicationID

}

void CompareAnnotationsWidget::exportDiffTableByLevelExcel(const QString &communicationID, const QString &annotationID, const QString &speakerID,
                                                           QMap<QString, QPointer<DiffSESforIntervalsTableModel> > &models,
                                                           QStringList levelIDs)
{
    if (levelIDs.isEmpty()) levelIDs = models.keys();
    // Create an Excel document
    QXlsx::Document xlsx;
    // Note: rows and columns start from 1 for QXlsx
    int row(1), col(1);
    // Format for the headers
    QXlsx::Format format_header;
    format_header.setFontBold(true);
    format_header.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    // Format: OP tMinA tMaxA tMinB tMaxB A B
    foreach (QString levelID, levelIDs) {
        QPointer<DiffSESforIntervalsTableModel> model = models.value(levelID);
        if (!model) continue;
        int countAdditions(0), countDeletions(0);
        // Header
        for (int j = 0; j < model->columnCount(); ++j) {
            xlsx.write(1, col + j, levelID, format_header);
            xlsx.write(2, col + j, model->headerData(j, Qt::Horizontal, Qt::DisplayRole), format_header);
        }
        row = 3;
        for (int i = 0; i < model->rowCount(); ++i) {
            QString opcode = model->data(model->index(i, 0), Qt::DisplayRole).toString();
            if (opcode == "+") countAdditions++;
            if (opcode == "-") countDeletions++;
            for (int j = 0; j < model->columnCount(); ++j) {
                if (j == 0)
                    xlsx.write(row + i, col + j, model->data(model->index(i, j), Qt::DisplayRole).toString().replace("=", ""));
                else
                    xlsx.write(row + i, col + j, model->data(model->index(i, j), Qt::DisplayRole));
            }
        }
        col = col + model->columnCount();
        // Counts
        QString message = QString("%1\t%2\t%3\t").arg(communicationID).arg(annotationID).arg(speakerID);
        message = message.append(QString("%1\t%2\t%3\t%4").arg(levelID).arg(model->rowCount()).arg(countAdditions).arg(countDeletions));
        ui->editCompareCorporaMessages->appendPlainText(message);
        QApplication::processEvents();
    }
    QString filename = QString("%1_%2.xlsx").arg(communicationID).arg(speakerID);
    xlsx.saveAs(d->outputPath + filename);
}

struct CombinedTimelineData {
    RealTime t;
    QString levelID;
    int rowIndex;
};

void CompareAnnotationsWidget::exportDiffTableCombinedExcel(const QString &communicationID, const QString &annotationID, const QString &speakerID,
                                                            QMap<QString, QPointer<DiffSESforIntervalsTableModel> > &models,
                                                            QStringList levelIDs)
{
    if (levelIDs.isEmpty()) levelIDs = models.keys();
    // Create an Excel document
    QXlsx::Document xlsx;
    // Note: rows and columns start from 1 for QXlsx
    int row(1), col(1);
    // Format for the headers
    QXlsx::Format format_header, format_addition, format_deletion;
    format_header.setFontBold(true);
    format_header.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format_addition.setFontBold(true); format_deletion.setFontBold(true);
    format_addition.setPatternBackgroundColor(QColor(204, 255, 204));
    format_deletion.setPatternBackgroundColor(QColor(255, 204, 255));
    // Format: OP tMinA tMaxA tMinB tMaxB A B
    QHash<QString, int> levelColumn;
    QMultiMap<RealTime, CombinedTimelineData> timeline;
    QHash<QString, int> countDifferences;
    bool insideDifferences(false);
    foreach (QString levelID, levelIDs) {
        // Get SES sequence table model for this level
        QPointer<DiffSESforIntervalsTableModel> model = models.value(levelID);
        if (!model) continue;
        levelColumn.insert(levelID, col);
        countDifferences.insert(levelID, 0);
        // Header
        for (int j = 0; j < model->columnCount(); ++j) {
            xlsx.write(1, (col - 1) * model->columnCount() + j + 1, levelID, format_header);
            xlsx.write(2, (col - 1) * model->columnCount() + j + 1, model->headerData(j, Qt::Horizontal, Qt::DisplayRole), format_header);
        }
        for (int i = 0; i < model->rowCount(); ++i) {
            RealTime t_A = RealTime::fromSeconds(model->data(model->index(i, 1), Qt::DisplayRole).toDouble());
            RealTime t_B = RealTime::fromSeconds(model->data(model->index(i, 3), Qt::DisplayRole).toDouble());
            RealTime t = qMax(t_A, t_B);
            CombinedTimelineData td;
            td.t = t;
            td.levelID = levelID;
            td.rowIndex = i;
            timeline.insert(t, td);
            // Count
            bool currentIsDifference(false);
            if (model->data(model->index(td.rowIndex, 0), Qt::DisplayRole).toString() == "+") currentIsDifference = true;
            if (model->data(model->index(td.rowIndex, 0), Qt::DisplayRole).toString() == "-") currentIsDifference = true;
            if ((!insideDifferences) && (currentIsDifference)) {
               insideDifferences = true;
            }
            else if ((insideDifferences) && (!currentIsDifference)) {
                insideDifferences = false;
                countDifferences[levelID] = countDifferences[levelID] + 1;
            }
        }
        if (insideDifferences) {
            insideDifferences = false;
            countDifferences[levelID] = countDifferences[levelID] + 1;
        }
        col++;
    }
    row = 3;
    foreach (RealTime t, timeline.uniqueKeys()) {
        QHash<QString, int> rowsPerModel;
        foreach (CombinedTimelineData td, timeline.values(t)) {
            QPointer<DiffSESforIntervalsTableModel> model = models.value(td.levelID);
            if (!rowsPerModel.contains(td.levelID)) rowsPerModel.insert(td.levelID, 1); else rowsPerModel[td.levelID] = rowsPerModel[td.levelID] + 1;
            for (int j = 0; j < model->columnCount(); ++j) {
                int colOffset = (levelColumn.value(td.levelID) - 1) * model->columnCount();
                QXlsx::Format format;
                if      (model->data(model->index(td.rowIndex, 0), Qt::DisplayRole).toString() == "+") format = format_addition;
                else if (model->data(model->index(td.rowIndex, 0), Qt::DisplayRole).toString() == "-") format = format_deletion;
                // write
                if (j == 0)
                    xlsx.write(row, colOffset + j + 1, model->data(model->index(td.rowIndex, j), Qt::DisplayRole).toString().replace("=", ""), format);
                else
                    xlsx.write(row, colOffset + j + 1, model->data(model->index(td.rowIndex, j), Qt::DisplayRole), format);
            }
        }
        int longest = 1;
        foreach (QString levelID, levelIDs) if (rowsPerModel.contains(levelID)) longest = qMax(longest, rowsPerModel[levelID]);
        row = row  + longest;
    }

    // Print counts
    QString message = QString("%1\t%2\t%3\t").arg(communicationID).arg(annotationID).arg(speakerID);
    foreach (QString levelID, levelIDs) message = message.append(QString::number(countDifferences[levelID])).append("\t");
    if (!message.isEmpty()) message.chop(1);
    ui->editCompareCorporaMessages->appendPlainText(message);
    QApplication::processEvents();

    QString filename = QString("Combined_%1_%2.xlsx").arg(communicationID).arg(speakerID);
    xlsx.saveAs(d->outputPath + filename);
}



