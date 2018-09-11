#include "CorpusModeWidget.h"
#include "ui_CorpusModeWidget.h"

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "corpusexplorer/CorpusExplorerWidget.h"
#include "corpusexplorer/CorpusExplorerTableWidget.h"
#include "structureeditors/MetadataStructureEditor.h"
#include "structureeditors/AnnotationStructureEditor.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"
#include "pncore/datastore/FileDatastore.h"
using namespace Praaline::Core;

#include "svcore/base/RecentFiles.h"
using std::vector;

#include "CorpusRepositoriesManager.h"
#include "CorpusRepositoryCreateWizard.h"
#include "CorpusRepositoryPropertiesDialog.h"

struct CorpusModeWidgetData {
    CorpusModeWidgetData() :
        recentFiles(0), recentFilesMenu(0)
    { }

    QAction *actionNewCorpusRepository;
    QAction *actionOpenCorpusRepository;
    QAction *actionEditCorpusRepository;
    QAction *actionCloseCorpusRepository;
    QAction *actionSaveCorpusRepository;
    QAction *actionSaveCorpusRepositoryAs;

    QAction *actionShowCorpusExplorer;
    QAction *actionShowCorpusExplorerTables;
    QAction *actionShowMetadataStructureEditor;
    QAction *actionShowAnnotationStructureEditor;

    CorpusExplorerWidget *widgetCorpusExplorer;
    CorpusExplorerTableWidget *widgetCorpusExplorerTables;
    MetadataStructureEditor *widgetMetadataStructureEditor;
    AnnotationStructureEditor *widgetAnnotationStructureEditor;

    RecentFiles *recentFiles;
    ActionContainer* recentFilesMenu;

    CorpusRepositoriesManager *corpusRepositoriesManager;
};

CorpusModeWidget::CorpusModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusModeWidget), d(new CorpusModeWidgetData)
{
    ui->setupUi(this);

    d->widgetCorpusExplorer = new CorpusExplorerWidget(this);
    d->widgetCorpusExplorerTables = new CorpusExplorerTableWidget(this);
    d->widgetMetadataStructureEditor = new MetadataStructureEditor(this);
    d->widgetAnnotationStructureEditor = new AnnotationStructureEditor(this);

    ui->gridLayoutCorpusExplorer->addWidget(d->widgetCorpusExplorer);
    ui->gridLayoutCorpusExplorerTables->addWidget(d->widgetCorpusExplorerTables);
    ui->gridLayoutMetadataStructureEditor->addWidget(d->widgetMetadataStructureEditor);
    ui->gridLayoutAnnotationStructureEditor->addWidget(d->widgetAnnotationStructureEditor);

    // Recent files
    d->recentFiles = new RecentFiles("RecentCorpusFiles", 20);

    setupActions();

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->commandCorpusExplorer, SIGNAL(clicked()), this, SLOT(showCorpusExplorer()));
    connect(ui->commandCorpusExplorerTables, SIGNAL(clicked()), this, SLOT(showCorpusExplorerTables()));
    connect(ui->commandMetadataStructureEditor, SIGNAL(clicked()), this, SLOT(showMetadataStructureEditor()));
    connect(ui->commandAnnotationStructureEditor, SIGNAL(clicked()), this, SLOT(showAnnotationStructureEditor()));

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
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, tr("&Window"));

    // ------------------------------------------------------------------------------------------------------
    // FILE MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionNewCorpusRepository = new QAction(tr("New Corpus Repository..."), this);
    connect(d->actionNewCorpusRepository, SIGNAL(triggered()), SLOT(newCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.NewCorpusRepository", d->actionNewCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    d->actionOpenCorpusRepository = new QAction(tr("Open Corpus Repository..."), this);
    connect(d->actionOpenCorpusRepository, SIGNAL(triggered()), SLOT(openCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.OpenCorpusRepository", d->actionOpenCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    d->recentFilesMenu = ACTION_MANAGER->createMenu(tr("Open &Recent Corpus Repository"), existed);
    if (!existed) menu_file->addMenu(d->recentFilesMenu, qti_action_FILE_SETTINGS);
    setupRecentFilesMenu();
    connect(d->recentFiles, SIGNAL(recentChanged()), this, SLOT(setupRecentFilesMenu()));

    menu_file->addSeparator(qti_action_FILE_SETTINGS);

    d->actionEditCorpusRepository = new QAction(tr("Edit Corpus Repository Properties..."), this);
    connect(d->actionEditCorpusRepository, SIGNAL(triggered()), SLOT(editCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.EditCorpusRepository", d->actionEditCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    menu_file->addSeparator(qti_action_FILE_SETTINGS);

    d->actionSaveCorpusRepository = new QAction(tr("Save Corpus Repository Properties File"), this);
    connect(d->actionSaveCorpusRepository, SIGNAL(triggered()), SLOT(saveCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.SaveCorpusRepository", d->actionSaveCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    d->actionSaveCorpusRepositoryAs = new QAction(tr("Save Corpus Repository Properties File As..."), this);
    connect(d->actionSaveCorpusRepositoryAs, SIGNAL(triggered()), SLOT(saveCorpusRepositoryAs()));
    command = ACTION_MANAGER->registerAction("File.SaveCorpusRepositoryAs", d->actionSaveCorpusRepositoryAs, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    menu_file->addSeparator(qti_action_FILE_SETTINGS);

    d->actionCloseCorpusRepository = new QAction(tr("Close Corpus Repository"), this);
    connect(d->actionCloseCorpusRepository, SIGNAL(triggered()), SLOT(closeCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.CloseCorpusRepository", d->actionCloseCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    menu_file->addAction(command, qti_action_FILE_SETTINGS);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorpusExplorer = new QAction(tr("Corpus Explorer (trees)"), this);
    connect(d->actionShowCorpusExplorer, SIGNAL(triggered()), SLOT(showCorpusExplorer()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorer", d->actionShowCorpusExplorer, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCorpusExplorerTables = new QAction(tr("Corpus Explorer (tabular)"), this);
    connect(d->actionShowCorpusExplorerTables, SIGNAL(triggered()), SLOT(showCorpusExplorerTables()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorerTables", d->actionShowCorpusExplorerTables, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowMetadataStructureEditor = new QAction(tr("Metadata Structure Editor"), this);
    connect(d->actionShowMetadataStructureEditor, SIGNAL(triggered()), SLOT(showMetadataStructureEditor()));
    command = ACTION_MANAGER->registerAction("Window.ShowMetadataStructureEditor", d->actionShowMetadataStructureEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowAnnotationStructureEditor = new QAction(tr("Annotation Structure Editor"), this);
    connect(d->actionShowAnnotationStructureEditor, SIGNAL(triggered()), SLOT(showAnnotationStructureEditor()));
    command = ACTION_MANAGER->registerAction("Window.ShowAnnotationStructureEditor", d->actionShowAnnotationStructureEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();
}

void CorpusModeWidget::setupRecentFilesMenu()
{
    d->recentFilesMenu->menu()->clear();
    vector<QString> files = d->recentFiles->getRecent();
    for (size_t i = 0; i < files.size(); ++i) {
        QAction *action = new QAction(files[i], this);
        connect(action, SIGNAL(triggered()), this, SLOT(openCorpusRepositoryRecent()));
        if (i == 0) {
            action->setShortcut(tr("Ctrl+R"));
        }
        d->recentFilesMenu->menu()->addAction(action);
    }
}

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
// Corpus repository management
// =========================================================================================================================================

void CorpusModeWidget::newCorpusRepository()
{
    CorpusRepositoryCreateWizard *wizard = new CorpusRepositoryCreateWizard(this);
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
        qDebug() << "WARNING: openRecentFile: sender is not an action" << endl;
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
    if (!d->corpusRepositoriesManager->activeCorpusRepository()) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->activeCorpusRepository();
    CorpusRepositoryPropertiesDialog *dialog = new CorpusRepositoryPropertiesDialog(repository->definition(), this);
    if (dialog->exec() == QDialog::Accepted) {
        repository->setID(dialog->repositoryDef().repositoryID);
        repository->setDescription(dialog->repositoryDef().repositoryName);
        repository->setBasePathMedia(dialog->repositoryDef().basePathMedia);
        if (!repository->definition().filenameDefinition.isEmpty()) {
            repository->definition().save(repository->definition().filenameDefinition);
        }
    }
    delete dialog;
}

void CorpusModeWidget::closeCorpusRepository()
{
    if (!d->corpusRepositoriesManager->activeCorpusRepository()) return;
    QString repositoryID = d->corpusRepositoriesManager->activeCorpusRepository()->ID();
    d->corpusRepositoriesManager->activeCorpusRepository()->close();
    // Remove the repository from the manager. It will automatically activate the next one, if available.
    d->corpusRepositoriesManager->removeCorpusRepository(repositoryID);
}

void CorpusModeWidget::saveCorpusRepository()
{
    if (!d->corpusRepositoriesManager->activeCorpusRepository()) return;
    CorpusRepository *repository = d->corpusRepositoriesManager->activeCorpusRepository();
    if (!repository->definition().filenameDefinition.isEmpty()) {
        repository->definition().save(repository->definition().filenameDefinition);
    } else {
        saveCorpusRepositoryAs();
    }
}

void CorpusModeWidget::saveCorpusRepositoryAs()
{
    if (!d->corpusRepositoriesManager->activeCorpusRepository()) return;
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

