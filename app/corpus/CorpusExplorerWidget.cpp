#include <QString>
#include <QList>
#include <QSet>
#include <QToolBar>
#include <QFileDialog>
#include <QMenuBar>
#include <QTreeView>
#include <QProgressDialog>

#include "CorpusExplorerWidget.h"
#include "ui_CorpusExplorerWidget.h"

#include <QtilitiesProjectManagement>
using namespace QtilitiesProjectManagement;

#include "svcore/base/RecentFiles.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/serialisers/CorpusDefinition.h"
#include "pngui/model/corpus/CorpusExplorerTreeModel.h"
#include "pngui/widgets/selectiondialog.h"
#include "pngui/widgets/metadataeditorwidget.h"
#include "pngui/observers/corpusobserver.h"
#include "CorporaManager.h"
#include "NewCorpusWizard.h"
#include "CorpusDatabaseConnectionDialog.h"
#include "AddNewCorpusItemDialog.h"
#include "CorpusExplorerOptionsDialog.h"
#include "CheckMediaFilesDialog.h"
#include "SplitCommunicationsDialog.h"
#include "importmetadatawizard/ImportMetadataWizard.h"
#include "exportmetadatawizard/ExportMetadataWizard.h"
#include "importcorpusitemswizard/ImportCorpusItemsWizard.h"
#include "exportannotationswizard/ExportAnnotationsWizard.h"

using std::vector;

struct CorpusExplorerWidgetData {
    CorpusExplorerWidgetData() :
        corporaManager(0), projectItem(0), corporaTopLevelNode(0), corporaObserverWidget(0),
        recentFilesMenu(0), recentFiles(0)
    { }

    CorporaManager *corporaManager;

    QAction *actionNewCorpus;
    QAction *actionOpenCorpusFile;
    QAction *actionOpenCorpusDbConnection;
    QAction *actionCloseCorpus;
    QAction *actionSaveCorpus;
    QAction *actionSaveCorpusAs;
    QAction *actionImportCorpus;
    QAction *actionAddCommunication;
    QAction *actionAddSpeaker;
    QAction *actionAddRecording;
    QAction *actionAddAnnotation;
    QAction *actionAddParticipation;
    QAction *actionRemoveCorpusItems;
    QAction *actionRelinkCorpusItem;
    QAction *actionAddItemsFromFolder;
    QAction *actionImportMetadata;
    QAction *actionExportMetadata;
    QAction *actionExportAnnotations;
    QAction *actionAttributesAndGroupings;
    QAction *actionCheckMediaFiles;
    QAction *actionCreateAnnotationsFromRecordings;
    QAction *actionCreateSpeakersFromAnnotations;
    QAction *actionSplitCommunications;
    QAction *actionCleanUpParticipations;

    RecentFiles *recentFiles;
    ActionContainer* recentFilesMenu;

    ObserverProjectItemWrapper* projectItem;

    QPointer<TreeNode> corporaTopLevelNode;
    ObserverWidget* corporaObserverWidget;
    MetadataEditorWidget *metadataEditorMain;
    MetadataEditorWidget *metadataEditorSecondary;

    QToolBar *toolbarCorpusExplorer;
};

CorpusExplorerWidget::CorpusExplorerWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CorpusExplorerWidget), d(new CorpusExplorerWidgetData)
{
    ui->setupUi(this);

    // Get Corpora Manager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorporaManager");
    foreach (QObject* obj, list) {
        CorporaManager *manager = qobject_cast<CorporaManager *>(obj);
        if (manager) d->corporaManager = manager;
    }

    d->corporaTopLevelNode = new TreeNode("Corpus Explorer");
    d->corporaTopLevelNode->enableNamingControl(ObserverHints::EditableNames, NamingPolicyFilter::ProhibitDuplicateNames,
                                                NamingPolicyFilter::AutoRename);
    d->corporaTopLevelNode->displayHints()->setActionHints(ObserverHints::ActionNoHints);
    d->corporaTopLevelNode->displayHints()->setDisplayFlagsHint(ObserverHints::NoDisplayFlagsHint);
    d->corporaTopLevelNode->displayHints()->setDragDropHint(ObserverHints::AllDragDrop);
    // Uncomment these to play around with root display formats.
    //d->corporaTopLevelNode->displayHints()->setRootIndexDisplayHint(ObserverHints::RootIndexDisplayUndecorated);
    //d->corporaTopLevelNode->displayHints()->setRootIndexDisplayHint(ObserverHints::RootIndexDisplayDecorated);
    d->corporaTopLevelNode->displayHints()->setRootIndexDisplayHint(ObserverHints::RootIndexHide); // This is the default

    OBJECT_MANAGER->registerObject(d->corporaTopLevelNode, QtilitiesCategory("Corpus"));

    // Project Item
    // d->projectItem = new ObserverProjectItemWrapper(d->top_level_node, this);

    // Recent files
    d->recentFiles = new RecentFiles("RecentCorpusFiles", 20);

    // Menu and Toolbar actions
    d->toolbarCorpusExplorer = new QToolBar("Corpus Explorer", this);
    d->toolbarCorpusExplorer->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setupActions();
    this->addToolBar(d->toolbarCorpusExplorer);

    // Set up corpora observer widget
    d->corporaObserverWidget = new ObserverWidget(Qtilities::TreeView);
    d->corporaObserverWidget->setRefreshMode(ObserverWidget::RefreshModeShowTree);
    d->corporaObserverWidget->setGlobalMetaType("Corpus Tree Meta Type");
    d->corporaObserverWidget->setAcceptDrops(true);
    connect(d->corporaObserverWidget, SIGNAL(selectedObjectsChanged(QList<QObject*>)), SLOT(selectionChanged(QList<QObject*>)));
    d->corporaObserverWidget->setObserverContext(d->corporaTopLevelNode);
    d->corporaObserverWidget->layout()->setMargin(0);
    d->corporaObserverWidget->initialize();

    // Create layout of the Corpus Explorer
    ui->gridLayout->setMargin(0);
    ui->dockCorpusExplorer->setWidget(d->corporaObserverWidget);
    d->metadataEditorMain = new MetadataEditorWidget(this);
    d->metadataEditorMain->layout()->setMargin(0);
    ui->dockMetadataPrimary->setWidget(d->metadataEditorMain);
    d->metadataEditorSecondary = new MetadataEditorWidget(this);
    d->metadataEditorSecondary->layout()->setMargin(0);
    ui->dockMetadataSecondary->setWidget(d->metadataEditorSecondary);
}

CorpusExplorerWidget::~CorpusExplorerWidget()
{
    delete ui;
    delete d;
}

void CorpusExplorerWidget::setupActions()
{
    // Context and menu bar
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    bool existed;
    Command* command;

    ActionContainer* file_menu = ACTION_MANAGER->menu(qti_action_FILE);
    Q_ASSERT(file_menu);
    ActionContainer* menu_bar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    if (!existed) menu_bar->addMenu(view_menu, qti_action_HELP);
    ActionContainer* corpus_menu = ACTION_MANAGER->createMenu("&Corpus", existed);
    if (!existed) menu_bar->addMenu(corpus_menu, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // FILE MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionNewCorpus = new QAction("New Corpus...", this);
    connect(d->actionNewCorpus, SIGNAL(triggered()), SLOT(newCorpus()));
    command = ACTION_MANAGER->registerAction("File.NewCorpus", d->actionNewCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionOpenCorpusFile = new QAction("Open Corpus...", this);
    connect(d->actionOpenCorpusFile, SIGNAL(triggered()), SLOT(openCorpusFile()));
    command = ACTION_MANAGER->registerAction("File.OpenCorpus", d->actionOpenCorpusFile, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionOpenCorpusDbConnection = new QAction("Open Corpus database connection...", this);
    connect(d->actionOpenCorpusDbConnection, SIGNAL(triggered()), SLOT(openCorpusDbConnection()));
    command = ACTION_MANAGER->registerAction("File.OpenCorpusDbConnection", d->actionOpenCorpusDbConnection, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->recentFilesMenu = ACTION_MANAGER->createMenu(tr("Open &Recent"), existed);
    if (!existed) file_menu->addMenu(d->recentFilesMenu, qti_action_FILE_SETTINGS);
    setupRecentFilesMenu();
    connect(d->recentFiles, SIGNAL(recentChanged()), this, SLOT(setupRecentFilesMenu()));

    file_menu->addSeperator(qti_action_FILE_SETTINGS);

    d->actionSaveCorpus = new QAction("Save Corpus", this);
    connect(d->actionSaveCorpus, SIGNAL(triggered()), SLOT(saveCorpus()));
    command = ACTION_MANAGER->registerAction("File.SaveCorpus", d->actionSaveCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionSaveCorpusAs = new QAction("Save Corpus as file...", this);
    connect(d->actionSaveCorpusAs, SIGNAL(triggered()), SLOT(saveCorpusAs()));
    command = ACTION_MANAGER->registerAction("File.SaveCorpusAs", d->actionSaveCorpusAs, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionCloseCorpus = new QAction("Close Corpus", this);
    connect(d->actionCloseCorpus, SIGNAL(triggered()), SLOT(closeCorpus()));
    command = ACTION_MANAGER->registerAction("File.CloseCorpus", d->actionCloseCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    file_menu->addSeperator(qti_action_FILE_SETTINGS);

    d->actionImportCorpus = new QAction("Import Corpus...", this);
    connect(d->actionImportCorpus, SIGNAL(triggered()), SLOT(importCorpus()));
    command = ACTION_MANAGER->registerAction("File.ImportCorpus", d->actionImportCorpus, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    file_menu->addSeperator(qti_action_FILE_SETTINGS);

    // ------------------------------------------------------------------------------------------------------
    // CORPUS MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionAddCommunication = new QAction(QIcon(":icons/actions/list_add.png"), "Add Communication...", this);
    connect(d->actionAddCommunication, SIGNAL(triggered()), SLOT(addCommunication()));
    command = ACTION_MANAGER->registerAction("Corpus.AddCommunication", d->actionAddCommunication, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddCommunication);

    d->actionAddSpeaker = new QAction(QIcon(":icons/actions/list_add.png"), "Add Speaker...", this);
    connect(d->actionAddSpeaker, SIGNAL(triggered()), SLOT(addSpeaker()));
    command = ACTION_MANAGER->registerAction("Corpus.AddSpeaker", d->actionAddSpeaker, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddSpeaker);

    d->actionAddRecording = new QAction(QIcon(":icons/actions/list_add.png"), "Add Media Recording...", this);
    connect(d->actionAddRecording, SIGNAL(triggered()), SLOT(addRecording()));
    command = ACTION_MANAGER->registerAction("Corpus.AddRecording", d->actionAddRecording, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddRecording);

    d->actionAddAnnotation = new QAction(QIcon(":icons/actions/list_add.png"), "Add Annotation...", this);
    connect(d->actionAddAnnotation, SIGNAL(triggered()), SLOT(addAnnotation()));
    command = ACTION_MANAGER->registerAction("Corpus.AddAnnotation", d->actionAddAnnotation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionAddAnnotation);

    d->actionRemoveCorpusItems = new QAction(QIcon(":icons/actions/list_remove.png"), "Remove Corpus Item(s)", this);
    connect(d->actionRemoveCorpusItems, SIGNAL(triggered()), SLOT(removeCorpusItems()));
    command = ACTION_MANAGER->registerAction("Corpus.RemoveCorpusItems", d->actionRemoveCorpusItems, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);
    d->toolbarCorpusExplorer->addAction(d->actionRemoveCorpusItems);

    d->actionRelinkCorpusItem = new QAction("Link Item to another Communication...", this);
    connect(d->actionRelinkCorpusItem, SIGNAL(triggered()), SLOT(relinkCorpusItem()));
    command = ACTION_MANAGER->registerAction("Corpus.RelinkCorpusItem", d->actionRelinkCorpusItem, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    corpus_menu->addSeperator();

    // Import - export functionality

    d->actionImportMetadata = new QAction("Import corpus metadata...", this);
    connect(d->actionImportMetadata, SIGNAL(triggered()), SLOT(importMetadata()));
    command = ACTION_MANAGER->registerAction("Corpus.ImportMetadata", d->actionImportMetadata, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionExportMetadata = new QAction("Export corpus metadata...", this);
    connect(d->actionExportMetadata, SIGNAL(triggered()), SLOT(exportMetadata()));
    command = ACTION_MANAGER->registerAction("Corpus.ExportMetadata", d->actionExportMetadata, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionAddItemsFromFolder = new QAction("Add corpus items from folder...", this);
    connect(d->actionAddItemsFromFolder, SIGNAL(triggered()), SLOT(addItemsFromFolder()));
    command = ACTION_MANAGER->registerAction("Corpus.AddItemsFromFolder", d->actionAddItemsFromFolder, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionExportAnnotations = new QAction("Export annotations...", this);
    connect(d->actionExportAnnotations, SIGNAL(triggered()), SLOT(exportAnnotations()));
    command = ACTION_MANAGER->registerAction("Corpus.ExportAnnotations", d->actionExportAnnotations, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    corpus_menu->addSeperator();

    d->actionCheckMediaFiles = new QAction("Check files of Media Recordings...", this);
    connect(d->actionCheckMediaFiles, SIGNAL(triggered()), SLOT(checkMediaFiles()));
    command = ACTION_MANAGER->registerAction("Corpus.CheckMediaFiles", d->actionCheckMediaFiles, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionSplitCommunications = new QAction("Split Communications based on annotation...", this);
    connect(d->actionSplitCommunications, SIGNAL(triggered()), SLOT(splitCommunications()));
    command = ACTION_MANAGER->registerAction("Corpus.SplitCommunications", d->actionSplitCommunications, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionCreateAnnotationsFromRecordings = new QAction("Create Annotations for Recordings not having one...", this);
    connect(d->actionCreateAnnotationsFromRecordings, SIGNAL(triggered()), SLOT(createAnnotationsFromRecordings()));
    command = ACTION_MANAGER->registerAction("Corpus.AnnotationsFromRecordings", d->actionCreateAnnotationsFromRecordings, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionCreateSpeakersFromAnnotations = new QAction("Create Speakers and Participations from Annotations...", this);
    connect(d->actionCreateSpeakersFromAnnotations, SIGNAL(triggered()), SLOT(createSpeakersFromAnnotations()));
    command = ACTION_MANAGER->registerAction("Corpus.SpeakersParticipationsFromAnnotations", d->actionCreateSpeakersFromAnnotations, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    d->actionCleanUpParticipations = new QAction("Clean-up Participations based on Annotations...", this);
    connect(d->actionCleanUpParticipations, SIGNAL(triggered()), SLOT(cleanUpParticipationsFromAnnotations()));
    command = ACTION_MANAGER->registerAction("Corpus.CleanUpParticipationsFromAnnotations", d->actionCleanUpParticipations, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    corpus_menu->addAction(command);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionAttributesAndGroupings = new QAction("Select Attributes and Groupings", this);
    connect(d->actionAttributesAndGroupings, SIGNAL(triggered()), SLOT(attributesAndGroupings()));
    command = ACTION_MANAGER->registerAction("Corpus.AttributesAndGroupings", d->actionAttributesAndGroupings, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);
}

void CorpusExplorerWidget::setupRecentFilesMenu()
{
    d->recentFilesMenu->menu()->clear();
    vector<QString> files = d->recentFiles->getRecent();
    for (size_t i = 0; i < files.size(); ++i) {
        QAction *action = new QAction(files[i], this);
        connect(action, SIGNAL(triggered()), this, SLOT(openCorpusRecentFile()));
        if (i == 0) {
            action->setShortcut(tr("Ctrl+R"));
        }
        d->recentFilesMenu->menu()->addAction(action);
    }
}

void CorpusExplorerWidget::updateMetadataEditorsForCom(CorpusCommunication *communication)
{
    if (!communication) return;
    // Primary Editor: Communication (+ Recordings, Annotations)
    // Secondary Editor: Participations + Speaker
    Corpus *corpus = qobject_cast<Corpus *>(communication->parent());
    if (!corpus) return;
    QList<QPointer<CorpusObject> > itemsMain;
    itemsMain << static_cast<CorpusObject *>(communication);
    foreach(QPointer<CorpusRecording> rec, communication->recordings())
        if (rec) itemsMain << static_cast<CorpusObject *>(rec);
    foreach(QPointer<CorpusAnnotation> annot, communication->annotations())
        if (annot) itemsMain << static_cast<CorpusObject *>(annot);
    d->metadataEditorMain->rebind(corpus->metadataStructure(), itemsMain);
    // Speakers participating in Communication
    QList<QPointer<CorpusObject> > itemsSecondary;
    foreach (QPointer<CorpusParticipation> part, corpus->participationsForCommunication(communication->ID()))
        if (part) itemsSecondary << static_cast<CorpusObject *>(part);
    d->metadataEditorSecondary->rebind(corpus->metadataStructure(), itemsSecondary, true, false);
}

void CorpusExplorerWidget::updateMetadataEditorsForSpk(CorpusSpeaker *speaker)
{
    if (!speaker) return;
    // Primary Editor: Speaker
    // Secondary Editor: Participations + Communications
    Corpus *corpus = qobject_cast<Corpus *>(speaker->parent());
    if (!corpus) return;
    QList<QPointer<CorpusObject> > itemsMain;
    itemsMain << static_cast<CorpusObject *>(speaker);
    d->metadataEditorMain->rebind(corpus->metadataStructure(), itemsMain);
    // Communication where Speaker participates
    QList<QPointer<CorpusObject> > itemsSecondary;
    foreach (QPointer<CorpusParticipation> part, corpus->participationsForSpeaker(speaker->ID()))
        if (part) itemsSecondary << static_cast<CorpusObject *>(part);
    d->metadataEditorSecondary->rebind(corpus->metadataStructure(), itemsSecondary, false, true);
}

void CorpusExplorerWidget::selectionChanged(QList<QObject*> selected)
{
    if (selected.isEmpty()) return;
    QObject *obj = selected.first();
    CorpusExplorerTreeNodeCommunication *nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
    if (nodeCom && nodeCom->communication) {
        d->corporaManager->setActiveCorpus(nodeCom->communication->corpusID());
        updateMetadataEditorsForCom(nodeCom->communication);
        return;
    }
    CorpusExplorerTreeNodeSpeaker *nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
    if (nodeSpk && nodeSpk->speaker) {
        d->corporaManager->setActiveCorpus(nodeSpk->speaker->corpusID());
        updateMetadataEditorsForSpk(nodeSpk->speaker);
        return;
    }
    CorpusExplorerTreeNodeRecording *nodeRec = qobject_cast<CorpusExplorerTreeNodeRecording *>(obj);
    if (nodeRec && nodeRec->recording) {
        d->corporaManager->setActiveCorpus(nodeRec->recording->corpusID());
        CorpusCommunication *communication = qobject_cast<CorpusCommunication *>(nodeRec->recording->parent());
        updateMetadataEditorsForCom(communication);
        return;
    }
    CorpusExplorerTreeNodeAnnotation *nodeAnnot = qobject_cast<CorpusExplorerTreeNodeAnnotation *>(obj);
    if (nodeAnnot && nodeAnnot->annotation) {
        d->corporaManager->setActiveCorpus(nodeAnnot->annotation->corpusID());
        CorpusCommunication *communication = qobject_cast<CorpusCommunication *>(nodeAnnot->annotation->parent());
        updateMetadataEditorsForCom(communication);
        return;
    }
}

// =========================================================================================================================================

void CorpusExplorerWidget::newCorpus()
{
    NewCorpusWizard *wizard = new NewCorpusWizard(this);
    wizard->exec();
    Corpus *corpus = wizard->newCorpus();
    if (!corpus) return;
    CorpusObserver *cobs = new CorpusObserver(corpus);
    cobs->setDefinition(wizard->newDefinition());
    OBJECT_MANAGER->registerObject(cobs, QtilitiesCategory("Corpus"));
    d->corporaTopLevelNode->addNode(cobs->nodeCorpus());
    d->corporaManager->addCorpus(corpus);
    d->corporaManager->setActiveCorpus(corpus->ID());
}

void CorpusExplorerWidget::openCorpusFile()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Corpus"), "",
                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    openCorpusFile(filename);
}

void CorpusExplorerWidget::openCorpusRecentFile()
{
    QObject *obj = sender();
    QAction *action = dynamic_cast<QAction *>(obj);
    if (!action) {
        qDebug() << "WARNING: openRecentFile: sender is not an action" << endl;
        return;
    }
    QString filename = action->text();
    if (filename.isEmpty()) return;
    openCorpusFile(filename);
}

Corpus *CorpusExplorerWidget::openCorpus(const QString &filename, CorpusDefinition &definition)
{
    // Open the corpus definition
    if (!definition.load(filename)) {
        QMessageBox::warning(this, "Cannot open corpus",
                             QString("Cannot open corpus definition file (%1). Is it a valid Praaline XML corpus definition?").arg(filename), QMessageBox::Ok);
        return 0;
    }
    // Ask for password, if needed
    QString password;
    if (definition.datastoreMetadata.usePassword == true || definition.datastoreAnnotations.usePassword) {
        password = QInputDialog::getText(this, "Enter password", "Enter password to connect to this corpus",
                                         QLineEdit::Password);
        if (definition.datastoreMetadata.usePassword) definition.datastoreMetadata.password = password;
        if (definition.datastoreAnnotations.usePassword) definition.datastoreAnnotations.password = password;
    }
    // Attempt to open corpus
    QString errorMessages;
    Corpus *corpus = Corpus::open(definition, errorMessages);
    if (!errorMessages.isEmpty()) {
        QMessageBox::warning(this, "Error opening corpus", errorMessages, QMessageBox::Ok);
        return 0;
    }
    return corpus;
}

void CorpusExplorerWidget::openCorpusFile(const QString &filename)
{
    // Check whether the corpus is already open
    QList<QObject *> listCorpora;
    listCorpora = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
    int countCorpora = 0;
    foreach (QObject* obj, listCorpora) {
        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
        if (obs) {
            countCorpora++;
            if (obs->definition().filenameDefinition == filename) {
                QMessageBox::warning(this, "Corpus already open",
                                     QString("This corpus (%1) is already open.").arg(filename), QMessageBox::Ok);
                return;
            }
        }
    }
    // Attempt to open corpus
    CorpusDefinition definition;
    Corpus *corpus = openCorpus(filename, definition);
    if (!corpus) return;
    // Register corpus with the Corpus Manager and global object pool
    CorpusObserver *cobs = new CorpusObserver(corpus);
    cobs->setDefinition(definition);
    OBJECT_MANAGER->registerObject(cobs, QtilitiesCategory("Corpus"));
    d->corporaTopLevelNode->addNode(cobs->nodeCorpus());
    d->corporaManager->addCorpus(corpus);
    d->corporaManager->setActiveCorpus(corpus->ID());
    //cobs->setCommunicationsGrouping(QStringList() << "type");
    d->recentFiles->addFile(filename);
}

void CorpusExplorerWidget::openCorpusDbConnection()
{
    QPointer<CorpusDatabaseConnectionDialog> dialog = new CorpusDatabaseConnectionDialog(this);
    dialog->exec();
    CorpusDefinition definition = dialog->corpusDefinition();
    QString errorMessages;
    Corpus *corpus = Corpus::open(definition, errorMessages);
    if (!errorMessages.isEmpty()) {
        QMessageBox::warning(this, "Error opening corpus", errorMessages, QMessageBox::Ok);
        return;
    }
    if (!corpus) return;
    // Register corpus with the Corpus Manager and global object pool
    CorpusObserver *cobs = new CorpusObserver(corpus);
    cobs->setDefinition(definition);
    OBJECT_MANAGER->registerObject(cobs, QtilitiesCategory("Corpus"));
    d->corporaTopLevelNode->addNode(cobs->nodeCorpus());
    d->corporaManager->addCorpus(corpus);
    d->corporaManager->setActiveCorpus(corpus->ID());
}

void CorpusExplorerWidget::closeCorpus()
{
    // Corpus Explorer observer
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    obs->corpus()->close();
    d->corporaTopLevelNode->removeItem(d->corporaManager->activeCorpusID());
    d->corporaManager->removeCorpus(d->corporaManager->activeCorpusID());
    OBJECT_MANAGER->removeObject(obs);
    delete obs;
    // Metadata editors
    d->metadataEditorMain->clear();
    d->metadataEditorSecondary->clear();
    // Activate next corpus (if available)
    QList<QObject *> listCorpora = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
    foreach (QObject* obj, listCorpora) {
        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
        if (obs && obs->corpus()) {
            d->corporaManager->setActiveCorpus(obs->corpus()->ID());
            break;
        }
    }
}

void CorpusExplorerWidget::saveCorpus()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    if (!obs->definition().filenameDefinition.isEmpty()) {
        obs->corpus()->save();
    } else {
        // database corpus
        saveCorpusAs();
    }
}

void CorpusExplorerWidget::saveCorpusAs()
{
    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    if (!obs->corpus()) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Corpus As..."), obs->definition().filenameDefinition,
                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"), &selectedFilter, options);
    if (filename.isEmpty()) return;
    obs->definition().save(filename);
    obs->corpus()->save();
    // else show error message
    // m_mainStatusBar->showMessage("Saved corpus \"" + fileName, 5000);
}

void CorpusExplorerWidget::importCorpus()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    // Find corpus to import
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Corpus"), "",
                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    CorpusDefinition def;
    Corpus *corpusSource = openCorpus(filename, def);
    if (!corpusSource) return;
    // Ask for prefix
    bool ok;
    QString prefix = QInputDialog::getText(this, tr("Imported corpus prefix"),
                                           tr("Prefix (optional):"), QLineEdit::Normal, "", &ok);
    if (!ok) return;
    // Import
    corpus->importCorpus(corpusSource, prefix);
}

// =========================================================================================================================================

void CorpusExplorerWidget::addCommunication()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    bool ok;
    QString communicationID = QInputDialog::getText(this, tr("Add new Communication"),
                                                    tr("Communication ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || communicationID.isEmpty()) return;
    corpus->addCommunication(new CorpusCommunication(communicationID));
}

void CorpusExplorerWidget::addSpeaker()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    bool ok;
    QString speakerID = QInputDialog::getText(this, tr("Add new Speaker"),
                                                    tr("Speaker ID:"), QLineEdit::Normal, "", &ok);
    if (!ok || speakerID.isEmpty()) return;
    corpus->addSpeaker(new CorpusSpeaker(speakerID));
}

void CorpusExplorerWidget::addRecording()
{
    CorpusExplorerTreeNodeCommunication *nodeCom = 0;
    if (d->corporaObserverWidget->selectedObjects().count() == 1) {
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(d->corporaObserverWidget->selectedObjects().first());
    }
    if ((!nodeCom) || ((nodeCom) && (!nodeCom->communication))) {
        QMessageBox::warning(this, "Add Recording to Communication",
                             "Please select the corpus Communication to which the Media Recordings will be added.",
                             QMessageBox::Ok);
        return;
    }
    Corpus *corpus = qobject_cast<Corpus *>(nodeCom->communication->parent());
    if (!corpus) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add Media Recordings to Corpus"),
                            corpus->basePath(), tr("Wave Files (*.wav);;All Files (*)"),
                            &selectedFilter, options);
    if (fileNames.count() == 0) return;
    foreach(QString fileName, fileNames) {
        QFileInfo info(fileName);
        if (info.suffix() == "wav") {
            CorpusRecording *rec = new CorpusRecording(info.baseName(), nodeCom->communication);
            rec->setFilename(corpus->getRelativeToBasePath(fileName));
            nodeCom->communication->addRecording(rec);
        }
    }
}

void CorpusExplorerWidget::addAnnotation()
{
    CorpusExplorerTreeNodeCommunication *nodeCom = 0;
    if (d->corporaObserverWidget->selectedObjects().count() == 1) {
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(d->corporaObserverWidget->selectedObjects().first());
    }
    if ((!nodeCom) || ((nodeCom) && (!nodeCom->communication))) {
        QMessageBox::warning(this, "Add Recording to Communication",
                             "Please select the corpus Communication to which the Annotation will be added.",
                             QMessageBox::Ok);
        return;
    }
    Corpus *corpus = qobject_cast<Corpus *>(nodeCom->communication->parent());
    if (!corpus) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add Annotation to Corpus"),
                            corpus->basePath(), tr("Praat textgrid (*.textgrid);;Text file (*.txt);;All Files (*)"),
                            &selectedFilter, options);
    if (fileNames.count() == 0) return;
    foreach(QString fileName, fileNames) {
        QFileInfo info(fileName);
        if (info.suffix() == "textgrid") {
            CorpusAnnotation *annot = new CorpusAnnotation(info.baseName(), nodeCom->communication);
            annot->setFilename(corpus->getRelativeToBasePath(fileName));
            if (nodeCom->communication->hasRecording(annot->ID()))
                annot->setRecordingID(annot->ID());
            annot->setFormat("Tier Group (Praat Textgrid)");
            nodeCom->communication->addAnnotation(annot);
        }
    }
}

void CorpusExplorerWidget::addParticipation()
{

}

QList<CorpusObject *> CorpusExplorerWidget::selectedCorpusItems()
{
    QList<CorpusObject *> selected;
    foreach (QObject *obj, d->corporaObserverWidget->selectedObjects()) {
        CorpusExplorerTreeNodeCommunication *nodeCom = 0;
        nodeCom = qobject_cast<CorpusExplorerTreeNodeCommunication *>(obj);
        if (nodeCom && nodeCom->communication) { selected << nodeCom->communication; continue; }
        CorpusExplorerTreeNodeSpeaker *nodeSpk = 0;
        nodeSpk = qobject_cast<CorpusExplorerTreeNodeSpeaker *>(obj);
        if (nodeSpk && nodeSpk->speaker) { selected << nodeSpk->speaker; continue; }
        CorpusExplorerTreeNodeRecording *nodeRec = 0;
        nodeRec = qobject_cast<CorpusExplorerTreeNodeRecording *>(obj);
        if (nodeRec && nodeRec->recording) { selected << nodeRec->recording; continue; }
        CorpusExplorerTreeNodeAnnotation *nodeAnnot = 0;
        nodeAnnot = qobject_cast<CorpusExplorerTreeNodeAnnotation *>(obj);
        if (nodeAnnot && nodeAnnot->annotation) { selected << nodeAnnot->annotation; continue; }
    }
    return selected;
}

QString corpusObjectTypeToString(CorpusObject::Type type)
{
    switch (type) {
    case CorpusObject::Type_Corpus: return "Corpus"; break;
    case CorpusObject::Type_Communication: return "Communication"; break;
    case CorpusObject::Type_Speaker: return "Speaker"; break;
    case CorpusObject::Type_Recording: return "Recording"; break;
    case CorpusObject::Type_Annotation: return "Annotation"; break;
    case CorpusObject::Type_Participation: return "Participation"; break;
    case CorpusObject::Type_Bookmark: return "Bookmark"; break;
    case CorpusObject::Type_Undefined: return "Corpus Item"; break;
    }
    return QString();
}

void CorpusExplorerWidget::removeCorpusItems()
{
    QList<CorpusObject *> selected = selectedCorpusItems();
    if (selected.count() == 0) return;
    if (selected.count() == 1) {
        CorpusCommunication *com = qobject_cast<CorpusCommunication *>(selected.first());
        if (com) {
            Corpus *corpus = qobject_cast<Corpus *>(com->parent());
            if (!corpus) return;
            bool alsoDeleteData = false;
            if (QMessageBox::warning(this, "Remove communication from corpus?",
                                     QString("Do you want to remove Communication %1 from Corpus %2?")
                                     .arg(com->name()).arg(corpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            if (QMessageBox::warning(this, "Permanently delete data?",
                                     QString("Do you also want to delete all the annotation data associated with Communication %1 from Corpus %2?")
                                     .arg(com->name()).arg(corpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                alsoDeleteData = true;
            }
            if (alsoDeleteData) {
                foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                    if (annot) {
                        corpus->datastoreAnnotations()->deleteAllForAnnotationID(annot->ID());
                        com->removeAnnotation(annot->ID());
                    }
                }
            }
            corpus->removeCommunication(com->ID());
            return;
        }
        CorpusSpeaker *spk = qobject_cast<CorpusSpeaker *>(selected.first());
        if (spk) {
            Corpus *corpus = qobject_cast<Corpus *>(spk->parent());
            if (!corpus) return;
            if (QMessageBox::warning(this, "Remove speaker from corpus?",
                                     QString("Do you want to remove Speaker %1 from Corpus %2?")
                                     .arg(spk->name()).arg(corpus->ID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            corpus->removeSpeaker(spk->ID());
            return;
        }
        CorpusRecording *rec = qobject_cast<CorpusRecording *>(selected.first());
        if (rec) {
            CorpusCommunication *com = qobject_cast<CorpusCommunication *>(rec->parent());
            if (!com) return;
            if (QMessageBox::warning(this, "Remove recording from communication?",
                                     QString("Do you want to remove Recording %1 from Communication %2?")
                                     .arg(rec->name()).arg(rec->communicationID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            com->removeRecording(rec->ID());
            return;
        }
        CorpusAnnotation *annot = qobject_cast<CorpusAnnotation *>(selected.first());
        if (annot) {
            CorpusCommunication *com = qobject_cast<CorpusCommunication *>(annot->parent());
            if (!com) return;
            bool alsoDeleteData = false;
            if (QMessageBox::warning(this, "Remove annotation from communication?",
                                     QString("Do you want to remove Annotation %1 from Communication %2?")
                                     .arg(annot->name()).arg(annot->communicationID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
            if (QMessageBox::warning(this, "Permanently delete data?",
                                     QString("Do you also want to delete all the annotation data associated with Annotation %1 in Communication %2?")
                                     .arg(annot->name()).arg(annot->communicationID()), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                alsoDeleteData = true;
            }
            if (alsoDeleteData) {
                Corpus *corpus = qobject_cast<Corpus *>(com->parent());
                if (corpus) {
                    corpus->datastoreAnnotations()->deleteAllForAnnotationID(annot->ID());
                }
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
        modelDel->setHorizontalHeaderLabels(QStringList() << "ID" << "Type");
        int i = 0;
        foreach (CorpusObject *cobj, selected) {
            QStandardItem *item = new QStandardItem(cobj->ID());
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            modelDel->setItem(i, 0, item);
            modelDel->setItem(i, 1, new QStandardItem(corpusObjectTypeToString(cobj->type())));
            i++;
        }
        // Confirmations
        QPointer<SelectionDialog> sel = new SelectionDialog("Confirm deletion of corpus items", modelDel, this);
        if (sel->exec() == QDialog::Rejected)
            return;
        bool alsoDeleteData = false;
        if (QMessageBox::warning(this, "Permanently delete data?",
                                 QString("Do you also want to delete the associated annotation data?"),
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
                                foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                                    if (annot) {
                                        corpus->datastoreAnnotations()->deleteAllForAnnotationID(annot->ID());
                                        com->removeAnnotation(annot->ID());
                                    }
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
                    if (com) com->removeAnnotation(cobj->ID());
                    if (alsoDeleteData) {
                        Corpus *corpus = qobject_cast<Corpus *>(com->parent());
                        if (corpus) {
                            corpus->datastoreAnnotations()->deleteAllForAnnotationID(cobj->ID());
                        }
                    }
                }
            }
        }
        d->corporaTopLevelNode->endTreeProcessingCycle();
    }
}

void CorpusExplorerWidget::relinkCorpusItem()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QList<CorpusObject *> selected = selectedCorpusItems();

    if (selected.count() < 1) {
        QMessageBox::warning(this, "Relink Corpus Items",
                             "Please select Recording(s) and Annotation(s) to change their association with a Communication.",
                             QMessageBox::Ok);
        return;
    }
    // Get destination communication
    bool ok;
    QString communicationID = QInputDialog::getItem (this, "Relink Corpus Item",
                                                    "Please select the ID of the Communication to which you want to move the selected Recording",
                                                    corpus->communicationIDs(), 0, false, &ok);
    if (!ok || communicationID.isEmpty()) return;
    // Relink
    foreach (CorpusObject *cobj, selected) {
        if (cobj->type() == CorpusObject::Type_Recording) {
            CorpusRecording *rec = qobject_cast<CorpusRecording *>(cobj);
            QPointer<CorpusCommunication> com = corpus->communication(rec->communicationID());
            if (!com) return;
            com->unlinkRecording(rec->ID());
            com = corpus->communication(communicationID);
            com->addRecording(rec);
        }
        else if (cobj->type() == CorpusObject::Type_Annotation) {
            CorpusAnnotation *annot = qobject_cast<CorpusAnnotation *>(cobj);
            QPointer<CorpusCommunication> com = corpus->communication(annot->communicationID());
            if (!com) return;
            com->unlinkAnnotation(annot->ID());
            com = corpus->communication(communicationID);
            com->addAnnotation(annot);
        }
    }
}

void CorpusExplorerWidget::addItemsFromFolder()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    d->corporaTopLevelNode->startTreeProcessingCycle();
    ImportCorpusItemsWizard *wizard = new ImportCorpusItemsWizard(corpus, this);
    wizard->exec(); // MODAL!
    d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusExplorerWidget::importMetadata()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Metadata Wizard - Select text file"), "",
                                                    tr("Text File (*.txt);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    d->corporaTopLevelNode->startTreeProcessingCycle();
    ImportMetadataWizard *wizard = new ImportMetadataWizard(filename, corpus, this);
    wizard->exec(); // MODAL!
    d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusExplorerWidget::exportMetadata()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    ExportMetadataWizard *wizard = new ExportMetadataWizard(corpus, this);
    wizard->exec(); // MODAL!
}

void CorpusExplorerWidget::exportAnnotations()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    ExportAnnotationsWizard *wizard = new ExportAnnotationsWizard(corpus, this);
    wizard->exec(); // MODAL!
}

void CorpusExplorerWidget::checkMediaFiles()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    CheckMediaFilesDialog *dialog = new CheckMediaFilesDialog(corpus, this);
    dialog->exec();
    delete dialog;
}

void CorpusExplorerWidget::splitCommunications()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    SplitCommunicationsDialog *dialog = new SplitCommunicationsDialog(corpus, this);
    dialog->exec();
    delete dialog;
}

void CorpusExplorerWidget::createAnnotationsFromRecordings()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    d->corporaTopLevelNode->startTreeProcessingCycle();
    foreach (CorpusCommunication *com, corpus->communications()) {
        if (!com) continue;
        if (com->hasAnnotations()) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            CorpusAnnotation *annot = new CorpusAnnotation(rec->ID());
            com->addAnnotation(annot);
        }
    }
    d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusExplorerWidget::createSpeakersFromAnnotations()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    d->corporaTopLevelNode->startTreeProcessingCycle();
    foreach (CorpusCommunication *com, corpus->communications()) {
        if (!com) continue;
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QList<QString> speakerIDs = corpus->datastoreAnnotations()->getSpeakersInAnnotation(annot->ID());
            foreach (QString speakerID, speakerIDs) {
                if (!corpus->hasSpeaker(speakerID))
                    corpus->addSpeaker(new CorpusSpeaker(speakerID));
                if (!corpus->hasParticipation(com->ID(), speakerID))
                    corpus->addParticipation(com->ID(), speakerID);
            }
        }
    }
    d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusExplorerWidget::cleanUpParticipationsFromAnnotations()
{
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;

    bool deleteSpeakers = false;
    if (QMessageBox::question(this, "Clean up Speakers as well?", "When a speaker does not participate in a Communication, based on its Annotations, "
                              "the Participation will be deleted. Should the Speaker metadata also be deleted? (Caution: this operation cannot be undone!)")
            == QMessageBox::Yes) {
        deleteSpeakers = true;
    }
    QSet<QString> allActiveSpeakerIDs;

    d->corporaTopLevelNode->startTreeProcessingCycle();

    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, corpus->communicationsCount());
    progressDialog.setWindowTitle(tr("Analysing Corpus Data"));

    int i = 1;
    foreach (CorpusCommunication *com, corpus->communications()) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Analysing Communication %1 of %2...")
                                    .arg(i).arg(corpus->communicationsCount()));
        ++i;
        qApp->processEvents();

        if (!com) continue;
        QStringList activeSpeakerIDs;
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QList<QString> speakerIDs = corpus->datastoreAnnotations()->getSpeakersActiveInAnnotation(annot->ID());
            foreach (QString speakerID, speakerIDs)
                if (!activeSpeakerIDs.contains(speakerID)) activeSpeakerIDs << speakerID;
        }
        allActiveSpeakerIDs.unite(QSet<QString>::fromList(activeSpeakerIDs));
        QStringList inactiveSpeakerIDs;
        foreach (QPointer<CorpusParticipation> participation, corpus->participationsForCommunication(com->ID())) {
            if (!participation) continue;
            if (!activeSpeakerIDs.contains(participation->speakerID()))
                if (!inactiveSpeakerIDs.contains(participation->speakerID()))
                    inactiveSpeakerIDs << participation->speakerID();
        }
        foreach (QString speakerID, inactiveSpeakerIDs) {
            corpus->removeParticipation(com->ID(), speakerID);
        }
    }
    if (deleteSpeakers) {
        foreach (QString speakerID, corpus->speakerIDs()) {
            if (!allActiveSpeakerIDs.contains(speakerID)) {
                corpus->removeSpeaker(speakerID);
            }
        }
    }
    d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusExplorerWidget::attributesAndGroupings()
{
    QPointer<CorpusObserver> obs = d->corporaManager->activeCorpusObserver();
    if (!obs) return;
    QPointer<Corpus> corpus = d->corporaManager->activeCorpus();
    if (!corpus) return;
    CorpusExplorerOptionsDialog *dialog = new CorpusExplorerOptionsDialog(corpus->metadataStructure(), this);
    if (dialog->exec() != QDialog::Accepted) return;

    obs->setCommunicationsGrouping(dialog->groupAttributesForCommunications());
    obs->setSpeakersGrouping(dialog->groupAttributesForSpeakers());
}
