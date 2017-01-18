#include "CorpusModeWidget.h"
#include "ui_CorpusModeWidget.h"

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "CorpusExplorerWidget.h"
#include "CorpusExplorerTableWidget.h"
#include "structureeditors/MetadataStructureEditor.h"
#include "structureeditors/AnnotationStructureEditor.h"

#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/CorpusRepositoryDefinition.h"
using namespace Praaline::Core;

#include "svcore/base/RecentFiles.h"
using std::vector;

#include "CorpusRepositoriesManager.h"
#include "NewCorpusRepositoryWizard.h"

struct CorpusModeWidgetData {
    CorpusModeWidgetData() :
        recentFiles(0), recentFilesMenu(0)
    { }

    QAction *actionNewCorpusRepository;
    QAction *actionOpenCorpusRepository;
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
    ActionContainer* file_menu = ACTION_MANAGER->menu(qti_action_FILE);
    Q_ASSERT(file_menu);
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, tr("&Window"));

    // ------------------------------------------------------------------------------------------------------
    // FILE MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionNewCorpusRepository = new QAction(tr("New Corpus Repository..."), this);
    connect(d->actionNewCorpusRepository, SIGNAL(triggered()), SLOT(newCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.NewCorpusRepository", d->actionNewCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionOpenCorpusRepository = new QAction(tr("Open Corpus Repository..."), this);
    connect(d->actionOpenCorpusRepository, SIGNAL(triggered()), SLOT(openCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.OpenCorpusRepository", d->actionOpenCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->recentFilesMenu = ACTION_MANAGER->createMenu(tr("Open &Recent Corpus Repository"), existed);
    if (!existed) file_menu->addMenu(d->recentFilesMenu, qti_action_FILE_SETTINGS);
    setupRecentFilesMenu();
    connect(d->recentFiles, SIGNAL(recentChanged()), this, SLOT(setupRecentFilesMenu()));

    file_menu->addSeparator(qti_action_FILE_SETTINGS);

    d->actionSaveCorpusRepository = new QAction(tr("Save Corpus Repository Access File"), this);
    connect(d->actionSaveCorpusRepository, SIGNAL(triggered()), SLOT(saveCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.SaveCorpusRepository", d->actionSaveCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionSaveCorpusRepositoryAs = new QAction(tr("Save Corpus Repository Access File As..."), this);
    connect(d->actionSaveCorpusRepositoryAs, SIGNAL(triggered()), SLOT(saveCorpusRepositoryAs()));
    command = ACTION_MANAGER->registerAction("File.SaveCorpusRepositoryAs", d->actionSaveCorpusRepositoryAs, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    d->actionCloseCorpusRepository = new QAction(tr("Close Corpus Repository"), this);
    connect(d->actionCloseCorpusRepository, SIGNAL(triggered()), SLOT(closeCorpusRepository()));
    command = ACTION_MANAGER->registerAction("File.CloseCorpusRepository", d->actionCloseCorpusRepository, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    file_menu->addAction(command, qti_action_FILE_SETTINGS);

    file_menu->addSeparator(qti_action_FILE_SETTINGS);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorpusExplorer = new QAction(tr("Show Corpus Explorer"), this);
    connect(d->actionShowCorpusExplorer, SIGNAL(triggered()), SLOT(showCorpusExplorer()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorer", d->actionShowCorpusExplorer, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCorpusExplorerTables = new QAction(tr("Show Corpus Explorer (tabular)"), this);
    connect(d->actionShowCorpusExplorerTables, SIGNAL(triggered()), SLOT(showCorpusExplorerTables()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorerTables", d->actionShowCorpusExplorerTables, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowMetadataStructureEditor = new QAction(tr("Show Metadata Structure Editor"), this);
    connect(d->actionShowMetadataStructureEditor, SIGNAL(triggered()), SLOT(showMetadataStructureEditor()));
    command = ACTION_MANAGER->registerAction("Window.ShowMetadataStructureEditor", d->actionShowMetadataStructureEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowAnnotationStructureEditor = new QAction(tr("Show Annotation Structure Editor"), this);
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
    NewCorpusRepositoryWizard *wizard = new NewCorpusRepositoryWizard(this);
    wizard->exec();
    CorpusRepository *repository = wizard->newCorpusRepository();
    if (!repository) return;
    d->corpusRepositoriesManager->addCorpusRepository(repository);
    d->corpusRepositoriesManager->setActiveCorpusRepository(repository->ID());
}

void CorpusModeWidget::openCorpusRepository()
{
//    QFileDialog::Options options;
//    QString selectedFilter;
//    QString filename = QFileDialog::getOpenFileName(this, tr("Open Corpus"), "",
//                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"),
//                                                    &selectedFilter, options);
//    if (filename.isEmpty()) return;
//    openCorpusFile(filename);
}

void CorpusModeWidget::openCorpusRepositoryRecent()
{
//    QObject *obj = sender();
//    QAction *action = dynamic_cast<QAction *>(obj);
//    if (!action) {
//        qDebug() << "WARNING: openRecentFile: sender is not an action" << endl;
//        return;
//    }
//    QString filename = action->text();
//    if (filename.isEmpty()) return;
//    openCorpusFile(filename);
}

CorpusRepository *CorpusModeWidget::openCorpusRepository(const QString &filename, CorpusRepositoryDefinition &definition)
{

}


//Corpus *CorpusModeWidget::openCorpus(const QString &filename, CorpusDefinition &definition)
//{
//    // Open the corpus definition
//    if (!definition.load(filename)) {
//        QMessageBox::warning(this, tr("Cannot open corpus"),
//                             QString(tr("Cannot open corpus definition file (%1). Is it a valid Praaline XML corpus definition?")).arg(filename),
//                             QMessageBox::Ok);
//        return 0;
//    }
//    // Ask for password, if needed
//    QString password;
//    if (definition.datastoreMetadata.usePassword == true || definition.datastoreAnnotations.usePassword) {
//        password = QInputDialog::getText(this, tr("Enter password"), tr("Enter password to connect to this corpus"),
//                                         QLineEdit::Password);
//        if (definition.datastoreMetadata.usePassword) definition.datastoreMetadata.password = password;
//        if (definition.datastoreAnnotations.usePassword) definition.datastoreAnnotations.password = password;
//    }
//    // Attempt to open corpus
//    QString errorMessages;
//    Corpus *corpus = Corpus::open(definition, errorMessages);
//    if (!errorMessages.isEmpty()) {
//        QMessageBox::warning(this, tr("Error opening corpus"), errorMessages, QMessageBox::Ok);
//        return 0;
//    }
//    return corpus;
//}

//void CorpusModeWidget::openCorpusFile(const QString &filename)
//{
//    // Check whether the corpus is already open
//    QList<QObject *> listCorpora;
//    listCorpora = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
//    int countCorpora = 0;
//    foreach (QObject* obj, listCorpora) {
//        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
//        if (obs) {
//            countCorpora++;
//            if (obs->definition().filenameDefinition == filename) {
//                QMessageBox::warning(this, tr("Corpus already open"),
//                                     QString(tr("This corpus (%1) is already open.")).arg(filename), QMessageBox::Ok);
//                return;
//            }
//        }
//    }
//    // Attempt to open corpus
//    CorpusDefinition definition;
//    Corpus *corpus = openCorpus(filename, definition);
//    if (!corpus) return;
//    // Register corpus with the Corpus Manager and global object pool
//    CorpusObserver *cobs = new CorpusObserver(corpus);
//    cobs->setDefinition(definition);
//    OBJECT_MANAGER->registerObject(cobs, QtilitiesCategory("Corpus"));
//    d->corporaTopLevelNode->addNode(cobs->nodeCorpus());
//    d->corporaManager->addCorpus(corpus);
//    d->corporaManager->setActiveCorpus(corpus->ID());
//    //cobs->setCommunicationsGrouping(QStringList() << "type");
//    d->recentFiles->addFile(filename);
//}

//void CorpusModeWidget::openCorpusDbConnection()
//{
//    QPointer<CorpusDatabaseConnectionDialog> dialog = new CorpusDatabaseConnectionDialog(this);
//    dialog->exec();
//    CorpusDefinition definition = dialog->corpusDefinition();
//    QString errorMessages;
//    Corpus *corpus = Corpus::open(definition, errorMessages);
//    if (!errorMessages.isEmpty()) {
//        QMessageBox::warning(this, tr("Error opening corpus"), errorMessages, QMessageBox::Ok);
//        return;
//    }
//    if (!corpus) return;
//    // Register corpus with the Corpus Manager and global object pool
//    CorpusObserver *cobs = new CorpusObserver(corpus);
//    cobs->setDefinition(definition);
//    OBJECT_MANAGER->registerObject(cobs, QtilitiesCategory("Corpus"));
//    d->corporaTopLevelNode->addNode(cobs->nodeCorpus());
//    d->corporaManager->addCorpus(corpus);
//    d->corporaManager->setActiveCorpus(corpus->ID());
//}

void CorpusModeWidget::closeCorpusRepository()
{
//    // Corpus Explorer observer
//    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
//    if (!obs) return;
//    obs->corpus()->close();
//    d->corporaTopLevelNode->removeItem(d->corporaManager->activeCorpusID());
//    d->corporaManager->removeCorpus(d->corporaManager->activeCorpusID());
//    OBJECT_MANAGER->removeObject(obs);
//    delete obs;
//    // Metadata editors
//    d->metadataEditorPrimary->clear();
//    d->metadataEditorSecondary->clear();
//    // Activate next corpus (if available)
//    QList<QObject *> listCorpora = OBJECT_MANAGER->registeredInterfaces("CorpusObserver");
//    foreach (QObject* obj, listCorpora) {
//        CorpusObserver *obs = qobject_cast<CorpusObserver *>(obj);
//        if (obs && obs->corpus()) {
//            d->corporaManager->setActiveCorpus(obs->corpus()->ID());
//            break;
//        }
//    }
}

void CorpusModeWidget::saveCorpusRepository()
{
//    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
//    if (!obs) return;
//    if (!obs->corpus()) return;
//    if (!obs->definition().filenameDefinition.isEmpty()) {
//        obs->corpus()->save();
//    } else {
//        // database corpus
//        saveCorpusAs();
//    }
}

void CorpusModeWidget::saveCorpusRepositoryAs()
{
//    CorpusObserver *obs = d->corporaManager->activeCorpusObserver();
//    if (!obs) return;
//    if (!obs->corpus()) return;
//    QFileDialog::Options options;
//    QString selectedFilter;
//    QString filename = QFileDialog::getSaveFileName(this, tr("Save Corpus As..."), obs->definition().filenameDefinition,
//                                                    tr("Praaline Corpus File (*.corpus);;All Files (*)"), &selectedFilter, options);
//    if (filename.isEmpty()) return;
//    obs->definition().save(filename);
//    obs->corpus()->save();
//    // else show error message
//    // m_mainStatusBar->showMessage("Saved corpus \"" + fileName, 5000);
}

