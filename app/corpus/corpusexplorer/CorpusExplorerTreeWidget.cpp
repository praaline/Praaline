#include <memory>
using namespace std;

#include <QString>
#include <QList>
#include <QSet>
#include <QHash>
#include <QAction>
#include <QToolBar>
#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QTreeView>
#include <QProgressDialog>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Datastore/MetadataDatastore.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
using namespace Praaline::Core;

#include "pngui/model/corpus/CorpusExplorerTreeModel.h"
#include "pngui/widgets/SelectionDialog.h"
#include "pngui/widgets/MetadataEditorWidget.h"
#include "pngui/widgets/CorpusItemPreview.h"
#include "pngui/observers/CorpusObserver.h"

#include "pngui/PraalineUserInterfaceOptions.h"
#include "CorpusRepositoriesManager.h"

#include "../CorpusModeWidget.h"

#include "CorpusExplorerOptionsDialog.h"
#include "AddCorpusCommunicationDialog.h"

#include "CorpusExplorerTreeWidget.h"
#include "ui_CorpusExplorerTreeWidget.h"

struct CorpusExplorerTreeWidgetData {
    CorpusExplorerTreeWidgetData() :
        widgetCorpusMode(nullptr), corpusRepositoriesManager(nullptr),
        corporaTopLevelNode(nullptr), corporaObserverWidget(nullptr),
        metadataEditorPrimary(nullptr), metadataEditorSecondary(nullptr),
        preview(nullptr)
    {}

    // Pointer to the parent widget
    CorpusModeWidget *widgetCorpusMode;
    // Corpora
    QAction *actionCreateCorpus;
    QAction *actionOpenCorpus;
    QAction *actionSaveMetadata;
    QAction *actionDeleteCorpus;
    // Corpus items
    QAction *actionAddCommunication;
    QAction *actionAddSpeaker;
    QAction *actionAddRecording;
    QAction *actionAddAnnotation;
    QAction *actionAddParticipation;
    QAction *actionRemoveCorpusItems;
    QAction *actionRelinkCorpusItem;
    // Presentation
    QAction *actionToggleSearchBox;
    QAction *actionAttributesAndGroupings;
    QAction *actionMetadataEditorPrimaryStyleTree;
    QAction *actionMetadataEditorPrimaryStyleGroupBox;
    QAction *actionMetadataEditorPrimaryStyleButton;
    QAction *actionMetadataEditorSecondaryStyleTree;
    QAction *actionMetadataEditorSecondaryStyleGroupBox;
    QAction *actionMetadataEditorSecondaryStyleButton;

    CorpusRepositoriesManager *corpusRepositoriesManager;
    QHash<QString, CorpusObserver *> observersForCorpusRepositories;
    QPointer<TreeNode> corporaTopLevelNode;
    QPointer<Corpus> activeCorpus;

    ObserverWidget* corporaObserverWidget;
    MetadataEditorWidget *metadataEditorPrimary;
    MetadataEditorWidget *metadataEditorSecondary;
    CorpusItemPreview *preview;

    QToolBar *toolbarCorpusExplorer;
    QMenu *menuMetadataEditorStyles;
};

CorpusExplorerTreeWidget::CorpusExplorerTreeWidget(CorpusModeWidget *widgetCorpusMode, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CorpusExplorerTreeWidget), d(new CorpusExplorerTreeWidgetData)
{
    ui->setupUi(this);

    // Parent corpus mode widget
    d->widgetCorpusMode = widgetCorpusMode;

    // Get Corpus Repositories Manager from global object list. When corpus repositories are opened or closed (i.e. added or removed from the manager)
    // we will receive a signal, and update the corpus explorer tree accordingly.
    foreach (QObject* obj, OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager")) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
    if (d->corpusRepositoriesManager) {
        connect(d->corpusRepositoriesManager, &CorpusRepositoriesManager::corpusRepositoryAdded,
                this, &CorpusExplorerTreeWidget::corpusRepositoryAdded);
        connect(d->corpusRepositoriesManager, &CorpusRepositoriesManager::corpusRepositoryRemoved,
                this, &CorpusExplorerTreeWidget::corpusRepositoryRemoved);
    }

    // Create an observer tree for corpus items
    d->corporaTopLevelNode = new TreeNode(tr("Corpus Explorer"));
    d->corporaTopLevelNode->enableNamingControl(ObserverHints::EditableNames, NamingPolicyFilter::ProhibitDuplicateNames,
                                                NamingPolicyFilter::AutoRename);
    d->corporaTopLevelNode->displayHints()->setActionHints(ObserverHints::ActionNoHints);
    d->corporaTopLevelNode->displayHints()->setDisplayFlagsHint(ObserverHints::NoDisplayFlagsHint);
    d->corporaTopLevelNode->displayHints()->setDragDropHint(ObserverHints::AllDragDrop);
    // Uncomment these to play around with root display formats.
    //d->corporaTopLevelNode->displayHints()->setRootIndexDisplayHint(ObserverHints::RootIndexDisplayUndecorated);
    //d->corporaTopLevelNode->displayHints()->setRootIndexDisplayHint(ObserverHints::RootIndexDisplayDecorated);
    d->corporaTopLevelNode->displayHints()->setRootIndexDisplayHint(ObserverHints::RootIndexHide); // This is the default
    // Register the observer tree for corpus items in the global object pool
    OBJECT_MANAGER->registerObject(d->corporaTopLevelNode, QtilitiesCategory("Corpus"));

    // Set up corpora observer widget.
    d->corporaObserverWidget = new ObserverWidget(Qtilities::TreeView);
    d->corporaObserverWidget->setRefreshMode(ObserverWidget::RefreshModeShowTree);
    d->corporaObserverWidget->setGlobalMetaType("Corpus Tree Meta Type");
    // Support drag and drop
    d->corporaObserverWidget->setAcceptDrops(true);
    // Events whenever the user changes its selection or double clicks on an item
    connect(d->corporaObserverWidget, &ObserverWidget::selectedObjectsChanged,
            this, &CorpusExplorerTreeWidget::corporaObserverWidgetSelectedObjectsChanged);
    connect(d->corporaObserverWidget, &Qtilities::CoreGui::ObserverWidget::doubleClickRequest,
            this, &CorpusExplorerTreeWidget::corporaObserverWidgetDoubleClickRequest);
    d->corporaObserverWidget->setObserverContext(d->corporaTopLevelNode);
    d->corporaObserverWidget->layout()->setMargin(0);
    d->corporaObserverWidget->initialize();
    d->corporaObserverWidget->toggleSearchBox();

    // Menu and Toolbar actions
    d->toolbarCorpusExplorer = new QToolBar(tr("Corpus Explorer"), this);
    d->toolbarCorpusExplorer->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbarCorpusExplorer->setIconSize(PraalineUserInterfaceOptions::smallIconSize());
    setupActions();
    this->addToolBar(d->toolbarCorpusExplorer);

    // Set up corpus item preview widget
    d->preview = new CorpusItemPreview(this);
    d->preview->layout()->setMargin(0);
    ui->dockCorpusItemPreview->setWidget(d->preview);

    // Create layout of the Corpus Explorer
    ui->gridLayout->setMargin(0);
    ui->dockCorpusExplorer->setWidget(d->corporaObserverWidget);
    d->metadataEditorPrimary = new MetadataEditorWidget(MetadataEditorWidget::TreeStyle, this);
    d->metadataEditorPrimary->layout()->setMargin(0);
    ui->dockMetadataPrimary->setWidget(d->metadataEditorPrimary);
    d->metadataEditorSecondary = new MetadataEditorWidget(MetadataEditorWidget::TreeStyle, this);
    d->metadataEditorSecondary->layout()->setMargin(0);
    ui->dockMetadataSecondary->setWidget(d->metadataEditorSecondary);

    // Layout
    // Set proportions
    QList<int> sizes;
    sizes << static_cast<int>(width() * 0.3) << static_cast<int>(width() * 0.7);
    ui->splitterMainVertical->setSizes(sizes);
    sizes.clear();
    sizes << static_cast<int>(height() * 0.7) << static_cast<int>(height() * 0.3);
    ui->splitterEditorsPreviewHorizontal->setSizes(sizes);
}

CorpusExplorerTreeWidget::~CorpusExplorerTreeWidget()
{
    delete ui;
    delete d;
}

void CorpusExplorerTreeWidget::setupActions()
{
    bool existed {false};
    Command* command {nullptr};

    // Context and menu bar
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    ActionContainer *menu_bar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer *menu_view = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    if (!existed) menu_bar->addMenu(menu_view, qti_action_HELP);
    ActionContainer *menu_corpus = ACTION_MANAGER->createMenu(tr("&Corpus"), existed);
    if (!existed) menu_bar->addMenu(menu_corpus, qti_action_HELP);
    ActionContainer *menu_annotation = ACTION_MANAGER->createMenu(tr("&Annotation"), existed);
    if (!existed) menu_bar->addMenu(menu_annotation, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // CORPUS MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionCreateCorpus = new QAction(QIcon(":icons/actions/action_new.png"), tr("Create New Corpus..."), this);
    connect(d->actionCreateCorpus, &QAction::triggered, this, &CorpusExplorerTreeWidget::createCorpus);
    command = ACTION_MANAGER->registerAction("Corpus.CreateCorpus", d->actionCreateCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionCreateCorpus);

    d->actionOpenCorpus = new QAction(QIcon(":icons/actions/action_open.png"), tr("Open Corpus"), this);
    connect(d->actionOpenCorpus, &QAction::triggered, this, &CorpusExplorerTreeWidget::openCorpus);
    command = ACTION_MANAGER->registerAction("Corpus.OpenCorpus", d->actionOpenCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionOpenCorpus);

    d->actionSaveMetadata = new QAction(QIcon(":icons/actions/action_save.png"), tr("Save Metadata"), this);
    connect(d->actionSaveMetadata, &QAction::triggered, this, &CorpusExplorerTreeWidget::saveCorpusMetadata);
    command = ACTION_MANAGER->registerAction("Corpus.SaveMetadata", d->actionSaveMetadata, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionSaveMetadata);

    d->actionDeleteCorpus = new QAction(QIcon(":icons/actions/action_delete.png"), tr("Delete Corpus"), this);
    connect(d->actionDeleteCorpus, &QAction::triggered, this, &CorpusExplorerTreeWidget::deleteCorpus);
    command = ACTION_MANAGER->registerAction("Corpus.DeleteCorpus", d->actionDeleteCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    menu_corpus->addSeparator();

    // Corpus Items
    // --------------------------------------------------------------------------------------------
    d->actionAddCommunication = new QAction(QIcon(":icons/actions/list_add.png"), tr("Add Communication..."), this);
    connect(d->actionAddCommunication, &QAction::triggered, this, &CorpusExplorerTreeWidget::addCommunication);
    command = ACTION_MANAGER->registerAction("Corpus.AddCommunication", d->actionAddCommunication, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddCommunication);

    d->actionAddSpeaker = new QAction(QIcon(":icons/actions/list_add.png"), tr("Add Speaker..."), this);
    connect(d->actionAddSpeaker, &QAction::triggered, this, &CorpusExplorerTreeWidget::addSpeaker);
    command = ACTION_MANAGER->registerAction("Corpus.AddSpeaker", d->actionAddSpeaker, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddSpeaker);

    d->actionAddRecording = new QAction(QIcon(":icons/actions/list_add.png"), tr("Add Media Recording..."), this);
    connect(d->actionAddRecording, &QAction::triggered, this, &CorpusExplorerTreeWidget::addRecording);
    command = ACTION_MANAGER->registerAction("Corpus.AddRecording", d->actionAddRecording, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddRecording);

    d->actionAddAnnotation = new QAction(QIcon(":icons/actions/list_add.png"), tr("Add Annotation..."), this);
    connect(d->actionAddAnnotation, &QAction::triggered, this, &CorpusExplorerTreeWidget::addAnnotation);
    command = ACTION_MANAGER->registerAction("Corpus.AddAnnotation", d->actionAddAnnotation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddAnnotation);

    d->actionAddParticipation = new QAction(QIcon(":icons/actions/list_add.png"), tr("Add Participation..."), this);
    connect(d->actionAddParticipation, &QAction::triggered, this, &CorpusExplorerTreeWidget::addParticipation);
    command = ACTION_MANAGER->registerAction("Corpus.AddParticipation", d->actionAddParticipation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddParticipation);

    d->actionRemoveCorpusItems = new QAction(QIcon(":icons/actions/list_remove.png"), tr("Remove Corpus Item(s)"), this);
    connect(d->actionRemoveCorpusItems, &QAction::triggered, this, &CorpusExplorerTreeWidget::removeCorpusItems);
    command = ACTION_MANAGER->registerAction("Corpus.RemoveCorpusItems", d->actionRemoveCorpusItems, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionRemoveCorpusItems);

    d->actionRelinkCorpusItem = new QAction(tr("Link Item to another Communication..."), this);
    connect(d->actionRelinkCorpusItem, &QAction::triggered, this, &CorpusExplorerTreeWidget::relinkCorpusItem);
    command = ACTION_MANAGER->registerAction("Corpus.RelinkCorpusItem", d->actionRelinkCorpusItem, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    menu_corpus->addSeparator();

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionAttributesAndGroupings = new QAction(tr("Select Attributes and Groupings"), this);
    connect(d->actionAttributesAndGroupings, &QAction::triggered, this, &CorpusExplorerTreeWidget::attributesAndGroupings);
    command = ACTION_MANAGER->registerAction("Corpus.AttributesAndGroupings", d->actionAttributesAndGroupings, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_view->addAction(command);

    d->actionToggleSearchBox = new QAction(QIcon(":icons/corpusexplorer/search.png"), tr("Search"), this);
    connect(d->actionToggleSearchBox, &QAction::triggered, d->corporaObserverWidget, &Qtilities::CoreGui::ObserverWidget::toggleSearchBox);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.ToggleSearchBox", d->actionToggleSearchBox, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_view->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionToggleSearchBox);

    // Metadata editor styling menus (inserted on the toolbar)
    setupMetadataEditorsStylingMenu();
}

void CorpusExplorerTreeWidget::setupMetadataEditorsStylingMenu()
{
    Command *command {nullptr};
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    d->menuMetadataEditorStyles = new QMenu(this);

    d->actionMetadataEditorPrimaryStyleTree = new QAction(tr("Primary Metadata Editor: Tree View"), this);
    connect(d->actionMetadataEditorPrimaryStyleTree, &QAction::triggered, this, &CorpusExplorerTreeWidget::metadataEditorPrimaryStyleTree);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.PrimaryMetadataEditor.Tree", d->actionMetadataEditorPrimaryStyleTree, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->menuMetadataEditorStyles->addAction(d->actionMetadataEditorPrimaryStyleTree);

    d->actionMetadataEditorPrimaryStyleGroupBox = new QAction(tr("Primary Metadata Editor: Form View"), this);
    connect(d->actionMetadataEditorPrimaryStyleGroupBox, &QAction::triggered, this, &CorpusExplorerTreeWidget::metadataEditorPrimaryStyleGroupBox);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.PrimaryMetadataEditor.GroupBox", d->actionMetadataEditorPrimaryStyleGroupBox, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->menuMetadataEditorStyles->addAction(d->actionMetadataEditorPrimaryStyleGroupBox);

    d->actionMetadataEditorPrimaryStyleButton = new QAction(tr("Primary Metadata Editor: Compact View"), this);
    connect(d->actionMetadataEditorPrimaryStyleButton, &QAction::triggered, this, &CorpusExplorerTreeWidget::metadataEditorPrimaryStyleButton);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.PrimaryMetadataEditor.Button", d->actionMetadataEditorPrimaryStyleButton, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->menuMetadataEditorStyles->addAction(d->actionMetadataEditorPrimaryStyleButton);

    d->menuMetadataEditorStyles->addSeparator();

    d->actionMetadataEditorSecondaryStyleTree = new QAction(tr("Secondary Metadata Editor: Tree View"), this);
    connect(d->actionMetadataEditorSecondaryStyleTree, &QAction::triggered, this, &CorpusExplorerTreeWidget::metadataEditorSecondaryStyleTree);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.SecondaryMetadataEditor.Tree", d->actionMetadataEditorSecondaryStyleTree, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->menuMetadataEditorStyles->addAction(d->actionMetadataEditorSecondaryStyleTree);

    d->actionMetadataEditorSecondaryStyleGroupBox = new QAction(tr("Secondary Metadata Editor: Form View"), this);
    connect(d->actionMetadataEditorSecondaryStyleGroupBox, &QAction::triggered, this, &CorpusExplorerTreeWidget::metadataEditorSecondaryStyleGroupBox);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.SecondaryMetadataEditor.GroupBox", d->actionMetadataEditorSecondaryStyleGroupBox, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->menuMetadataEditorStyles->addAction(d->actionMetadataEditorSecondaryStyleGroupBox);

    d->actionMetadataEditorSecondaryStyleButton = new QAction(tr("Secondary Metadata Editor: Compact View"), this);
    connect(d->actionMetadataEditorSecondaryStyleButton, &QAction::triggered, this, &CorpusExplorerTreeWidget::metadataEditorSecondaryStyleButton);
    command = ACTION_MANAGER->registerAction("Corpus.Explorer.SecondaryMetadataEditor.Button", d->actionMetadataEditorSecondaryStyleButton, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    d->menuMetadataEditorStyles->addAction(d->actionMetadataEditorSecondaryStyleButton);

    // Add the pop-up menu to the corpus explorer toolbar
    QToolButton* toolButtonMetadataEditorStyles = new QToolButton();
    toolButtonMetadataEditorStyles->setMenu(d->menuMetadataEditorStyles);
    toolButtonMetadataEditorStyles->setIcon(QIcon(":icons/corpusexplorer/corpus_explorer_syle.png"));
    toolButtonMetadataEditorStyles->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolButtonMetadataEditorStyles->setPopupMode(QToolButton::InstantPopup);
    QWidgetAction* toolButtonActionMetadataEditorStyles = new QWidgetAction(this);
    toolButtonActionMetadataEditorStyles->setDefaultWidget(toolButtonMetadataEditorStyles);

    d->toolbarCorpusExplorer->addAction(toolButtonActionMetadataEditorStyles);
}

// ==============================================================================================================================
// Respond to changes in the corpus repositories manager (opening and closing corpus repositories)
// ==============================================================================================================================
void CorpusExplorerTreeWidget::corpusRepositoryAdded(const QString &repositoryID)
{
    if (!d->corpusRepositoriesManager) return;
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->corpusRepositoryByID(repositoryID);
    if (!repository) return;
    CorpusObserver *obs = new CorpusObserver(repository);
    OBJECT_MANAGER->registerObject(obs, QtilitiesCategory("CorpusObserver"));
    d->observersForCorpusRepositories.insert(repositoryID, obs);
    d->corporaTopLevelNode->addNode(obs->nodeRepository());
    // If there is only one corpus in one repository, open it
    QStringList corpusIDs = d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID);
    if (corpusIDs.count() == 1) {
        QPointer<Corpus> corpus = d->corpusRepositoriesManager->getCorpus(repositoryID, corpusIDs.constFirst());
        if (corpus) d->activeCorpus = corpus;
    }
}

void CorpusExplorerTreeWidget::corpusRepositoryRemoved(const QString &repositoryID)
{
    if (!d->corpusRepositoriesManager) return;
    d->corporaTopLevelNode->removeItem(repositoryID);
    CorpusObserver *obs = d->observersForCorpusRepositories.value(repositoryID);
    if (obs) {
        OBJECT_MANAGER->removeObject(obs);
        delete obs;
    }
    // Metadata editors
    d->metadataEditorPrimary->clear();
    d->metadataEditorSecondary->clear();
    d->preview->clear();
}

QList<CorpusObject *> CorpusExplorerTreeWidget::selectedCorpusItems()
{
    QList<CorpusObject *> selected;
    foreach (QObject *obj, d->corporaObserverWidget->selectedObjects()) {
        CorpusExplorerTreeNodeCorpus *nodeCorpus = nullptr;
        nodeCorpus = qobject_cast<CorpusExplorerTreeNodeCorpus *>(obj);
        if (nodeCorpus && nodeCorpus->corpus()) { selected << nodeCorpus->corpus(); continue; }
        CorpusExplorerTreeNodeCommunication *nodeCom = nullptr;
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
        if (nodeCom && nodeCom->communication) { selected << nodeCom->communication; continue; }
        CorpusExplorerTreeNodeSpeaker *nodeSpk = nullptr;
        nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
        if (nodeSpk && nodeSpk->speaker) { selected << nodeSpk->speaker; continue; }
        CorpusExplorerTreeNodeRecording *nodeRec = nullptr;
        nodeRec = qobject_cast<CorpusExplorerTreeNodeRecording *>(obj);
        if (nodeRec && nodeRec->recording) { selected << nodeRec->recording; continue; }
        CorpusExplorerTreeNodeAnnotation *nodeAnnot = nullptr;
        nodeAnnot = qobject_cast<CorpusExplorerTreeNodeAnnotation *>(obj);
        if (nodeAnnot && nodeAnnot->annotation) { selected << nodeAnnot->annotation; continue; }
    }
    return selected;
}

// ==============================================================================================================================
// Update metadata editors when the user selects a communication or speaker
// ==============================================================================================================================

void CorpusExplorerTreeWidget::updateMetadataEditorsForCom(CorpusCommunication *communication)
{
    if (!communication) return;
    if (!communication->repository()) return;
    if (!communication->corpus()) return;
    // Primary Editor: Communication (+ Recordings, Annotations)
    // Secondary Editor: Participations + Speaker
    QList<QPointer<CorpusObject> > itemsMain;
    itemsMain << static_cast<CorpusObject *>(communication);
    foreach(CorpusRecording *rec, communication->recordings())
        if (rec) itemsMain << static_cast<CorpusObject *>(rec);
    foreach(CorpusAnnotation *annot, communication->annotations())
        if (annot) itemsMain << static_cast<CorpusObject *>(annot);
    d->metadataEditorPrimary->rebind(communication->repository()->metadataStructure(), itemsMain);
    // Speakers participating in Communication
    QList<QPointer<CorpusObject> > itemsSecondary;
    foreach (CorpusParticipation *part, communication->corpus()->participationsForCommunication(communication->ID()))
        if (part) itemsSecondary << static_cast<CorpusObject *>(part);
    d->metadataEditorSecondary->rebind(communication->repository()->metadataStructure(), itemsSecondary, true, false);
}

void CorpusExplorerTreeWidget::updateMetadataEditorsForSpk(CorpusSpeaker *speaker)
{
    if (!speaker) return;
    if (!speaker->repository()) return;
    if (!speaker->corpus()) return;
    // Primary Editor: Speaker
    // Secondary Editor: Participations + Communications
    QList<QPointer<CorpusObject> > itemsMain;
    itemsMain << static_cast<CorpusObject *>(speaker);
    d->metadataEditorPrimary->rebind(speaker->repository()->metadataStructure(), itemsMain);
    // Communication where Speaker participates
    QList<QPointer<CorpusObject> > itemsSecondary;
    foreach (CorpusParticipation *part, speaker->corpus()->participationsForSpeaker(speaker->ID()))
        if (part) itemsSecondary << static_cast<CorpusObject *>(part);
    d->metadataEditorSecondary->rebind(speaker->repository()->metadataStructure(), itemsSecondary, false, true);
}

void CorpusExplorerTreeWidget::updateMetadataEditorsForCorpus(Corpus *corpus)
{
    if (!corpus) return;
    if (!corpus->repository()) return;
    // Primary Editor: Corpus - Secondary Editor: None, show corpus statistics widget
    QList<QPointer<CorpusObject> > itemsMain;
    itemsMain << static_cast<CorpusObject *>(corpus);
    d->metadataEditorPrimary->rebind(corpus->repository()->metadataStructure(), itemsMain);
    QList<QPointer<CorpusObject> > itemsSecondary;
    d->metadataEditorSecondary->rebind(corpus->repository()->metadataStructure(), itemsSecondary, false, false);
}

void CorpusExplorerTreeWidget::corporaObserverWidgetSelectedObjectsChanged(QList<QObject*> selected)
{
    if (selected.isEmpty()) {
        if (d->preview) d->preview->openCommunication(nullptr);
        return;
    }
    QObject *obj = selected.first();
    CorpusExplorerTreeNodeCorpus *nodeCorpus = qobject_cast<CorpusExplorerTreeNodeCorpus *>(obj);
    if (nodeCorpus && nodeCorpus->corpus()) {
        d->activeCorpus = nodeCorpus->corpus();
        updateMetadataEditorsForCorpus(nodeCorpus->corpus());
        return;
    }
    CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
    if (nodeCom && nodeCom->communication) {
        d->activeCorpus = nodeCom->communication->corpus();
        updateMetadataEditorsForCom(nodeCom->communication);
        d->preview->openCommunication(nodeCom->communication);
        return;
    }
    CorpusExplorerTreeNodeSpeaker *nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
    if (nodeSpk && nodeSpk->speaker) {
        d->activeCorpus = nodeSpk->speaker->corpus();
        updateMetadataEditorsForSpk(nodeSpk->speaker);
        return;
    }
    CorpusExplorerTreeNodeRecording *nodeRec = qobject_cast<CorpusExplorerTreeNodeRecording *>(obj);
    if (nodeRec && nodeRec->recording) {
        d->activeCorpus = nodeRec->recording->corpus();
        CorpusCommunication *communication = qobject_cast<CorpusCommunication *>(nodeRec->recording->parent());
        updateMetadataEditorsForCom(communication);
        d->preview->openCommunication(communication);
        return;
    }
    CorpusExplorerTreeNodeAnnotation *nodeAnnot = qobject_cast<CorpusExplorerTreeNodeAnnotation *>(obj);
    if (nodeAnnot && nodeAnnot->annotation) {
        d->activeCorpus = nodeAnnot->annotation->corpus();
        CorpusCommunication *communication = qobject_cast<CorpusCommunication *>(nodeAnnot->annotation->parent());
        updateMetadataEditorsForCom(communication);
        d->preview->openCommunication(communication);
        return;
    }
}

void CorpusExplorerTreeWidget::corporaObserverWidgetDoubleClickRequest(QObject *object, Observer* parent_observer)
{
    CorpusExplorerTreeNodeCorpus *nodeCorpus = qobject_cast<CorpusExplorerTreeNodeCorpus *>(object);
    if (nodeCorpus && (!nodeCorpus->corpus())) {
        // It is a corpus but not opened yet.
        QString repositoryID = parent_observer->observerName();
        QString corpusID = nodeCorpus->getName();
        d->corpusRepositoriesManager->getCorpus(repositoryID, corpusID);
    }
}

bool CorpusExplorerTreeWidget::checkForActiveCorpus()
{
    if (!d->activeCorpus) {
        QMessageBox::warning(this, tr("No Corpus Selected"),  tr("Please open a Corpus first."), QMessageBox::Ok);
        return false;
    }
    return true;
}

// =========================================================================================================================================
// Corpora
// =========================================================================================================================================

void CorpusExplorerTreeWidget::saveCorpusMetadata()
{
    if (d->activeCorpus) d->activeCorpus->save();
}

void CorpusExplorerTreeWidget::createCorpus()
{
    if (!d->corpusRepositoriesManager) return;
    QString repositoryID = d->corpusRepositoriesManager->activeCorpusRepositoryID();
    if (repositoryID.isEmpty()) {
        // Create a corpus repository first
        if (d->widgetCorpusMode) d->widgetCorpusMode->newCorpusRepository();
        return;
    }
    bool ok {false};
    QString corpusID = QInputDialog::getText(this, tr("Add New Corpus"),
                                             tr("Corpus ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || corpusID.isEmpty()) return;
    Corpus *corpus = d->corpusRepositoriesManager->createCorpus(corpusID, repositoryID);
    if (corpus) d->activeCorpus = corpus;
}

void CorpusExplorerTreeWidget::openCorpus()
{
    if (!d->corpusRepositoriesManager) return;
    QString repositoryID = d->corpusRepositoriesManager->activeCorpusRepositoryID();
    if (repositoryID.isEmpty()) {
        // Open a corpus repository first
        if (d->widgetCorpusMode) d->widgetCorpusMode->openCorpusRepository();
        return;
    }
    QInputDialog dialog;
    dialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
    dialog.setComboBoxItems(d->corpusRepositoriesManager->listAvailableCorpusIDs(repositoryID));
    dialog.setWindowTitle("Select the corpus to open");
    if (dialog.exec()) {
       QString corpusID = dialog.textValue();
       Corpus *corpus = d->corpusRepositoriesManager->getCorpus(repositoryID, corpusID);
       if (corpus) d->activeCorpus = corpus;
    }
}

void CorpusExplorerTreeWidget::deleteCorpus()
{
    if (!d->corpusRepositoriesManager) return;
    if (!d->activeCorpus) {
        QMessageBox::warning(this, tr("Delete Corpus"), tr("Please select the Corpus that you want to delete."),
                             QMessageBox::Ok);
        return;
    }
    bool alsoDeleteData = false;
    if (QMessageBox::warning(this, tr("Delete Corpus?"),
                             QString(tr("Do you want to permanently delete Corpus %1?"))
                             .arg(d->activeCorpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    if (QMessageBox::warning(this, tr("Permanently delete data?"),
                             QString(tr("Do you also want to delete all the annotation data of Corpus %1?"))
                             .arg(d->activeCorpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        alsoDeleteData = true;
    }
    if (alsoDeleteData) {
        foreach (CorpusCommunication *com, d->activeCorpus->communications()) {
            if (!com) continue;
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (!annot) continue;
                d->activeCorpus->repository()->annotations()->deleteAllTiersAllSpeakers(annot->ID());
            }
        }
    }
    QString corpusID = d->activeCorpus->ID();
    if (!d->activeCorpus->repository()->metadata()->deleteCorpus(corpusID)) {
        return;
    }
    d->corpusRepositoriesManager->removeCorpus(d->activeCorpus->repository()->ID(), corpusID);
    d->activeCorpus = nullptr;
}

// =========================================================================================================================================
// Add and remove corpus objects
// =========================================================================================================================================

void CorpusExplorerTreeWidget::addCommunication()
{
    if (!d->activeCorpus) {
        QMessageBox::warning(this, tr("Add Communication to Corpus"),
                             tr("Please select the Corpus to which the Communication will be added."),
                             QMessageBox::Ok);
        return;
    }
    if (!d->activeCorpus->repository()) return;
    AddCorpusCommunicationDialog *dialog = new AddCorpusCommunicationDialog(this);

    dialog->setCorpusID(d->activeCorpus->corpusID());
    dialog->exec();
    if (dialog->result() == QDialog::Rejected) return;
    QString communicationID = dialog->communicationID();
    // = QInputDialog::getText(this, tr("Add new Communication"),
    //                       tr("Communication ID:"), QLineEdit::Normal, "", &ok);
    if (communicationID.isEmpty()) {
        QMessageBox::warning(this, tr("Add Communication to Corpus"), tr("The Communication ID cannot be empty."), QMessageBox::Ok);
        return;
    }
    d->activeCorpus->addCommunication(new CorpusCommunication(communicationID, d->activeCorpus->repository(), d->activeCorpus));
}

void CorpusExplorerTreeWidget::addSpeaker()
{
    if (!d->activeCorpus) {
        QMessageBox::warning(this, tr("Add Speaker to Corpus"),
                             tr("Please select the Corpus to which the Speaker will be added."),
                             QMessageBox::Ok);
        return;
    }
    if (!d->activeCorpus->repository()) return;
    bool ok(false);
    QString speakerID = QInputDialog::getText(this, tr("Add new Speaker"),
                                                    tr("Speaker ID:"), QLineEdit::Normal, "", &ok);
    if (!ok) return;
    if (speakerID.isEmpty()) {
        QMessageBox::warning(this, tr("Add Speaker to Corpus"), tr("The Speaker ID cannot be empty."), QMessageBox::Ok);
        return;
    }
    d->activeCorpus->addSpeaker(new CorpusSpeaker(speakerID, d->activeCorpus->repository(), d->activeCorpus));
}

void CorpusExplorerTreeWidget::addRecording()
{
    // Check that there is an active corpus
    if (!d->activeCorpus) {
        QMessageBox::warning(this, tr("Add Recording"),
                             tr("Please select or create the Corpus to which the Recording(s) will be added."),
                             QMessageBox::Ok);
        return;
    }
    if (!d->activeCorpus->repository()) return;
    // Check to see whether there is a selected Communication
    CorpusExplorerTreeNodeCommunication *nodeCom = nullptr;
    if (d->corporaObserverWidget->selectedObjects().count() == 1) {
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(d->corporaObserverWidget->selectedObjects().constFirst());
    }
    // Get the filenames of the recordings to add
    QFileDialog::Options options;
    QString selectedFilter;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add Media Recordings to Corpus"),
                            d->activeCorpus->repository()->files()->basePath(),
                            tr("Wave Files (*.wav);;MP3 Files (*.mp3);;All Files (*)"),
                            &selectedFilter, options);
    if (fileNames.count() == 0) return;
    // Process files to add
    foreach(QString fileName, fileNames) {
        QFileInfo info(fileName);
        if (info.suffix() == "wav" || info.suffix() == "mp3") {
            CorpusCommunication *com(nullptr);
            // Without a selected Communication => create a Communication with the same name
            if ((!nodeCom) || ((nodeCom) && (!nodeCom->communication))) {
                com = new CorpusCommunication(info.baseName(), d->activeCorpus->repository(), d->activeCorpus);
                d->activeCorpus->addCommunication(com);
            }
            else {
                // Otherwise, add the file to the selected Communication
                com = nodeCom->communication;
            }
            CorpusRecording *rec = new CorpusRecording(info.baseName(), d->activeCorpus->repository(), com);
            rec->setFilename(d->activeCorpus->repository()->files()->getRelativeToBasePath(fileName));
            com->addRecording(rec);
        }
    }
}

void CorpusExplorerTreeWidget::addAnnotation()
{
    CorpusExplorerTreeNodeCommunication *nodeCom = nullptr;
    if (d->corporaObserverWidget->selectedObjects().count() == 1) {
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(d->corporaObserverWidget->selectedObjects().constFirst());
    }
    if ((!nodeCom) || ((nodeCom) && (!nodeCom->communication))) {
        QMessageBox::warning(this, tr("Add Annotation to Communication"),
                             tr("Please select the corpus Communication to which the Annotation will be added."),
                             QMessageBox::Ok);
        return;
    }
    Corpus *corpus = qobject_cast<Corpus *>(nodeCom->communication->parent());
    if (!corpus) return;
    bool ok;
    QString annotationID = QInputDialog::getText(this, tr("Add new Annotation"),
                                                 tr("Annotation ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || annotationID.isEmpty()) return;
    CorpusAnnotation *annot = new CorpusAnnotation(annotationID,corpus->repository(), nodeCom->communication);
    if (nodeCom->communication->hasRecording(annot->ID()))
        annot->setRecordingID(annot->ID());
    nodeCom->communication->addAnnotation(annot);
}

void CorpusExplorerTreeWidget::addParticipation()
{
    CorpusExplorerTreeNodeCommunication *nodeCom {nullptr};
    CorpusExplorerTreeNodeSpeaker *nodeSpk {nullptr};
    if (d->corporaObserverWidget->selectedObjects().count() != 2) {
        QMessageBox::warning(this, tr("Add Participation"),
                             tr("Please select exactly one Communication and one Speaker belonging to the same corpus."),
                             QMessageBox::Ok);
        return;
    }
    nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(d->corporaObserverWidget->selectedObjects().constFirst());
    nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(d->corporaObserverWidget->selectedObjects().constLast());
    if ((!nodeCom) || (!nodeSpk)) {
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(d->corporaObserverWidget->selectedObjects().constLast());
        nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(d->corporaObserverWidget->selectedObjects().constFirst());
    }
    if ((!nodeCom) || ((nodeCom) && (!nodeCom->communication)) || (!nodeSpk) || ((nodeSpk) && (!nodeSpk->speaker))) {
        QMessageBox::warning(this, tr("Add Participation"),
                             tr("Please select exactly one Communication and one Speaker belonging to the same corpus."),
                             QMessageBox::Ok);
        return;
    }
    Corpus *corpusCom = qobject_cast<Corpus *>(nodeCom->communication->parent());
    Corpus *corpusSpk = qobject_cast<Corpus *>(nodeSpk->speaker->parent());
    if ((!corpusCom) || (!corpusSpk)) return;
    if (corpusCom != corpusSpk) {
        QMessageBox::warning(this, tr("Add Participation"),
                             tr("The Communication and the Speaker must belong to the same corpus."),
                             QMessageBox::Ok);
        return;
    }
    corpusCom->addParticipation(nodeCom->communication->ID(), nodeSpk->speaker->ID());
    d->corporaObserverWidget->selectObject(nodeCom);
}

void CorpusExplorerTreeWidget::removeCorpusItems()
{
    QList<CorpusObject *> selected = selectedCorpusItems();
    if (selected.count() == 0) return;
    if (selected.count() == 1) {
        CorpusCommunication *com = qobject_cast<CorpusCommunication *>(selected.first());
        if (com) {
            Corpus *corpus = qobject_cast<Corpus *>(com->parent());
            if (!corpus) return;
            bool alsoDeleteData = false;
            if (QMessageBox::warning(this, tr("Remove communication from corpus?"),
                                     QString(tr("Do you want to remove Communication %1 from Corpus %2?"))
                                     .arg(com->name(), corpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            if (QMessageBox::warning(this, tr("Permanently delete data?"),
                                     QString(tr("Do you also want to delete all the annotation data associated with Communication %1 from Corpus %2?"))
                                     .arg(com->name(), corpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                alsoDeleteData = true;
            }
            if (alsoDeleteData) {
                foreach (CorpusAnnotation *annot, com->annotations()) {
                    if (!annot) continue;
                    corpus->repository()->annotations()->deleteAllTiersAllSpeakers(annot->ID());
                }
            }
            corpus->removeCommunication(com->ID());
            return;
        }
        CorpusSpeaker *spk = qobject_cast<CorpusSpeaker *>(selected.first());
        if (spk) {
            Corpus *corpus = qobject_cast<Corpus *>(spk->parent());
            if (!corpus) return;
            if (QMessageBox::warning(this, tr("Remove speaker from corpus?"),
                                     QString(tr("Do you want to remove Speaker %1 from Corpus %2?"))
                                     .arg(spk->name(), corpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            corpus->removeSpeaker(spk->ID());
            return;
        }
        CorpusRecording *rec = qobject_cast<CorpusRecording *>(selected.first());
        if (rec) {
            CorpusCommunication *com = qobject_cast<CorpusCommunication *>(rec->parent());
            if (!com) return;
            if (QMessageBox::warning(this, tr("Remove recording from communication?"),
                                     QString(tr("Do you want to remove Recording %1 from Communication %2?"))
                                     .arg(rec->name(), rec->communicationID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            com->removeRecording(rec->ID());
            return;
        }
        CorpusAnnotation *annot = qobject_cast<CorpusAnnotation *>(selected.first());
        if (annot) {
            CorpusCommunication *com = qobject_cast<CorpusCommunication *>(annot->parent());
            if (!com) return;
            bool alsoDeleteData = false;
            if (QMessageBox::warning(this, tr("Remove annotation from communication?"),
                                     QString(tr("Do you want to remove Annotation %1 from Communication %2?"))
                                     .arg(annot->name(), annot->communicationID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            if (QMessageBox::warning(this, tr("Permanently delete data?"),
                                     QString(tr("Do you also want to delete all the annotation data associated with Annotation %1 in Communication %2?"))
                                     .arg(annot->name(), annot->communicationID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                alsoDeleteData = true;
            }
            if (alsoDeleteData) {
                Corpus *corpus = qobject_cast<Corpus *>(com->parent());
                if (corpus) corpus->repository()->annotations()->deleteAllTiersAllSpeakers(annot->ID());
            }
            com->removeAnnotation(annot->ID());
            return;
        }
    }
    else {
        // Show dialogue to confirm multiple corpus item deletion
        QPointer<QStandardItemModel> modelDel = new QStandardItemModel(this);
        modelDel->setColumnCount(2);
        modelDel->setRowCount(selected.count());
        modelDel->setHorizontalHeaderLabels(QStringList() << tr("ID") << tr("Type"));
        int i = 0;
        foreach (CorpusObject *cobj, selected) {
            QStandardItem *item = new QStandardItem(cobj->ID());
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            modelDel->setItem(i, 0, item);
            modelDel->setItem(i, 1, new QStandardItem(CorpusObject::typeToFriendlyString(cobj->type())));
            i++;
        }
        // Confirmations
        QPointer<SelectionDialog> sel = new SelectionDialog(tr("Confirm deletion of corpus items"), modelDel, this);
        if (sel->exec() == QDialog::Rejected)
            return;
        bool alsoDeleteData = false;
        if (QMessageBox::warning(this, tr("Permanently delete data?"),
                                 QString(tr("Do you also want to delete the associated annotation data?")),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            alsoDeleteData = true;
        }
        // delete those confirmed
        d->corporaTopLevelNode->startTreeProcessingCycle();
        for (int i = 0; i < modelDel->rowCount(); ++i) {
            QStandardItem *item = modelDel->item(i, 0);
            if (item->checkState() == Qt::Checked) {
                // delete corpus item at position i
                CorpusObject *cobj = selected.at(i);
                if (cobj->type() == CorpusObject::Type_Communication) {
                    Corpus *corpus = qobject_cast<Corpus *>(cobj->parent());
                    if (corpus) {
                        if (alsoDeleteData) {
                            CorpusCommunication *com = qobject_cast<CorpusCommunication *>(cobj);
                            if (com) {
                                foreach (CorpusAnnotation *annot, com->annotations()) {
                                    if (annot) corpus->repository()->annotations()->deleteAllTiersAllSpeakers(annot->ID());
                                }
                            }
                        }
                        corpus->removeCommunication(cobj->ID());
                    }
                }
                else if (cobj->type() == CorpusObject::Type_Speaker) {
                    Corpus *corpus = qobject_cast<Corpus *>(cobj->parent());
                    if (corpus) corpus->removeSpeaker(cobj->ID());
                }
                else if (cobj->type() == CorpusObject::Type_Recording) {
                    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(cobj->parent());
                    if (com) com->removeRecording(cobj->ID());
                }
                else if (cobj->type() == CorpusObject::Type_Annotation) {
                    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(cobj->parent());
                    if (alsoDeleteData) {
                        Corpus *corpus = qobject_cast<Corpus *>(com->parent());
                        if (corpus) corpus->repository()->annotations()->deleteAllTiersAllSpeakers(cobj->ID());
                    }
                    if (com) com->removeAnnotation(cobj->ID());
                }
            }
        }
        d->corporaTopLevelNode->endTreeProcessingCycle();
    }
}

void CorpusExplorerTreeWidget::relinkCorpusItem()
{
    if (!d->activeCorpus) {
        QMessageBox::warning(this, tr("No Corpus Selected"),  tr("Please open a Corpus first."), QMessageBox::Ok);
        return;
    }
    QList<CorpusObject *> selected = selectedCorpusItems();

    if (selected.count() < 1) {
        QMessageBox::warning(this, tr("Relink Corpus Items"),
                             tr("Please select Recording(s) and Annotation(s) to change their association with a Communication."),
                             QMessageBox::Ok);
        return;
    }
    // Get destination communication
    bool ok;
    QString communicationID = QInputDialog::getItem (this, tr("Relink Corpus Item"),
                                                    tr("Please select the ID of the Communication to which you want to move the selected Recording"),
                                                    d->activeCorpus->communicationIDs(), 0, false, &ok);
    if (!ok || communicationID.isEmpty()) return;
    // Relink
    foreach (CorpusObject *cobj, selected) {
        if (cobj->type() == CorpusObject::Type_Recording) {
            CorpusRecording *rec = qobject_cast<CorpusRecording *>(cobj);
            CorpusCommunication *com = d->activeCorpus->communication(rec->communicationID());
            if (!com) return;
//            com->unlinkRecording(rec->ID());
//            com = corpus->communication(communicationID);
//            com->addRecording(rec);
        }
        else if (cobj->type() == CorpusObject::Type_Annotation) {
            CorpusAnnotation *annot = qobject_cast<CorpusAnnotation *>(cobj);
            CorpusCommunication *com = d->activeCorpus->communication(annot->communicationID());
            if (!com) return;
//            com->unlinkAnnotation(annot->ID());
//            com = corpus->communication(communicationID);
//            com->addAnnotation(annot);
        }
    }
}

// ==============================================================================================================================
// Presentation
// ==============================================================================================================================

void CorpusExplorerTreeWidget::attributesAndGroupings()
{
    if (!d->corpusRepositoriesManager) return;
    QPointer<CorpusRepository> repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository) return;
    // Ask user for new groupings
    unique_ptr<CorpusExplorerOptionsDialog> dialog(new CorpusExplorerOptionsDialog(repository->metadataStructure(), this));
    if (dialog->exec() != QDialog::Accepted) return;
    // Apply to observer
    QPointer<CorpusObserver> obs = d->corpusRepositoriesManager->activeCorpusObserver();
    if (obs) {
        obs->setCommunicationsGrouping(dialog->groupAttributesForCommunications());
        obs->setSpeakersGrouping(dialog->groupAttributesForSpeakers());
    }
}

// ==============================================================================================================================
// Metadata editors (primary and secondary) styling
// ==============================================================================================================================

void CorpusExplorerTreeWidget::metadataEditorPrimaryStyleTree()
{
    d->metadataEditorPrimary->setEditorStyle(MetadataEditorWidget::TreeStyle);
}

void CorpusExplorerTreeWidget::metadataEditorPrimaryStyleGroupBox()
{
    d->metadataEditorPrimary->setEditorStyle(MetadataEditorWidget::GroupBoxStyle);
}

void CorpusExplorerTreeWidget::metadataEditorPrimaryStyleButton()
{
    d->metadataEditorPrimary->setEditorStyle(MetadataEditorWidget::ButtonStyle);
}

void CorpusExplorerTreeWidget::metadataEditorSecondaryStyleTree()
{
    d->metadataEditorSecondary->setEditorStyle(MetadataEditorWidget::TreeStyle);
}

void CorpusExplorerTreeWidget::metadataEditorSecondaryStyleGroupBox()
{
    d->metadataEditorSecondary->setEditorStyle(MetadataEditorWidget::GroupBoxStyle);
}

void CorpusExplorerTreeWidget::metadataEditorSecondaryStyleButton()
{
    d->metadataEditorSecondary->setEditorStyle(MetadataEditorWidget::ButtonStyle);
}
