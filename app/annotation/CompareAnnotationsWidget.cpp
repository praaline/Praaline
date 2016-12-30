#include <QString>
#include <QAction>
#include <QMessageBox>

#include "CompareAnnotationsWidget.h"
#include "ui_CompareAnnotationsWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "pnlib/diff/diffintervals.h"
#include "pngui/model/diff/diffsesforintervalstablemodel.h"
#include "pngui/widgets/CheckBoxList.h"
#include "pngui/widgets/GridViewWidget.h"
#include "CorporaManager.h"

struct CompareAnnotationsWidgetData {
    CompareAnnotationsWidgetData() : modelResults(0) {}

    QToolBar *toolbarMain;
    QAction *actionCompare;
    QAction *actionExport;

    // Results grid
    GridViewWidget *gridviewResults;
    dtl::Ses<Interval *>::sesElemVec sesSequence;
    DiffSESforIntervalsTableModel *modelResults;

    // Corpus Manager
    CorporaManager *corporaManager;
};

CompareAnnotationsWidget::CompareAnnotationsWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CompareAnnotationsWidget), d(new CompareAnnotationsWidgetData)
{
    ui->setupUi(this);

    // Setup user interfacce
    d->toolbarMain = new QToolBar(tr("Compare Annotations"), this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(QSize(24, 24));
    this->addToolBar(d->toolbarMain);

    d->gridviewResults = new GridViewWidget(this);
    d->gridviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    this->setCentralWidget(d->gridviewResults);

    setupActions();

    // Corpora manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }
    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));

    connect(ui->comboBoxLeftLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(levelLeftChanged(QString)));
    connect(ui->comboBoxRightLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(levelRightChanged(QString)));
    connect(ui->comboBoxLeftAnnotation, SIGNAL(currentTextChanged(QString)), this, SLOT(annotationLeftChanged(QString)));
    connect(ui->comboBoxRightAnnotation, SIGNAL(currentTextChanged(QString)), this, SLOT(annotationRightChanged(QString)));

    connect(ui->commandCompare, SIGNAL(clicked()), this, SLOT(compare()));
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

void CompareAnnotationsWidget::activeCorpusChanged(const QString &corpusID)
{
    Q_UNUSED(corpusID)
    ui->comboBoxLeftAnnotation->clear();    ui->comboBoxRightAnnotation->clear();
    ui->comboBoxLeftSpeaker->clear();       ui->comboBoxRightSpeaker->clear();
    ui->comboBoxLeftLevel->clear();         ui->comboBoxRightLevel->clear();
    ui->comboBoxLeftAttribute->clear();     ui->comboBoxRightAttribute->clear();
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QPair<QString, QString> pair;
    foreach (pair, corpus->getCommunicationsAnnotationsIDs()) {
        ui->comboBoxLeftAnnotation->addItem(pair.first + " :: " + pair.second, pair.second);
        ui->comboBoxRightAnnotation->addItem(pair.first + " :: " + pair.second, pair.second);
    }
    foreach (AnnotationStructureLevel *level, corpus->annotationStructure()->levels()) {
        ui->comboBoxLeftLevel->addItem(level->name(), level->ID());
        ui->comboBoxRightLevel->addItem(level->name(), level->ID());
    }
    annotationLeftChanged("");
    annotationRightChanged("");
    levelLeftChanged("");
    levelRightChanged("");
}

void CompareAnnotationsWidget::levelLeftChanged(QString text)
{
    Q_UNUSED(text)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString levelID = ui->comboBoxLeftLevel->currentData().toString();
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxLeftAttribute->clear();
    ui->comboBoxLeftAttribute->addItem("", "");
    int i = 0;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxLeftAttribute->addItem(attribute->name(), attribute->ID());
        ui->comboBoxExtraAttributesLeft->insertItem(i, attribute->name(), false);
        i++;
    }
}

void CompareAnnotationsWidget::levelRightChanged(QString text)
{
    Q_UNUSED(text)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString levelID = ui->comboBoxRightLevel->currentData().toString();
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID);
    if (!level) return;
    ui->comboBoxRightAttribute->clear();
    ui->comboBoxRightAttribute->addItem("", "");
    int i = 0;
    foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
        ui->comboBoxLeftAttribute->addItem(attribute->name(), attribute->ID());
        ui->comboBoxExtraAttributesRight->insertItem(i, attribute->name(), false);
        i++;
    }
}

void CompareAnnotationsWidget::annotationLeftChanged(QString text)
{
    Q_UNUSED(text)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString annotationID = ui->comboBoxLeftAnnotation->currentData().toString();
    ui->comboBoxLeftSpeaker->clear();
    ui->comboBoxLeftSpeaker->addItems(corpus->datastoreAnnotations()->getSpeakersInAnnotation(annotationID));
}

void CompareAnnotationsWidget::annotationRightChanged(QString text)
{
    Q_UNUSED(text)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString annotationID = ui->comboBoxRightAnnotation->currentData().toString();
    ui->comboBoxRightSpeaker->clear();
    ui->comboBoxRightSpeaker->addItems(corpus->datastoreAnnotations()->getSpeakersInAnnotation(annotationID));
}

void CompareAnnotationsWidget::compare()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;

    QString annotationID_left, speakerID_left, levelID_left, attributeID_left;
    QString annotationID_right, speakerID_right, levelID_right, attributeID_right;
    annotationID_left = ui->comboBoxLeftAnnotation->currentData().toString();
    speakerID_left = ui->comboBoxLeftSpeaker->currentText();
    levelID_left = ui->comboBoxLeftLevel->currentData().toString();
    attributeID_left = ui->comboBoxLeftAttribute->currentData().toString();
    annotationID_right = ui->comboBoxRightAnnotation->currentData().toString();
    speakerID_right = ui->comboBoxRightSpeaker->currentText();
    levelID_right = ui->comboBoxRightLevel->currentData().toString();
    attributeID_right = ui->comboBoxRightAttribute->currentData().toString();

    AnnotationTierGroup *group_left = corpus->datastoreAnnotations()->getTiers(
                annotationID_left, speakerID_left, QStringList() << levelID_left);
    IntervalTier *tier_left = group_left->getIntervalTierByName(levelID_left);
    if (!tier_left) {
        QMessageBox::warning(this, tr("Cannot find left data"),
                             QString(tr("No data for left pane: annotation %1, speaker %2 and annotation level %3."))
                             .arg(annotationID_left).arg(speakerID_left).arg(levelID_left), QMessageBox::Ok);
        return;
    }
    AnnotationTierGroup *group_right = corpus->datastoreAnnotations()->getTiers(
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
            AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID_left);
            if (level && i < level->attributesCount()) extraAttributes_left << level->attributes().at(i)->ID();
        }
    }
    for (int i = 0; i < ui->comboBoxExtraAttributesRight->count(); ++i) {
        if (ui->comboBoxExtraAttributesRight->itemData(i).toBool() == true) {
            AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID_right);
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

