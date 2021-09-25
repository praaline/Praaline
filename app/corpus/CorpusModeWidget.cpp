#include <memory>
#include <vector>
using namespace std;

#include "CorpusModeWidget.h"
#include "ui_CorpusModeWidget.h"

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "corpusexplorer/CorpusExplorerTreeWidget.h"
#include "corpusexplorer/CorpusExplorerTableWidget.h"
#include "structureeditors/MetadataStructureEditor.h"
#include "structureeditors/AnnotationStructureEditor.h"

#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/CorpusRepositoryDefinition.h"
#include "PraalineCore/Datastore/FileDatastore.h"
using namespace Praaline::Core;

#include "svcore/base/RecentFiles.h"

#include "CorpusRepositoriesManager.h"
#include "CorpusRepositoryCreateWizard.h"
#include "CorpusRepositoryPropertiesDialog.h"

#include "importcorpusitems/ImportCorpusItemsWizard.h"
#include "importmetadata/ImportMetadataWizard.h"
#include "exportmetadata/ExportMetadataWizard.h"
#include "utilities/CheckMediaFilesDialog.h"
#include "utilities/SplitCommunicationsDialog.h"
#include "utilities/MergeCommunicationsDialog.h"
#include "utilities/DecodeFilenameToMetadataDialog.h"
#include "utilities/MergeCorporaDialog.h"


struct CorpusModeWidgetData {
    CorpusModeWidgetData() :
        recentFiles(nullptr), recentFilesMenu(nullptr)
    { }

    // Repository Management
    QAction *actionNewCorpusRepository;
    QAction *actionOpenCorpusRepository;
    QAction *actionEditCorpusRepository;
    QAction *actionCloseCorpusRepository;
    QAction *actionSaveCorpusRepository;
    QAction *actionSaveCorpusRepositoryAs;
    // Show different widgets of the Corpus Mode
    QAction *actionShowCorpusExplorer;
    QAction *actionShowCorpusExplorerTables;
    QAction *actionShowMetadataStructureEditor;
    QAction *actionShowAnnotationStructureEditor;
    // Import-export
    QAction *actionAddItemsFromFolder;
    QAction *actionImportMetadata;
    QAction *actionExportMetadata;
    // Corpus integrity
    QAction *actionCheckMediaFiles;
    QAction *actionCreateAnnotationsFromRecordings;
    QAction *actionCreateSpeakersFromAnnotations;
    QAction *actionCleanUpParticipations;
    // Utilities
    QAction *actionSplitCommunications;
    QAction *actionMergeCommunications;
    QAction *actionDecodeFilenameToMetadata;
    QAction *actionMergeCorpora;

    // Widgets belonging to the Corpus Mode
    CorpusExplorerTreeWidget *widgetCorpusExplorerTree;
    CorpusExplorerTableWidget *widgetCorpusExplorerTables;
    MetadataStructureEditor *widgetMetadataStructureEditor;
    AnnotationStructureEditor *widgetAnnotationStructureEditor;

    // Recent files functionality
    RecentFiles *recentFiles;
    ActionContainer* recentFilesMenu;

    // Pointer to the central corpus repository manager
    CorpusRepositoriesManager *corpusRepositoriesManager;
};

CorpusModeWidget::CorpusModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusModeWidget), d(new CorpusModeWidgetData)
{
    ui->setupUi(this);

    d->widgetCorpusExplorerTree = new CorpusExplorerTreeWidget(this);
    d->widgetCorpusExplorerTables = new CorpusExplorerTableWidget(this);
    d->widgetMetadataStructureEditor = new MetadataStructureEditor(this);
    d->widgetAnnotationStructureEditor = new AnnotationStructureEditor(this);

    ui->gridLayoutCorpusExplorer->addWidget(d->widgetCorpusExplorerTree);
    ui->gridLayoutCorpusExplorerTables->addWidget(d->widgetCorpusExplorerTables);
    ui->gridLayoutMetadataStructureEditor->addWidget(d->widgetMetadataStructureEditor);
    ui->gridLayoutAnnotationStructureEditor->addWidget(d->widgetAnnotationStructureEditor);

    // Recent files
    d->recentFiles = new RecentFiles("RecentCorpusFiles", 20);

    setupActions();

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->commandCorpusExplorer, &QAbstractButton::clicked, this, &CorpusModeWidget::showCorpusExplorer);
    connect(ui->commandCorpusExplorerTables, &QAbstractButton::clicked, this, &CorpusModeWidget::showCorpusExplorerTables);
    connect(ui->commandMetadataStructureEditor, &QAbstractButton::clicked, this, &CorpusModeWidget::showMetadataStructureEditor);
    connect(ui->commandAnnotationStructureEditor, &QAbstractButton::clicked, this, &CorpusModeWidget::showAnnotationStructureEditor);

    // Get Corpus Repositories Manager from global object list
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("CorpusRepositoriesManager");
    foreach (QObject* obj, list) {
        CorpusRepositoriesManager *manager = qobject_cast<CorpusRepositoriesManager *>(obj);
        if (manager) d->corpusRepositoriesManager = manager;
    }
}

CorpusModeWidget::~CorpusModeWidget()
{
    delete ui;
    delete d;
}

void CorpusModeWidget::setupActions()
{
    // Context and menu bar
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    bool existed;
    Command* command;

    ActionContainer* menubar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* menu_file = ACTION_MANAGER->menu(qti_action_FILE);
    Q_ASSERT(menu_file);
    ActionContainer* menu_corpus = ACTION_MANAGER->createMenu(tr("&Corpus"), existed);
    if (!existed) menubar->addMenu(menu_corpus, qti_action_HELP);
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, tr("&Window"));

    // ------------------------------------------------------------------------------------------------------
    // FILE MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionNewCorpusRepository = new QAction(tr("New Corpus Repository..."), this);
    connect(d->actionNewCorpusRepository, &QAction::triggered, this, &CorpusModeWidget::newCorpusRepository);
    command = ACTION_MANAGER->registerAction("File.NewCorpusRepository", d->actionNewCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    d->actionOpenCorpusRepository = new QAction(tr("Open Corpus Repository..."), this);
    connect(d->actionOpenCorpusRepository, &QAction::triggered, this, qOverload<>(&CorpusModeWidget::openCorpusRepository));
    command = ACTION_MANAGER->registerAction("File.OpenCorpusRepository", d->actionOpenCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    d->recentFilesMenu = ACTION_MANAGER->createMenu(tr("Open &Recent Corpus Repository"), existed);
    if (!existed) menu_file->addMenu(d->recentFilesMenu, qti_action_FILE_SETTINGS);
    setupRecentFilesMenu();
    connect(d->recentFiles, &RecentFiles::recentChanged, this, &CorpusModeWidget::setupRecentFilesMenu);

    menu_file->addSeparator(qti_action_FILE_SETTINGS);

    d->actionEditCorpusRepository = new QAction(tr("Edit Corpus Repository Properties..."), this);
    connect(d->actionEditCorpusRepository, &QAction::triggered, this, &CorpusModeWidget::editCorpusRepository);
    command = ACTION_MANAGER->registerAction("File.EditCorpusRepository", d->actionEditCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    menu_file->addSeparator(qti_action_FILE_SETTINGS);

    d->actionSaveCorpusRepository = new QAction(tr("Save Corpus Repository Properties File"), this);
    connect(d->actionSaveCorpusRepository, &QAction::triggered, this, &CorpusModeWidget::saveCorpusRepository);
    command = ACTION_MANAGER->registerAction("File.SaveCorpusRepository", d->actionSaveCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    d->actionSaveCorpusRepositoryAs = new QAction(tr("Save Corpus Repository Properties File As..."), this);
    connect(d->actionSaveCorpusRepositoryAs, &QAction::triggered, this, &CorpusModeWidget::saveCorpusRepositoryAs);
    command = ACTION_MANAGER->registerAction("File.SaveCorpusRepositoryAs", d->actionSaveCorpusRepositoryAs, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    menu_file->addSeparator(qti_action_FILE_SETTINGS);

    d->actionCloseCorpusRepository = new QAction(tr("Close Corpus Repository"), this);
    connect(d->actionCloseCorpusRepository, &QAction::triggered, this, &CorpusModeWidget::closeCorpusRepository);
    command = ACTION_MANAGER->registerAction("File.CloseCorpusRepository", d->actionCloseCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorpusExplorer = new QAction(tr("Corpus Explorer (trees)"), this);
    connect(d->actionShowCorpusExplorer, &QAction::triggered, this, &CorpusModeWidget::showCorpusExplorer);
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorer", d->actionShowCorpusExplorer, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCorpusExplorerTables = new QAction(tr("Corpus Explorer (tabular)"), this);
    connect(d->actionShowCorpusExplorerTables, &QAction::triggered, this, &CorpusModeWidget::showCorpusExplorerTables);
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorerTables", d->actionShowCorpusExplorerTables, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowMetadataStructureEditor = new QAction(tr("Metadata Structure Editor"), this);
    connect(d->actionShowMetadataStructureEditor, &QAction::triggered, this, &CorpusModeWidget::showMetadataStructureEditor);
    command = ACTION_MANAGER->registerAction("Window.ShowMetadataStructureEditor", d->actionShowMetadataStructureEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowAnnotationStructureEditor = new QAction(tr("Annotation Structure Editor"), this);
    connect(d->actionShowAnnotationStructureEditor, &QAction::triggered, this, &CorpusModeWidget::showAnnotationStructureEditor);
    command = ACTION_MANAGER->registerAction("Window.ShowAnnotationStructureEditor", d->actionShowAnnotationStructureEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();

    // ------------------------------------------------------------------------------------------------------
    // CORPUS MENU
    // ------------------------------------------------------------------------------------------------------

    // Import - export functionality
    // --------------------------------------------------------------------------------------------
    d->actionAddItemsFromFolder = new QAction(tr("Add corpus items from folder..."), this);
    connect(d->actionAddItemsFromFolder, &QAction::triggered, this, &CorpusModeWidget::addItemsFromFolder);
    command = ACTION_MANAGER->registerAction("Corpus.AddItemsFromFolder", d->actionAddItemsFromFolder, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionImportMetadata = new QAction(tr("Import corpus metadata..."), this);
    connect(d->actionImportMetadata, &QAction::triggered, this, &CorpusModeWidget::importMetadata);
    command = ACTION_MANAGER->registerAction("Corpus.ImportMetadata", d->actionImportMetadata, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionExportMetadata = new QAction(tr("Export corpus metadata..."), this);
    connect(d->actionExportMetadata, &QAction::triggered, this, &CorpusModeWidget::exportMetadata);
    command = ACTION_MANAGER->registerAction("Corpus.ExportMetadata", d->actionExportMetadata, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    menu_corpus->addSeparator();

    // Corpus integrity
    // --------------------------------------------------------------------------------------------

    d->actionCheckMediaFiles = new QAction(tr("Check files of Media Recordings..."), this);
    connect(d->actionCheckMediaFiles, &QAction::triggered, this, &CorpusModeWidget::checkMediaFiles);
    command = ACTION_MANAGER->registerAction("Corpus.CheckMediaFiles", d->actionCheckMediaFiles, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionCreateAnnotationsFromRecordings = new QAction(tr("Create Annotations for Recordings not having one..."), this);
    connect(d->actionCreateAnnotationsFromRecordings, &QAction::triggered, this, &CorpusModeWidget::createAnnotationsFromRecordings);
    command = ACTION_MANAGER->registerAction("Corpus.AnnotationsFromRecordings", d->actionCreateAnnotationsFromRecordings, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionCreateSpeakersFromAnnotations = new QAction(tr("Create Speakers and Participations from Annotations..."), this);
    connect(d->actionCreateSpeakersFromAnnotations, &QAction::triggered, this, &CorpusModeWidget::createSpeakersFromAnnotations);
    command = ACTION_MANAGER->registerAction("Corpus.SpeakersParticipationsFromAnnotations", d->actionCreateSpeakersFromAnnotations, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionCleanUpParticipations = new QAction(tr("Clean-up Participations based on Annotations..."), this);
    connect(d->actionCleanUpParticipations, &QAction::triggered, this, &CorpusModeWidget::cleanUpParticipationsFromAnnotations);
    command = ACTION_MANAGER->registerAction("Corpus.CleanUpParticipationsFromAnnotations", d->actionCleanUpParticipations, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    menu_corpus->addSeparator();

    // Utilities
    // --------------------------------------------------------------------------------------------

    d->actionSplitCommunications = new QAction(tr("Split Communications based on annotation..."), this);
    connect(d->actionSplitCommunications, &QAction::triggered, this, &CorpusModeWidget::utilitiesSplitCommunications);
    command = ACTION_MANAGER->registerAction("Corpus.SplitCommunications", d->actionSplitCommunications, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionMergeCommunications = new QAction(tr("Merge Communications..."), this);
    connect(d->actionMergeCommunications, &QAction::triggered, this, &CorpusModeWidget::utilitiesMergeCommunications);
    command = ACTION_MANAGER->registerAction("Corpus.MergeCommunications", d->actionMergeCommunications, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionDecodeFilenameToMetadata = new QAction(tr("Decode Filename into Metadata..."), this);
    connect(d->actionDecodeFilenameToMetadata, &QAction::triggered, this, &CorpusModeWidget::utilitiesDecodeFilenameToMetadata);
    command = ACTION_MANAGER->registerAction("Corpus.DecodeFilenameToMetadata", d->actionDecodeFilenameToMetadata, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);

    d->actionMergeCorpora = new QAction(tr("Merge Corpora..."), this);
    connect(d->actionMergeCorpora, &QAction::triggered, this, &CorpusModeWidget::utilitiesMergeCorpora);
    command = ACTION_MANAGER->registerAction("Corpus.MergeCorpora", d->actionMergeCorpora, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_corpus->addAction(command);
}

void CorpusModeWidget::setupRecentFilesMenu()
{
    d->recentFilesMenu->menu()->clear();
    vector<QString> files = d->recentFiles->getRecent();
    for (size_t i = 0; i < files.size(); ++i) {
        QAction *action = new QAction(files[i], this);
        connect(action, &QAction::triggered, this, &CorpusModeWidget::openCorpusRepositoryRecent);
        if (i == 0) {
            action->setShortcut(tr("Ctrl+R"));
        }
        d->recentFilesMenu->menu()->addAction(action);
    }
}

// =========================================================================================================================================
// Corpus Mode Widgets (private slots)
// =========================================================================================================================================

void CorpusModeWidget::showCorpusExplorer()
{
    ui->stackedWidget->setCurrentIndex(0);
    emit activateMode();
}

void CorpusModeWidget::showCorpusExplorerTables()
{
    ui->stackedWidget->setCurrentIndex(1);
    emit activateMode();
}

void CorpusModeWidget::showMetadataStructureEditor()
{
    ui->stackedWidget->setCurrentIndex(2);
    emit activateMode();
}

void CorpusModeWidget::showAnnotationStructureEditor()
{
    ui->stackedWidget->setCurrentIndex(3);
    emit activateMode();
}

// =========================================================================================================================================
// Corpus Repository Management (public slots)
// =========================================================================================================================================

bool CorpusModeWidget::checkForActiveCorpusRepository()
{
    if (!d->corpusRepositoriesManager->activeCorpusRepository()) {
        QMessageBox::warning(this, tr("No Corpus Repository"),
                             tr("Please create, open or connect to a Praaline Corpus Repository first."), QMessageBox::Ok);
        return false;
    }
    return true;
}

void CorpusModeWidget::newCorpusRepository()
{
    unique_ptr<CorpusRepositoryCreateWizard> wizard(new CorpusRepositoryCreateWizard(this));
    wizard->exec();
    CorpusRepository *repository = wizard->newCorpusRepository();
    if (!repository) return;
    d->corpusRepositoriesManager->addCorpusRepository(repository);
    d->corpusRepositoriesManager->setActiveCorpusRepository(repository->ID());
    // Update recent files
    d->recentFiles->addFile(repository->definition().filenameDefinition);
    setupRecentFilesMenu();
}

void CorpusModeWidget::openCorpusRepository()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Corpus"), "",
                                                    tr("Praaline Corpus File (*.PraalineRepository);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    openCorpusRepositoryFromDefinition(filename);
}

void CorpusModeWidget::openCorpusRepositoryRecent()
{
    QObject *obj = sender();
    QAction *action = dynamic_cast<QAction *>(obj);
    if (!action) {
        qDebug() << "WARNING: openRecentFile: sender is not an action" << Qt::endl;
        return;
    }
    QString filename = action->text();
    if (filename.isEmpty()) return;
    openCorpusRepositoryFromDefinition(filename);
}

void CorpusModeWidget::openCorpusRepositoryFromDefinition(const QString &filename)
{
    // Check whether the corpus is already open
    if (d->corpusRepositoriesManager->isRepositoryDefinitionAlreadyOpen(filename)) {
        QMessageBox::warning(this, tr("Corpus repository already open"),
                             QString(tr("This corpus repository (%1) is already open.")).arg(filename), QMessageBox::Ok);
        return;
    }
    // Open the corpus definition
    CorpusRepositoryDefinition definition;
    if (!definition.load(filename)) {
        QMessageBox::warning(this, tr("Cannot open corpus repository"),
                             QString(tr("Cannot open corpus repository definition file (%1). Is it a valid Praaline XML corpus repository definition?")).arg(filename),
                             QMessageBox::Ok);
        return;
    }
    // Ask for password, if needed
    QString password;
    if (definition.infoDatastoreMetadata.usePassword == true || definition.infoDatastoreAnnotations.usePassword) {
        password = QInputDialog::getText(this, tr("Enter password"), tr("Enter password to connect to this corpus repository"),
                                         QLineEdit::Password);
        if (definition.infoDatastoreMetadata.usePassword)    definition.infoDatastoreMetadata.password = password;
        if (definition.infoDatastoreAnnotations.usePassword) definition.infoDatastoreAnnotations.password = password;
    }
    // Attempt to open corpus repository
    QString errorMessages;
    CorpusRepository *repository = CorpusRepository::open(definition, errorMessages);
    if ((!errorMessages.isEmpty()) || (!repository)) {
        QMessageBox::warning(this, tr("Error opening corpus repository.\n%1"), errorMessages, QMessageBox::Ok);
        return;
    }
    // Corpus repository opened succesfully. Register corpus with the Corpus Manager and global object pool.
    d->corpusRepositoriesManager->addCorpusRepository(repository);
    d->corpusRepositoriesManager->setActiveCorpusRepository(repository->ID());
    // Update recent files
    d->recentFiles->addFile(filename);
    setupRecentFilesMenu();
}

void CorpusModeWidget::editCorpusRepository()
{
    if (!checkForActiveCorpusRepository()) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->activeCorpusRepository();
    unique_ptr<CorpusRepositoryPropertiesDialog> dialog(new CorpusRepositoryPropertiesDialog(repository->definition(), this));
    if (dialog->exec() == QDialog::Accepted) {
        repository->setID(dialog->repositoryDef().repositoryID);
        repository->setDescription(dialog->repositoryDef().repositoryName);
        repository->setBasePathMedia(dialog->repositoryDef().basePathMedia);
        if (!repository->definition().filenameDefinition.isEmpty()) {
            repository->definition().save(repository->definition().filenameDefinition);
        }
    }
}

void CorpusModeWidget::closeCorpusRepository()
{
    if (!checkForActiveCorpusRepository()) return;
    QString repositoryID = d->corpusRepositoriesManager->activeCorpusRepository()->ID();
    d->corpusRepositoriesManager->activeCorpusRepository()->close();
    // Remove the repository from the manager. It will automatically activate the next one, if available.
    d->corpusRepositoriesManager->removeCorpusRepository(repositoryID);
}

void CorpusModeWidget::saveCorpusRepository()
{
    if (!checkForActiveCorpusRepository()) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository->definition().filenameDefinition.isEmpty()) {
        repository->definition().save(repository->definition().filenameDefinition);
    } else {
        saveCorpusRepositoryAs();
    }
}

void CorpusModeWidget::saveCorpusRepositoryAs()
{
    if (!checkForActiveCorpusRepository()) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->activeCorpusRepository();
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Corpus As..."), repository->definition().filenameDefinition,
                                                    tr("Praaline Corpus File (*.PraalineRepository);;All Files (*)"), &selectedFilter, options);
    if (filename.isEmpty()) return;
    repository->definition().save(filename);
    // Update recent files
    d->recentFiles->addFile(repository->definition().filenameDefinition);
    setupRecentFilesMenu();
    // else show error message
    // m_mainStatusBar->showMessage("Saved corpus \"" + fileName, 5000);
}

// =========================================================================================================================================
// Import Corpus Items from other software formats
// =========================================================================================================================================

void CorpusModeWidget::addItemsFromFolder()
{
    if (!checkForActiveCorpusRepository()) return;
    // d->corporaTopLevelNode->startTreeProcessingCycle();
    // FIX ME ImportCorpusItemsWizard *wizard = new ImportCorpusItemsWizard(d->activeCorpus, this);
    // wizard->exec(); // MODAL!
    // d->corporaTopLevelNode->endTreeProcessingCycle();
    CorpusObserver *obj = d->corpusRepositoriesManager->corpusObserverForRepository(d->corpusRepositoriesManager->activeCorpusRepositoryID());
    if (obj) obj->refresh();
    // fix update
    QString filenameDef = d->corpusRepositoriesManager->activeCorpusRepository()->definition().filenameDefinition;
    saveCorpusRepository();
    closeCorpusRepository();
    openCorpusRepositoryFromDefinition(filenameDef);
}

// =========================================================================================================================================
// Import / Export Metadata
// =========================================================================================================================================

void CorpusModeWidget::importMetadata()
{
    if (!checkForActiveCorpusRepository()) return;
    QFileDialog::Options options;
    QString selectedFilter;
    QString filename = QFileDialog::getOpenFileName(this, tr("Import Metadata Wizard - Select text file"), "",
                                                    tr("Text File (*.txt);;All Files (*)"),
                                                    &selectedFilter, options);
    if (filename.isEmpty()) return;
    unique_ptr<ImportMetadataWizard> wizard(new ImportMetadataWizard(filename, this));
    wizard->exec(); // modal
    CorpusObserver *obj = d->corpusRepositoriesManager->corpusObserverForRepository(d->corpusRepositoriesManager->activeCorpusRepositoryID());
    if (obj) obj->refresh();
}

void CorpusModeWidget::exportMetadata()
{
    if (!checkForActiveCorpusRepository()) return;
    unique_ptr<ExportMetadataWizard> wizard(new ExportMetadataWizard(this));
    wizard->exec(); // modal
}

// ==============================================================================================================================
// Corpus Integrity
// ==============================================================================================================================

void CorpusModeWidget::checkMediaFiles()
{
    if (!checkForActiveCorpusRepository()) return;
    // unique_ptr<CheckMediaFilesDialog> dialog(new CheckMediaFilesDialog(d->activeCorpus, this));
    // dialog->exec();
}

void CorpusModeWidget::createAnnotationsFromRecordings()
{
    if (!checkForActiveCorpusRepository()) return;
    // d->corporaTopLevelNode->startTreeProcessingCycle();
    /*
    foreach (CorpusCommunication *com, d->activeCorpus->communications()) {
        if (!com) continue;
        if (com->hasAnnotations()) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            CorpusAnnotation *annot = new CorpusAnnotation(rec->ID());
            com->addAnnotation(annot);
        }
    }
    */
    // d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusModeWidget::createSpeakersFromAnnotations()
{
    if (!checkForActiveCorpusRepository()) return;
    // d->corporaTopLevelNode->startTreeProcessingCycle();
    // for each corpus
    /*
    foreach (CorpusCommunication *com, d->activeCorpus->communications()) {
        if (!com) continue;
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QList<QString> speakerIDs = d->activeCorpus->repository()->annotations()->getSpeakersInAnnotation(annot->ID());
            foreach (QString speakerID, speakerIDs) {
                if (!d->activeCorpus->hasSpeaker(speakerID))
                    d->activeCorpus->addSpeaker(new CorpusSpeaker(speakerID));
                if (!d->activeCorpus->hasParticipation(com->ID(), speakerID))
                    d->activeCorpus->addParticipation(com->ID(), speakerID);
            }
        }
    }
    */
    // d->corporaTopLevelNode->endTreeProcessingCycle();
}

void CorpusModeWidget::cleanUpParticipationsFromAnnotations()
{
    if (!checkForActiveCorpusRepository()) return;
    bool deleteSpeakers = false;
    if (QMessageBox::question(this, tr("Clean up Speakers as well?"), tr("When a speaker does not participate in a Communication, based on its Annotations, "
                              "the Participation will be deleted. Should the Speaker metadata also be deleted? (Caution: this operation cannot be undone!)"))
            == QMessageBox::Yes) {
        deleteSpeakers = true;
    }
    QSet<QString> allActiveSpeakerIDs;

    // d->corporaTopLevelNode->startTreeProcessingCycle();

/*
    QProgressDialog progressDialog(this);
    progressDialog.setCancelButtonText(tr("&Cancel"));
    progressDialog.setRange(0, d->activeCorpus->communicationsCount());
    progressDialog.setWindowTitle(tr("Analysing Corpus Data"));

    int i = 1;
    foreach (CorpusCommunication *com, d->activeCorpus->communications()) {
        progressDialog.setValue(i);
        progressDialog.setLabelText(tr("Analysing Communication %1 of %2...")
                                    .arg(i).arg(d->activeCorpus->communicationsCount()));
        ++i;
        qApp->processEvents();

        if (!com) continue;
        QStringList activeSpeakerIDs;
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QList<QString> speakerIDs = d->activeCorpus->repository()->annotations()->getSpeakersActiveInAnnotation(annot->ID());
            foreach (QString speakerID, speakerIDs)
                if (!activeSpeakerIDs.contains(speakerID)) activeSpeakerIDs << speakerID;
        }
        allActiveSpeakerIDs.unite(QSet<QString>::fromList(activeSpeakerIDs));
        QStringList inactiveSpeakerIDs;
        foreach (CorpusParticipation *participation, d->activeCorpus->participationsForCommunication(com->ID())) {
            if (!participation) continue;
            if (!activeSpeakerIDs.contains(participation->speakerID()))
                if (!inactiveSpeakerIDs.contains(participation->speakerID()))
                    inactiveSpeakerIDs << participation->speakerID();
        }
        foreach (QString speakerID, inactiveSpeakerIDs) {
            d->activeCorpus->removeParticipation(com->ID(), speakerID);
        }
    }
    if (deleteSpeakers) {
        foreach (QString speakerID, d->activeCorpus->speakerIDs()) {
            if (!allActiveSpeakerIDs.contains(speakerID)) {
                d->activeCorpus->removeSpeaker(speakerID);
            }
        }
    }
    // d->corporaTopLevelNode->endTreeProcessingCycle();
    */
}

// =========================================================================================================================================
// Corpus Management Utilities
// =========================================================================================================================================

void CorpusModeWidget::utilitiesSplitCommunications()
{
    if (!checkForActiveCorpusRepository()) return;
    // FIX ME: require repository and not corpus
    // unique_ptr<SplitCommunicationsDialog> dialog(new SplitCommunicationsDialog(d->activeCorpus, this));
    // dialog->exec();
}

void CorpusModeWidget::utilitiesMergeCommunications()
{
    if (!checkForActiveCorpusRepository()) return;
    // FIX ME: require repository and not corpus
    // unique_ptr<MergeCommunicationsDialog> dialog(new MergeCommunicationsDialog(d->activeCorpus, this));
    // dialog->exec();
}

void CorpusModeWidget::utilitiesDecodeFilenameToMetadata()
{
    if (!checkForActiveCorpusRepository()) return;
    // FIX ME: require repository and not corpus
    // unique_ptr<DecodeFilenameToMetadataDialog> dialog(new DecodeFilenameToMetadataDialog(d->activeCorpus, this));
    // dialog->exec();
}

void CorpusModeWidget::utilitiesMergeCorpora()
{
    unique_ptr<MergeCorporaDialog> dialog(new MergeCorporaDialog(this));
    dialog->exec();
}
