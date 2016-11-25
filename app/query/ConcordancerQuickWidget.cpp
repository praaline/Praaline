#include <QDebug>
#include <QSpinBox>
#include <QToolBar>
#include <QTreeView>
#include <QTableView>
#include "ConcordancerQuickWidget.h"
#include "ui_ConcordancerQuickWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/query/QueryDefinition.h"
#include "pncore/serialisers/xml/XMLSerialiserCorpusBookmark.h"
using namespace Praaline::Core;

#include "pngui/observers/corpusobserver.h"
#include "pngui/model/checkableproxymodel.h"
#include "pngui/model/query/QueryOccurrenceTableModel.h"
#include "pngui/model/query/queryfiltersequencetablemodel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "pngui/widgets/gridviewwidget.h"
#include "CorporaManager.h"


struct ConcordancerQuickWidgetData {
    ConcordancerQuickWidgetData() : modelLevelsAttributes(0), corporaManager(0), modelResults(0), currentQuery(0) {}

    QToolBar *toolbarMain;
    QAction *actionDefinitionOpen;
    QAction *actionDefinitionSave;

    QToolBar *toolbarMetadataFilters;
    QMainWindow *innerwindowMetadataFilters;
    QAction *actionMetadataFilterAdd;
    QAction *actionMetadataFilterRemove;
    QTreeView *treeviewMetadataFilters;

//    QToolBar *toolbarQueryDefinition;
//    QMainWindow *innerwindowQueryDefinition;
//    QAction *actionQueryAddFilterGroup;
//    QAction *actionQueryAddFilterSequence;
//    QAction *actionQueryAddFilterAttribute;
//    QAction *actionQueryRemove;
//    QTreeView *treeviewQueryDefinition;

    QToolBar *toolbarLevelsAttributes;
    QMainWindow *innerwindowLevelsAttributes;
    QAction *actionLevelAttributeAdd;
    QAction *actionLevelAttributeRemove;
    QTreeView *treeviewLevelsAttributes;
    AnnotationStructureTreeModel *modelLevelsAttributes;
    QSpinBox *spinboxContextSize;

    QList<QComboBox *> filterAttributeComboBoxes;
    QList<QList<QLineEdit *> > filterLineEdits;

    // Results
    QToolBar *toolbarResults;
    QMainWindow *innerwindowResults;
    QAction *actionResultsCreateBookmarks;
    QAction *actionResultsCreateSoundSamples;
    GridViewWidget *tableviewResults;

    QList<QueryOccurrencePointer *> resultsPointers;
    QueryOccurrenceTableModel *modelResults;

    // Corpus Manager
    CorporaManager *corporaManager;

    // Query
    QueryDefinition *currentQuery;
};

ConcordancerQuickWidget::ConcordancerQuickWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConcordancerQuickWidget), d(new ConcordancerQuickWidgetData())
{
    ui->setupUi(this);

    // Toolbars
    d->toolbarMain = new QToolBar(this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(QSize(24, 24));
    d->toolbarMetadataFilters = new QToolBar(this);
    d->toolbarMetadataFilters->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMetadataFilters->setIconSize(QSize(24, 24));
//    d->toolbarQueryDefinition = new QToolBar(this);
//    d->toolbarQueryDefinition->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//    d->toolbarQueryDefinition->setIconSize(QSize(24, 24));
    d->toolbarLevelsAttributes = new QToolBar(this);
    d->toolbarLevelsAttributes->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarLevelsAttributes->setIconSize(QSize(24, 24));
    d->toolbarResults = new QToolBar(this);
    d->toolbarResults->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarResults->setIconSize(QSize(24, 24));
    this->addToolBar(d->toolbarMain);

    // Metadata filters
    d->innerwindowMetadataFilters = new QMainWindow(this);
    d->innerwindowMetadataFilters->addToolBar(d->toolbarMetadataFilters);
    d->treeviewMetadataFilters = new QTreeView(this);
    d->innerwindowMetadataFilters->setCentralWidget(d->treeviewMetadataFilters);
    ui->gridLayoutMetadataFilters->addWidget(d->innerwindowMetadataFilters);

    // Query definition
//    d->innerwindowQueryDefinition = new QMainWindow(this);
//    d->innerwindowQueryDefinition->addToolBar(d->toolbarQueryDefinition);
//    d->treeviewQueryDefinition = new QTreeView(this);
//    d->innerwindowQueryDefinition->setCentralWidget(d->treeviewQueryDefinition);
//    ui->gridLayoutQueryDefinition->addWidget(d->innerwindowQueryDefinition);

    d->filterAttributeComboBoxes << ui->comboBoxAttribute1 << ui->comboBoxAttribute2 << ui->comboBoxAttribute3;
    QList<QLineEdit *> editAttr1, editAttr2, editAttr3;
    editAttr1 << ui->textAttr1Seq1 << ui->textAttr1Seq2 << ui->textAttr1Seq3 << ui->textAttr1Seq4;
    editAttr2 << ui->textAttr2Seq1 << ui->textAttr2Seq2 << ui->textAttr2Seq3 << ui->textAttr2Seq4;
    editAttr3 << ui->textAttr3Seq1 << ui->textAttr3Seq2 << ui->textAttr3Seq3 << ui->textAttr3Seq4;
    d->filterLineEdits << editAttr1 << editAttr2 << editAttr3;

    // Levels/Attributes to display in results
    d->innerwindowLevelsAttributes = new QMainWindow(this);
    d->innerwindowLevelsAttributes->addToolBar(d->toolbarLevelsAttributes);
    d->treeviewLevelsAttributes = new QTreeView(this);
    d->innerwindowLevelsAttributes->setCentralWidget(d->treeviewLevelsAttributes);
    ui->gridLayoutLevelsAttributes->addWidget(d->innerwindowLevelsAttributes);

    // Results
    d->tableviewResults = new GridViewWidget(this);
    d->tableviewResults->tableView()->verticalHeader()->setDefaultSectionSize(20);
    d->innerwindowResults = new QMainWindow(this);
    d->innerwindowResults->addToolBar(d->toolbarResults);
    d->innerwindowResults->setCentralWidget(d->tableviewResults);
    ui->gridLayoutResults->addWidget(d->innerwindowResults);
    ui->optionFormatCompact->setChecked(true);

    setupActions();
    connect(ui->commandSearchOccurrences, SIGNAL(clicked()), this, SLOT(searchOccurrences()));

    // Corpora manager
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }

    connect(d->corporaManager, SIGNAL(activeCorpusChanged(QString)), this, SLOT(activeCorpusChanged(QString)));
    connect(ui->comboBoxAnnotationLevel, SIGNAL(currentTextChanged(QString)), this, SLOT(levelChanged(QString)));
}

ConcordancerQuickWidget::~ConcordancerQuickWidget()
{
    delete ui;
    delete d;
}

void ConcordancerQuickWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // MAIN TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionDefinitionOpen = new QAction(QIcon(":/icons/actions/action_open.png"), tr("Open Query Definition"), this);
    connect(d->actionDefinitionOpen, SIGNAL(triggered()), SLOT(definitionOpen()));
    command = ACTION_MANAGER->registerAction("Query.ConcordancerQuick.DefinitionOpen", d->actionDefinitionOpen, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionDefinitionOpen);

    d->actionDefinitionSave = new QAction(QIcon(":/icons/actions/action_save.png"), tr("Save Query Definition"), this);
    connect(d->actionDefinitionSave, SIGNAL(triggered()), SLOT(definitionSave()));
    command = ACTION_MANAGER->registerAction("Query.ConcordancerQuick.DefinitionSave", d->actionDefinitionSave, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionDefinitionSave);

    // METADATA FILTER TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionMetadataFilterAdd = new QAction(QIcon(":/icons/actions/list_add.png"), tr("Add Filter"), this);
    connect(d->actionMetadataFilterAdd, SIGNAL(triggered()), SLOT(metadataFilterAdd()));
    command = ACTION_MANAGER->registerAction("Query.ConcordancerQuick.MetadataFilterAdd", d->actionMetadataFilterAdd, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataFilters->addAction(d->actionMetadataFilterAdd);

    d->actionMetadataFilterRemove = new QAction(QIcon(":/icons/actions/list_remove.png"), tr("Remove Filter"), this);
    connect(d->actionMetadataFilterRemove, SIGNAL(triggered()), SLOT(metadataFilterRemove()));
    command = ACTION_MANAGER->registerAction("Query.ConcordancerQuick.MetadataFilterRemove", d->actionMetadataFilterRemove, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataFilters->addAction(d->actionMetadataFilterRemove);


    // LEVELS-ATTRIBUTES TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionLevelAttributeAdd = new QAction(QIcon(":/icons/actions/list_add.png"), tr("Add"), this);
    connect(d->actionLevelAttributeAdd, SIGNAL(triggered()), SLOT(displayLevelsAttributesAdd()));
    command = ACTION_MANAGER->registerAction("Query.C5oncordancer.LevelsAttributesAdd", d->actionLevelAttributeAdd, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionLevelAttributeAdd);

    d->actionLevelAttributeRemove = new QAction(QIcon(":/icons/actions/list_remove.png"), tr("Remove"), this);
    connect(d->actionLevelAttributeRemove, SIGNAL(triggered()), SLOT(displayLevelsAttributesRemove()));
    command = ACTION_MANAGER->registerAction("Query.ConcordancerQuick.LevelsAttributesRemove", d->actionLevelAttributeRemove, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionLevelAttributeRemove);

    d->actionResultsCreateBookmarks = new QAction(QIcon(":/icons/actions/action_export.png"), tr("Save as Bookmarks"), this);
    connect(d->actionResultsCreateBookmarks, SIGNAL(triggered()), SLOT(resultsCreateBookmarks()));
    command = ACTION_MANAGER->registerAction("Query.ConcordancerQuick.ResultsCreateBookmarks", d->actionResultsCreateBookmarks, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarResults->addAction(d->actionResultsCreateBookmarks);

}

void ConcordancerQuickWidget::activeCorpusChanged(const QString &corpusID)
{
    Q_UNUSED(corpusID)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    // In any case, clear results
    d->tableviewResults->tableView()->setModel(0);
    if (d->modelResults) delete d->modelResults;
    if (corpus) {
        AnnotationStructureTreeModel *model = new AnnotationStructureTreeModel(corpus->annotationStructure(), true, true, this);
        d->treeviewLevelsAttributes->setModel(model);
        if (d->modelLevelsAttributes) delete d->modelLevelsAttributes;
        d->modelLevelsAttributes = model;
        // update levels
        ui->comboBoxAnnotationLevel->clear();
        foreach (AnnotationStructureLevel *level, corpus->annotationStructure()->levels()) {
            ui->comboBoxAnnotationLevel->addItem(level->name(), level->ID());
        }
        levelChanged("");
    }
}

void ConcordancerQuickWidget::levelChanged(const QString &text)
{
    Q_UNUSED(text)
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QString levelID = ui->comboBoxAnnotationLevel->currentData().toString();
    AnnotationStructureLevel *level = corpus->annotationStructure()->level(levelID);
    if (!level) return;
    foreach (QComboBox *comboBoxAttributes, d->filterAttributeComboBoxes) {
        comboBoxAttributes->clear();
        comboBoxAttributes->insertItem(0, QString(tr("%1 (text)")).arg(level->name()), level->ID());
        foreach (AnnotationStructureAttribute *attribute, level->attributes()) {
            comboBoxAttributes->addItem(attribute->name(), attribute->ID());
        }
    }
}


void ConcordancerQuickWidget::definitionOpen()
{

}

void ConcordancerQuickWidget::definitionSave()
{

}

void ConcordancerQuickWidget::metadataFilterAdd()
{

}

void ConcordancerQuickWidget::metadataFilterRemove()
{

}

void ConcordancerQuickWidget::queryAddFilterGroup()
{
    if (!d->currentQuery) return;
    d->currentQuery->filterGroups << QueryFilterGroup();
}

void ConcordancerQuickWidget::queryAddFilterSequence()
{
    if (!d->currentQuery) return;
    if (d->currentQuery->filterGroups.empty()) return;
    bool ok;
    QString levelID = QInputDialog::getText(this, tr("Annotation Level ID"), tr("Level ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || levelID.isEmpty()) return;
    QueryFilterSequence sequence(levelID);
    d->currentQuery->filterGroups.first().filterSequences.append(sequence);
}

void ConcordancerQuickWidget::queryAddFilterAttribute()
{
    if (!d->currentQuery) return;
    if (d->currentQuery->filterGroups.empty()) return;
    QueryFilterGroup &group = d->currentQuery->filterGroups.first();
    if (group.filterSequences.empty()) return;
    bool ok;
    QString attributeID = QInputDialog::getText(this, tr("Annotation Level ID"), tr("Level ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || attributeID.isEmpty()) return;
    group.filterSequences.first().addAttribute(attributeID);
    group.filterSequences.first().expandSequence(2);
}

void ConcordancerQuickWidget::queryRemove()
{

}

void ConcordancerQuickWidget::displayLevelsAttributesAdd()
{

}

void ConcordancerQuickWidget::displayLevelsAttributesRemove()
{

}

void ConcordancerQuickWidget::searchOccurrences()
{

//    if (d->currentQuery.filterGroups.empty()) return;
//    QueryFilterGroup &group = d->currentQuery.filterGroups.first();
//    if (group.filterSequences.empty()) return;
//    QueryFilterSequenceTableModel *seqmodel = new QueryFilterSequenceTableModel(group.filterSequences[0], this);
//    d->treeviewQueryDefinition->setModel(seqmodel);

    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    if (d->modelLevelsAttributes->selectedLevelsAttributes().isEmpty()) {
        QMessageBox::warning(this, tr("No output selected"),
                             tr("You must select the output annotation Levels/Attributes from the panel on the right. "
                             "What do you want to see in your search results?"));
        return;
    }
    QString levelID = ui->comboBoxAnnotationLevel->currentData().toString();
    if (!corpus->annotationStructure()->hasLevel(levelID)) return;
    QueryDefinition *query = new QueryDefinition();
    query->filterGroups << QueryFilterGroup();
    QueryFilterSequence sequence(levelID);
    int sequenceLength = 0;
    foreach (QList<QLineEdit *> editForAttr, d->filterLineEdits) {
        for (int i = 0; i < editForAttr.count(); ++i) {
            if ((!editForAttr.at(i)->text().trimmed().isEmpty()) && (sequenceLength < i + 1))
                sequenceLength = i + 1;
        }
    }
    if (sequenceLength == 0) return;
    for (int i = 0; i < d->filterAttributeComboBoxes.count(); ++i) {
        QString attributeID = d->filterAttributeComboBoxes.at(i)->currentData().toString();
        bool add = false;
        foreach (QLineEdit *edit, d->filterLineEdits.at(i)) {
            if (!edit->text().trimmed().isEmpty()) add = true;
        }
        if (add) sequence.addAttribute(attributeID);
    }
    sequence.expandSequence(sequenceLength);
    for (int i = 0; i < d->filterAttributeComboBoxes.count(); ++i) {
        QString attributeID = d->filterAttributeComboBoxes.at(i)->currentData().toString();
        QList<QLineEdit *> editsForAttr = d->filterLineEdits.at(i);
        for (int j = 0; j < editsForAttr.count(); ++j) {
            QString value = editsForAttr.at(j)->text().trimmed();
            if (!value.isEmpty()) {
                if (value.contains("%"))
                    sequence.setCondition(attributeID, j, QueryFilterSequence::Condition(QueryFilterSequence::Like, value));
                else
                    sequence.setCondition(attributeID, j, QueryFilterSequence::Condition(QueryFilterSequence::Equals, value));
            }
        }
    }
    query->filterGroups.first().filterSequences.append(sequence);

    // Results attributes
    QPair<QString, QString> pair;
    foreach (pair, d->modelLevelsAttributes->selectedLevelsAttributes())
        query->resultLevelsAttributes << QPair<QString, QString>(pair);
    if (query->resultLevelsAttributes.isEmpty()) return;

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    qApp->processEvents();
    QList<QueryOccurrencePointer *> occurrencePointers = corpus->datastoreAnnotations()->runQuery(query);
    ui->progressBar->setMaximum(occurrencePointers.count());
    d->resultsPointers = occurrencePointers;

    bool multiline = false;
    if (ui->optionFormatMultiline->isChecked()) multiline = true;
    qDebug() << query->resultLevelsAttributes;
    QueryOccurrenceTableModel *modelResults = new QueryOccurrenceTableModel(corpus, query, occurrencePointers, multiline, this);
    d->tableviewResults->tableView()->setModel(modelResults);
    if (d->modelResults) delete d->modelResults;
    d->modelResults = modelResults;
    if (d->currentQuery) delete d->currentQuery;
    d->currentQuery = query;
}

void ConcordancerQuickWidget::resultsSaveChanges()
{

}

void ConcordancerQuickWidget::resultsCreateBookmarks()
{
    QString filename;
    if (filename.isEmpty()) {
        QFileDialog::Options options;
        QString selectedFilter;
        filename = QFileDialog::getSaveFileName(this, tr("Save Bookmarks"),
                                                tr("query_results.xml"), tr("Praaline Bookmarks (*.xml);;All Files (*)"),
                                                &selectedFilter, options);
    }
    if (filename.isEmpty()) return;

    QList<QPointer<CorpusBookmark> > bookmarks;
    foreach (QueryOccurrencePointer *pointer, d->resultsPointers) {
        bookmarks << new CorpusBookmark(pointer->corpusID, pointer->communicationID, pointer->annotationID,
                                        RealTime::fromNanoseconds(pointer->tMin_nsec), "", "");
    }

    XMLSerialiserCorpusBookmark::saveCorpusBookmarks(bookmarks, filename);
}
