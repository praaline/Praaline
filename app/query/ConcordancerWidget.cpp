#include <QDebug>
#include <QSpinBox>
#include <QToolBar>
#include <QTreeView>
#include <QTableView>
#include "ConcordancerWidget.h"
#include "ui_ConcordancerWidget.h"

#include "pncore/corpus/Corpus.h"
#include "pncore/query/QueryDefinition.h"
#include "pncore/serialisers/xml/XMLSerialiserCorpusBookmark.h"
using namespace Praaline::Core;

#include "pngui/observers/corpusobserver.h"
#include "pngui/model/checkableproxymodel.h"
#include "pngui/model/query/QueryOccurrenceTableModel.h"
#include "pngui/model/query/queryfiltersequencetablemodel.h"
#include "pngui/model/corpus/AnnotationStructureTreeModel.h"
#include "CorporaManager.h"

struct ConcordancerWidgetData {
    ConcordancerWidgetData() {}

    QToolBar *toolbarMain;
    QAction *actionDefinitionOpen;
    QAction *actionDefinitionSave;

    QToolBar *toolbarMetadataFilters;
    QMainWindow *innerwindowMetadataFilters;
    QAction *actionMetadataFilterAdd;
    QAction *actionMetadataFilterRemove;
    QTreeView *treeviewMetadataFilters;

    QToolBar *toolbarQueryDefinition;
    QMainWindow *innerwindowQueryDefinition;
    QAction *actionQueryAddFilterGroup;
    QAction *actionQueryAddFilterSequence;
    QAction *actionQueryAddFilterAttribute;
    QAction *actionQueryRemove;
    QTreeView *treeviewQueryDefinition;

    QToolBar *toolbarLevelsAttributes;
    QMainWindow *innerwindowLevelsAttributes;
    QAction *actionLevelAttributeAdd;
    QAction *actionLevelAttributeRemove;
    QTreeView *treeviewLevelsAttributes;
    QSpinBox *spinboxContextSize;

    QToolBar *toolbarResults;
    QMainWindow *innerwindowResults;
    QAction *actionResultsSaveAnnotations;
    QAction *actionResultsExport;
    QAction *actionResultsCreateBookmarks;
    QTableView *tableviewResults;

    // Corpus Manager
    CorporaManager *corporaManager;

    // Query
    QueryDefinition currentQuery;
};

ConcordancerWidget::ConcordancerWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConcordancerWidget)
{
    ui->setupUi(this);
    d = new ConcordancerWidgetData();

    // Toolbars
    d->toolbarMain = new QToolBar(this);
    d->toolbarMain->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMain->setIconSize(QSize(24, 24));
    d->toolbarMetadataFilters = new QToolBar(this);
    d->toolbarMetadataFilters->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarMetadataFilters->setIconSize(QSize(24, 24));
    d->toolbarQueryDefinition = new QToolBar(this);
    d->toolbarQueryDefinition->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarQueryDefinition->setIconSize(QSize(24, 24));
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
    d->innerwindowQueryDefinition = new QMainWindow(this);
    d->innerwindowQueryDefinition->addToolBar(d->toolbarQueryDefinition);
    d->treeviewQueryDefinition = new QTreeView(this);
    d->innerwindowQueryDefinition->setCentralWidget(d->treeviewQueryDefinition);
    ui->gridLayoutQueryDefinition->addWidget(d->innerwindowQueryDefinition);

    // Levels/Attributes to display in results
    d->innerwindowLevelsAttributes = new QMainWindow(this);
    d->innerwindowLevelsAttributes->addToolBar(d->toolbarLevelsAttributes);
    d->treeviewLevelsAttributes = new QTreeView(this);
    d->innerwindowLevelsAttributes->setCentralWidget(d->treeviewLevelsAttributes);
    ui->gridLayoutLevelsAttributes->addWidget(d->innerwindowLevelsAttributes);

    // Results
    d->innerwindowResults = new QMainWindow(this);
    d->innerwindowResults->addToolBar(d->toolbarResults);
    d->tableviewResults = new QTableView(this);
    d->tableviewResults->verticalHeader()->setDefaultSectionSize(22);
    d->innerwindowResults->setCentralWidget(d->tableviewResults);
    ui->gridLayoutResults->addWidget(d->innerwindowResults);

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

}

ConcordancerWidget::~ConcordancerWidget()
{
    delete ui;
    delete d;
}

void ConcordancerWidget::setupActions()
{
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command;

    // MAIN TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionDefinitionOpen = new QAction(QIcon(":/icons/actions/action_open.png"), "Open Query Definition", this);
    connect(d->actionDefinitionOpen, SIGNAL(triggered()), SLOT(definitionOpen()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.DefinitionOpen", d->actionDefinitionOpen, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionDefinitionOpen);

    d->actionDefinitionSave = new QAction(QIcon(":/icons/actions/action_save.png"), "Save Query Definition", this);
    connect(d->actionDefinitionSave, SIGNAL(triggered()), SLOT(definitionSave()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.DefinitionSave", d->actionDefinitionSave, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMain->addAction(d->actionDefinitionSave);

    // METADATA FILTER TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionMetadataFilterAdd = new QAction(QIcon(":/icons/actions/list_add.png"), "Add Filter", this);
    connect(d->actionMetadataFilterAdd, SIGNAL(triggered()), SLOT(metadataFilterAdd()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.MetadataFilterAdd", d->actionMetadataFilterAdd, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataFilters->addAction(d->actionMetadataFilterAdd);

    d->actionMetadataFilterRemove = new QAction(QIcon(":/icons/actions/list_remove.png"), "Remove Filter", this);
    connect(d->actionMetadataFilterRemove, SIGNAL(triggered()), SLOT(metadataFilterRemove()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.MetadataFilterRemove", d->actionMetadataFilterRemove, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarMetadataFilters->addAction(d->actionMetadataFilterRemove);

    // QUERY TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionQueryAddFilterGroup = new QAction(QIcon(":/icons/actions/list_add.png"), "Group", this);
    connect(d->actionQueryAddFilterGroup, SIGNAL(triggered()), SLOT(queryAddFilterGroup()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.QueryAddFilterGroup", d->actionQueryAddFilterGroup, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarQueryDefinition->addAction(d->actionQueryAddFilterGroup);

    d->actionQueryAddFilterSequence = new QAction(QIcon(":/icons/actions/list_add.png"), "Level", this);
    connect(d->actionQueryAddFilterSequence, SIGNAL(triggered()), SLOT(queryAddFilterSequence()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.QueryAddFilterSequence", d->actionQueryAddFilterSequence, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarQueryDefinition->addAction(d->actionQueryAddFilterSequence);

    d->actionQueryAddFilterAttribute = new QAction(QIcon(":/icons/actions/list_add.png"), "Attribute", this);
    connect(d->actionQueryAddFilterAttribute, SIGNAL(triggered()), SLOT(queryAddFilterAttribute()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.QueryAddFilterAttribute", d->actionQueryAddFilterAttribute, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarQueryDefinition->addAction(d->actionQueryAddFilterAttribute);

    d->actionQueryRemove = new QAction(QIcon(":/icons/actions/list_remove.png"), "Remove", this);
    connect(d->actionQueryRemove, SIGNAL(triggered()), SLOT(queryRemove()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.QueryRemove", d->actionQueryRemove, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarQueryDefinition->addAction(d->actionQueryRemove);

    // LEVELS-ATTRIBUTES TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionLevelAttributeAdd = new QAction(QIcon(":/icons/actions/list_add.png"), "Add", this);
    connect(d->actionLevelAttributeAdd, SIGNAL(triggered()), SLOT(displayLevelsAttributesAdd()));
    command = ACTION_MANAGER->registerAction("Query.C5oncordancer.LevelsAttributesAdd", d->actionLevelAttributeAdd, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionLevelAttributeAdd);

    d->actionLevelAttributeRemove = new QAction(QIcon(":/icons/actions/list_remove.png"), "Remove", this);
    connect(d->actionLevelAttributeRemove, SIGNAL(triggered()), SLOT(displayLevelsAttributesRemove()));
    command = ACTION_MANAGER->registerAction("Query.Concordancer.LevelsAttributesRemove", d->actionLevelAttributeRemove, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarLevelsAttributes->addAction(d->actionLevelAttributeRemove);

    // RESULTS TOOLBAR
    // ----------------------------------------------------------------------------------------------------------------
    d->actionResultsExport = new QAction(QIcon(":/icons/actions/export.png"), "Export", this);
    connect(d->actionResultsExport, SIGNAL(triggered()), SLOT(resultsExport()));
    command = ACTION_MANAGER->registerAction("Query.C5oncordancer.ResultsExport", d->actionResultsExport, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->toolbarResults->addAction(d->actionResultsExport);

}

void ConcordancerWidget::activeCorpusChanged(const QString &corpusID)
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    AnnotationStructureTreeModel *model = new AnnotationStructureTreeModel(corpus->annotationStructure(), true, true, this);
    d->treeviewLevelsAttributes->setModel(model);
}

void ConcordancerWidget::definitionOpen()
{

}

void ConcordancerWidget::definitionSave()
{

}

void ConcordancerWidget::metadataFilterAdd()
{

}

void ConcordancerWidget::metadataFilterRemove()
{

}

void ConcordancerWidget::queryAddFilterGroup()
{
    d->currentQuery.filterGroups << QueryFilterGroup();
}

void ConcordancerWidget::queryAddFilterSequence()
{
    if (d->currentQuery.filterGroups.empty()) return;
    bool ok;
    QString levelID = QInputDialog::getText(this, tr("Annotation Level ID"), tr("Level ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || levelID.isEmpty()) return;
    QueryFilterSequence sequence(levelID);
    d->currentQuery.filterGroups.first().filterSequences.append(sequence);
}

void ConcordancerWidget::queryAddFilterAttribute()
{
    if (d->currentQuery.filterGroups.empty()) return;
    QueryFilterGroup &group = d->currentQuery.filterGroups.first();
    if (group.filterSequences.empty()) return;
    bool ok;
    QString attributeID = QInputDialog::getText(this, tr("Annotation Level ID"), tr("Level ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || attributeID.isEmpty()) return;
    group.filterSequences.first().addAttribute(attributeID);
    group.filterSequences.first().expandSequence(2);
}

void ConcordancerWidget::queryRemove()
{

}

void ConcordancerWidget::displayLevelsAttributesAdd()
{

}

void ConcordancerWidget::displayLevelsAttributesRemove()
{

}

void ConcordancerWidget::resultsExport()
{

}

void ConcordancerWidget::resultsSaveChanges()
{

}

void ConcordancerWidget::resultsCreateBookmarks()
{

}

void ConcordancerWidget::searchOccurrences()
{

    if (d->currentQuery.filterGroups.empty()) return;
    QueryFilterGroup &group = d->currentQuery.filterGroups.first();
    if (group.filterSequences.empty()) return;
    QueryFilterSequenceTableModel *seqmodel = new QueryFilterSequenceTableModel(group.filterSequences[0], this);
    d->treeviewQueryDefinition->setModel(seqmodel);

    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
//    if (!d->first){
//        QList<QueryOccurrence *> occurrences = corpus->datastoreAnnotations()->runQuery(d->currentQuery);

//        QueryOccurrenceTableModel *model = new QueryOccurrenceTableModel(d->currentQuery, occurrences, this);
//        d->tableviewResults->setModel(model);
////    }
////    else {
//        d->currentQuery.resultLevelsAttributes.append(QPair<QString, QString>("tok_min", ""));
//        d->currentQuery.resultLevelsAttributes.append(QPair<QString, QString>("tok_min", "pos_min"));
}

