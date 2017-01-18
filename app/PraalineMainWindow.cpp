#include <QString>
#include <QIcon>
#include <QStyleFactory>
#include <QProcess>
#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>

#include "svcore/base/UnitDatabase.h"
#include "svcore/base/Preferences.h"
#include "svgui/layer/ColourDatabase.h"
#include "svgui/widgets/KeyReference.h"
#include "svgui/widgets/ActivityLog.h"
#include "svgui/widgets/UnitConverter.h"
#include "svgui/widgets/IconLoader.h"
#include "svgui/widgets/CommandHistory.h"

#include "PraalineMainWindow.h"

#include "svgui/layer/SVLayersInitialiser.h"
#include "pngui/layer/PraalineLayersInitialiser.h"
#include "QtilitiesLogging/QtilitiesLogging"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
#include "QtilitiesExtensionSystem/QtilitiesExtensionSystem"
#include "QtilitiesProjectManagement/QtilitiesProjectManagement"

// Application Modes
#include "corpus/CorpusMode.h"
#include "annotation/AnnotationMode.h"
#include "visualisation/VisualisationMode.h"
#include "query/QueryMode.h"
#include "statistics/StatisticsMode.h"
#include "scripting/ScriptingMode.h"
#include "help/HelpMode.h"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;
using namespace QtilitiesExtensionSystem;
using namespace QtilitiesProjectManagement;
using namespace Praaline;

#include "CorpusRepositoriesManager.h"

struct PraalineMainWindowData {
    PraalineMainWindowData() : usingDarkPalette(false) {}

    QString shortcut_mapping_file;
    ConfigurationWidget *configurationWidget;
    ActivityLog         *activityLog;
    UnitConverter       *unitConverter;
    KeyReference        *keyReference;

    ActionContainer *menubar;
    ActionContainer *menu_file;
    ActionContainer *menu_edit;
    ActionContainer *menu_view;
    ActionContainer *menu_corpus;
    ActionContainer *menu_annotation;
    ActionContainer *menu_visualisation;
    ActionContainer *menu_playback;
    ActionContainer *menu_window;
    ActionContainer *menu_help;

    bool usingDarkPalette;
};

PraalineMainWindow::PraalineMainWindow(QtilitiesMainWindow *mainWindow, QObject *parent) :
    QObject(parent), m_mainWindow(mainWindow), d(new PraalineMainWindowData())
{
    d->configurationWidget = new ConfigurationWidget();
    d->activityLog = new ActivityLog();
    d->unitConverter = new UnitConverter();
    d->keyReference = new KeyReference();
    d->unitConverter->hide();
}

PraalineMainWindow::~PraalineMainWindow()
{
    delete d->configurationWidget;
    delete d->keyReference;
    delete d->activityLog;
    delete d->unitConverter;
    delete d;
}

void PraalineMainWindow::initialise()
{
    // Set application icon
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/praaline-alpha.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_mainWindow->setWindowIcon(icon);

    // Create the configuration widget
    d->configurationWidget->setCategorizedTabDisplay(true);
    QtilitiesApplication::setConfigWidget(d->configurationWidget);

    // Initialize the logger
    Log->setLoggerSessionConfigPath(QtilitiesApplication::applicationSessionPath());
    LOG_INITIALIZE();
    Log->setIsQtMessageHandler(false);

    // Add a formatting hint to the RichText formatting engine.
    AbstractFormattingEngine* rich_text_engine = Log->formattingEngineReference(qti_def_FORMATTING_ENGINE_RICH_TEXT);
    if (rich_text_engine) {
        QRegExp reg_exp_success_color = QRegExp(tr("Successfully") + "*", Qt::CaseInsensitive, QRegExp::Wildcard);
        CustomFormattingHint success_message_hint(reg_exp_success_color, "green", Logger::Info);
        rich_text_engine->addColorFormattingHint(success_message_hint);
    }

    // Speed up application launching a bit...
    ACTION_MANAGER->commandObserver()->startProcessingCycle();
    ACTION_MANAGER->actionContainerObserver()->startProcessingCycle();
    OBJECT_MANAGER->objectPool()->startProcessingCycle();

    // We show a splash screen in this example:
    #ifdef QT_NO_DEBUG
//    QPixmap pixmap(QTILITIES_LOGO_BT_300x300);
//    QSplashScreen *splash = new QSplashScreen(pixmap);
//    splash->setWindowFlags(Qt::SplashScreen | Qt::WindowStaysOnTopHint);
//    splash->show();
//    QObject::connect(EXTENSION_SYSTEM,SIGNAL(newProgressMessage(QString)),splash,SLOT(showMessage(QString)));
//    a.processEvents();
    #endif

    // Initialise basic layers: SV and Praaline
    SVLayersInitialiser::initialise();
    PraalineLayersInitialiser::initialise();

    // Initialize the clipboard manager
    CLIPBOARD_MANAGER->initialize();

    // Initialise Unit Database
    UnitDatabase *udb = UnitDatabase::getInstance();
    udb->registerUnit("Hz");
    udb->registerUnit("dB");
    udb->registerUnit("s");
    udb->registerUnit("ST");

    // Initialise the Colours Database
    ColourDatabase *cdb = ColourDatabase::getInstance();
    cdb->addColour(Qt::black, tr("Black"));
    cdb->addColour(Qt::darkRed, tr("Red"));
    cdb->addColour(Qt::darkBlue, tr("Blue"));
    cdb->addColour(Qt::darkGreen, tr("Green"));
    cdb->addColour(QColor(200, 50, 255), tr("Purple"));
    cdb->addColour(QColor(255, 150, 50), tr("Orange"));
    cdb->setUseDarkBackground(cdb->addColour(Qt::white, tr("White")), true);
    cdb->setUseDarkBackground(cdb->addColour(Qt::red, tr("Bright Red")), true);
    cdb->setUseDarkBackground(cdb->addColour(QColor(30, 150, 255), tr("Bright Blue")), true);
    cdb->setUseDarkBackground(cdb->addColour(Qt::green, tr("Bright Green")), true);
    cdb->setUseDarkBackground(cdb->addColour(QColor(225, 74, 255), tr("Bright Purple")), true);
    cdb->setUseDarkBackground(cdb->addColour(QColor(255, 188, 80), tr("Bright Orange")), true);

    // Initialise Sonic Visualiser preferences
    Preferences *pref = Preferences::getInstance();
    pref->setResampleOnLoad(true);
    pref->setFixedSampleRate(44100);
    pref->setSpectrogramSmoothing(Preferences::SpectrogramInterpolated);
    pref->setNormaliseAudio(true);

    // Create the main menu bar and its menus
    setupMenuBar();
    setupFileMenu();
    setupEditMenu();
    setupViewMenu();
    setupCorpusMenu();
    setupAnnotationMenu();
    setupVisualisationMenu();
    setupPlaybackMenu();
    setupHelpMenu();

    #ifdef QT_NO_DEBUG
    // splash->clearMessage();
    #endif

    // REGISTER GLOBALLY AVAILABLE OBJECTS
    // ============================================================================================
    CorpusRepositoriesManager *corpusRepositoryManager = new CorpusRepositoriesManager();
    OBJECT_MANAGER->registerObject(corpusRepositoryManager, QtilitiesCategory("Corpus"));

    // Register main window in object manager
    OBJECT_MANAGER->registerObject(m_mainWindow, QtilitiesCategory("PraalineMainWindow"));

    // Register KeyReference and ActivityLog
    OBJECT_MANAGER->registerObject(d->keyReference, QtilitiesCategory("KeyReference"));
    OBJECT_MANAGER->registerObject(d->activityLog, QtilitiesCategory("ActivityLog"));

    // HELP MANAGER
    // Register main help file. Plug-ins may register their help files.
    HELP_MANAGER->registerFile(QApplication::applicationDirPath() + "/praaline.qch", false);

    // PLUGINS
    // ============================================================================================
    // Load plugins using the extension system
    Log->toggleQtMsgEngine(true);
    EXTENSION_SYSTEM->enablePluginActivityControl();
    EXTENSION_SYSTEM->loadPluginConfiguration(QDir::homePath() + "/Praaline/plugins/default" + qti_def_SUFFIX_PLUGIN_CONFIG);
    EXTENSION_SYSTEM->setCorePlugins(QStringList("Session Log Plugin"));
    EXTENSION_SYSTEM->addPluginPath(QDir::homePath() + "/Praaline/plugins/");
    EXTENSION_SYSTEM->initialize();
    Log->toggleQtMsgEngine(false);

    // INITIALISE COMPONENTS
    // ============================================================================================
    // Initialise the help manager. Plugins had their chance to add help files to the pool.
    // This initialisation should happen before creating the Help interface mode.
    HELP_MANAGER->initialize();

    // Initialise the project manager
    PROJECT_MANAGER_INITIALIZE();

    // CONFIGURATION PAGES
    // ============================================================================================
    // Register command editor config page
    OBJECT_MANAGER->registerObject(ACTION_MANAGER->commandEditor(), QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));
    // Register extension system config page
    OBJECT_MANAGER->registerObject(EXTENSION_SYSTEM->configWidget(), QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));
    // Logging configuration page
    OBJECT_MANAGER->registerObject(LoggerGui::createLoggerConfigWidget(), QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));

    // USER INTERFACE MODES
    // ============================================================================================
    // Create an instance of the Corpus Mode
    CorpusMode* corpus_mode = new CorpusMode();
    CONTEXT_MANAGER->registerContext(corpus_mode->contextString());
    OBJECT_MANAGER->registerObject(corpus_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // Create an instance of the Annotation Mode
    AnnotationMode* annotation_mode = new AnnotationMode();
    CONTEXT_MANAGER->registerContext(annotation_mode->contextString());
    OBJECT_MANAGER->registerObject(annotation_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // Create an instance of the Visualisation Mode
    VisualisationMode* visualisation_mode = new VisualisationMode();
    CONTEXT_MANAGER->registerContext(visualisation_mode->contextString());
    OBJECT_MANAGER->registerObject(visualisation_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // Create an instance of the Query Mode
    QueryMode* query_mode = new QueryMode();
    CONTEXT_MANAGER->registerContext(query_mode->contextString());
    OBJECT_MANAGER->registerObject(query_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // Create an instance of the Statistics Mode
    StatisticsMode* statistics_mode = new StatisticsMode();
    CONTEXT_MANAGER->registerContext(statistics_mode->contextString());
    OBJECT_MANAGER->registerObject(statistics_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // Create an instance of the Scripting Mode
    ScriptingMode* scripting_mode = new ScriptingMode();
    CONTEXT_MANAGER->registerContext(scripting_mode->contextString());
    OBJECT_MANAGER->registerObject(scripting_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // Create an instance of the Help Mode
    HelpMode* help_mode = new HelpMode();
    CONTEXT_MANAGER->registerContext(help_mode->contextString());
    OBJECT_MANAGER->registerObject(help_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

    // ============================================================================================

    // Add tools to the Window menu, after all modes
    setupWindowMenu();

    // Now that all the modes have been loaded from the plugins, add them to the main window:
    m_mainWindow->modeManager()->initialize();
    QStringList mode_order;
    mode_order << tr("Corpus") << tr("Annotation") << tr("Visualisation") << tr("Query") << tr("Statistics") << tr("Scripting");
    mode_order << tr("Help") << tr("Session Log");
    m_mainWindow->modeManager()->setPreferredModeOrder(mode_order);
    m_mainWindow->modeManager()->setActiveMode(MODE_CORPUS_ID);

    // Initialize the config widget:
    d->configurationWidget->initialize();

    // Load the previous session's keyboard mapping file.
    d->shortcut_mapping_file = QString("%1/%2").arg(QtilitiesApplication::applicationSessionPath()).arg(qti_def_PATH_SHORTCUTS_FILE);
    ACTION_MANAGER->loadShortcutMapping(d->shortcut_mapping_file);

//    #if cocoa
//    Melder_setTracing (true);
//    #endif
//    praat_setLogo (130, 80, logo);
//    praat_init ("Praaline", argc, argv);
//    INCLUDE_LIBRARY (praat_uvafon_init)
//    INCLUDE_LIBRARY (praat_contrib_Ola_KNN_init)
//    praat_run ();

    // Show the main window:
    m_mainWindow->readSettings();
    m_mainWindow->showMaximized();

    ACTION_MANAGER->commandObserver()->endProcessingCycle(false);
    ACTION_MANAGER->actionContainerObserver()->endProcessingCycle(false);
    OBJECT_MANAGER->objectPool()->endProcessingCycle(false);
}

void PraalineMainWindow::finalise()
{
    // APPLICATION SHUTDOWN
    // Save main window state:
    m_mainWindow->writeSettings();
    // Save the current keyboard mapping for the next session.
    ACTION_MANAGER->saveShortcutMapping(d->shortcut_mapping_file);
    PROJECT_MANAGER_FINALIZE();
    LOG_FINALIZE();
    EXTENSION_SYSTEM->finalize();
}

// ==============================================================================================================================
// COLOUR PALETTES
// ==============================================================================================================================

void PraalineMainWindow::toggleColourPalette()
{
    if (d->usingDarkPalette)
        selectLightPalette();
    else
        selectDarkPalette();
}

void PraalineMainWindow::selectLightPalette()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(m_mainWindow->style()->standardPalette());
    qApp->setStyleSheet("");
    // Special stylesheet for the application mode list
    QString stylesheet = "";
    // Give the view a colored background:
    stylesheet += "QListView { background-color: #FFFFFF; border-style: none; }";
    // The text underneath the unselected items:
    stylesheet += "QListView::item::text { font-weight: bold; border-style: none; color: black }";
    // The text underneath the selected item:
    stylesheet += "QListView::item:selected:active { font-weight: bold; border-style: none; color: white }";
    stylesheet += "QListView::item:selected:!active { font-weight: bold; border-style: none; color: white }";
    // Hover effect:
    stylesheet += "QListView::item:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 white, stop: 0.4 #EEEEEE, stop:1 #CCCCCC); }";
    // Selected item gradient:
    stylesheet += "QListView::item:selected:active { background: #CCCCCC; }";
    stylesheet += "QListView::item:selected:!active { background: #CCCCCC; }";
    // The padding of items in the list:
    if (m_mainWindow->modeLayout() == QtilitiesMainWindow::ModesLeft || m_mainWindow->modeLayout() == QtilitiesMainWindow::ModesRight)
        stylesheet += "QListView::item { padding: 5px 1px 5px 1px;}";
    else
        stylesheet += "QListView::item { padding: 5px 0px 5px 0px;}";
    m_mainWindow->modeManager()->modeListWidget()->setStyleSheet(stylesheet);
    d->usingDarkPalette = false;
}

void PraalineMainWindow::selectDarkPalette()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkPalette.setColor(QPalette::Background, QColor(53,53,53));
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; } ");
    // Special stylesheet for the application mode list
    QString stylesheet = "";
    // Give the view a colored background:
    stylesheet += "QListView { background-color: #252525; border-style: none; }";
    // The text underneath the unselected items:
    stylesheet += "QListView::item::text { font-weight: bold; border-style: none; color: white }";
    // The text underneath the selected item:
    stylesheet += "QListView::item:selected:active { font-weight: bold; border-style: none; color: black }";
    stylesheet += "QListView::item:selected:!active { font-weight: bold; border-style: none; color: black }";
    // Hover effect:
    stylesheet += "QListView::item:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 white, stop: 0.4 #EEEEEE, stop:1 #CCCCCC); }";
    // Selected item gradient:
    stylesheet += "QListView::item:selected:active { background: #CCCCCC; }";
    stylesheet += "QListView::item:selected:!active { background: #CCCCCC; }";
    // The padding of items in the list:
    if (m_mainWindow->modeLayout() == QtilitiesMainWindow::ModesLeft || m_mainWindow->modeLayout() == QtilitiesMainWindow::ModesRight)
        stylesheet += "QListView::item { padding: 5px 1px 5px 1px;}";
    else
        stylesheet += "QListView::item { padding: 5px 0px 5px 0px;}";
    m_mainWindow->modeManager()->modeListWidget()->setStyleSheet(stylesheet);
    d->usingDarkPalette = true;
}

// ==============================================================================================================================
// MAIN MENU
// ==============================================================================================================================

void PraalineMainWindow::setupMenuBar()
{
    bool existed;
    d->menubar = ACTION_MANAGER->createMenuBar(qti_action_MENUBAR_STANDARD, existed);
    m_mainWindow->setMenuBar(d->menubar->menuBar());
    d->menu_file            = ACTION_MANAGER->createMenu(tr("&File"), existed);
    d->menu_edit            = ACTION_MANAGER->createMenu(tr("&Edit"), existed);
    d->menu_view            = ACTION_MANAGER->createMenu(tr("&View"), existed);
    d->menu_corpus          = ACTION_MANAGER->createMenu(tr("&Corpus"), existed);
    d->menu_annotation      = ACTION_MANAGER->createMenu(tr("&Annotation"), existed);
    d->menu_visualisation   = ACTION_MANAGER->createMenu(tr("V&isualisation"), existed);
    d->menu_playback        = ACTION_MANAGER->createMenu(tr("&Playback"), existed);
    d->menu_window          = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    d->menu_help            = ACTION_MANAGER->createMenu(tr("&Help"), existed);
    d->menubar->addMenu(d->menu_file);
    d->menubar->addMenu(d->menu_edit);
    d->menubar->addMenu(d->menu_view);
    d->menubar->addMenu(d->menu_corpus);
    d->menubar->addMenu(d->menu_annotation);
    d->menubar->addMenu(d->menu_visualisation);
    d->menubar->addMenu(d->menu_playback);
    d->menubar->addMenu(d->menu_window);
    d->menubar->addMenu(d->menu_help);
    d->menu_file->menu()->setTearOffEnabled(true);
    d->menu_edit->menu()->setTearOffEnabled(true);
    d->menu_view->menu()->setTearOffEnabled(true);
    d->menu_corpus->menu()->setTearOffEnabled(true);
    d->menu_annotation->menu()->setTearOffEnabled(true);
    d->menu_visualisation->menu()->setTearOffEnabled(true);
    d->menu_playback->menu()->setTearOffEnabled(true);
    d->menu_window->menu()->setTearOffEnabled(true);
    d->menu_help->menu()->setTearOffEnabled(true);
}

void PraalineMainWindow::setupFileMenu()
{
    QList<int> std_context;
    std_context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    // File menu
    Command* command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_FILE_SETTINGS, tr("&Settings"), QKeySequence(), std_context);
    connect(command->action(), SIGNAL(triggered()), d->configurationWidget, SLOT(show()));
    d->menu_file->addAction(command);
    d->menu_file->addSeparator();
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_FILE_EXIT, tr("&Quit"), QKeySequence(QKeySequence::Quit), std_context);
    connect(command->action(), SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));
    d->menu_file->addAction(command);
}

void PraalineMainWindow::setupEditMenu()
{
    // Edit menu
    Command* command;
    // Undo and Redo from CommandHistory signleton
    CommandHistory::getInstance()->registerMenu(d->menu_edit->menu());
    d->menu_edit->addSeparator();
    command = ACTION_MANAGER->registerActionPlaceHolder("Edit.Cut", QObject::tr("Cu&t"), QKeySequence(QKeySequence::Cut));
    command->setCategory(QtilitiesCategory("Editing"));
    d->menu_edit->addAction(command);
    command = ACTION_MANAGER->registerActionPlaceHolder("Edit.Copy", QObject::tr("&Copy"), QKeySequence(QKeySequence::Copy));
    command->setCategory(QtilitiesCategory("Editing"));
    d->menu_edit->addAction(command);
    command = ACTION_MANAGER->registerActionPlaceHolder("Edit.Paste", QObject::tr("&Paste"), QKeySequence(QKeySequence::Paste));
    command->setCategory(QtilitiesCategory("Editing"));
    d->menu_edit->addAction(command);
    d->menu_edit->addSeparator();
    command = ACTION_MANAGER->registerActionPlaceHolder("Edit.SelectAll", QObject::tr("Select &All"), QKeySequence(QKeySequence::SelectAll));
    command->setCategory(QtilitiesCategory("Editing"));
    d->menu_edit->addAction(command);
    command = ACTION_MANAGER->registerActionPlaceHolder("Edit.Clear", QObject::tr("C&lear"));
    command->setCategory(QtilitiesCategory("Editing"));
    d->menu_edit->addAction(command);
    d->menu_edit->addSeparator();
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_FIND, QObject::tr("&Find"), QKeySequence(QKeySequence::Find));
    d->menu_edit->addAction(command);
}

void PraalineMainWindow::setupViewMenu()
{
    // View menu
    IconLoader il;
    QAction *action;
    Command *command;
    QList<int> std_context;
    std_context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    // Select colour palette
    action = new QAction(tr("Change Colour Palette (Light/Dark)"), this);
    action->setStatusTip(tr("Change the display colours used in Praaline between a light and a dark palette."));
    connect(action, SIGNAL(triggered()), this, SLOT(toggleColourPalette()));
    command = ACTION_MANAGER->registerAction("View.ChangePalette", action, std_context);
    command->setCategory(QtilitiesCategory("View"));
    d->menu_view->addAction(command);
}

void PraalineMainWindow::setupCorpusMenu()
{

}

void PraalineMainWindow::setupAnnotationMenu()
{

}

void PraalineMainWindow::setupVisualisationMenu()
{

}

void PraalineMainWindow::setupPlaybackMenu()
{
    IconLoader il;
    Command* command;
    // Play / Pause
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.PlayPause", tr("Play / Pause"),
                                                        QKeySequence(Qt::Key_Tab), QList<int>(), il.load("playpause"));
    command->action()->setCheckable(true);
    command->action()->setStatusTip(tr("Start or stop playback from the current position"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Play Selection
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.PlaySelection", tr("Constrain Playback to Selection"),
                                                        QKeySequence(tr("Ctrl+Alt+S")), QList<int>(), il.load("playselection"));
    command->action()->setCheckable(true);
    command->action()->setStatusTip(tr("Constrain playback to the selected regions"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Play Loop
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.PlayLoop", tr("Loop Playback"),
                                                        QKeySequence(tr("Ctrl+Alt+L")), QList<int>(), il.load("playloop"));
    command->action()->setCheckable(true);
    command->action()->setStatusTip(tr("Loop playback"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Play Solo
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.PlaySolo", tr("Solo Current Pane"),
                                                        QKeySequence(tr("Ctrl+Alt+O")), QList<int>(), il.load("solo"));
    command->action()->setCheckable(true);
    command->action()->setStatusTip(tr("Solo the current pane during playback"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Align
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.Align", tr("Align File Timelines"),
                                                        QKeySequence(), QList<int>(), il.load("align"));
    command->action()->setCheckable(true);
    command->action()->setStatusTip(tr("Treat multiple audio files as versions of the same recording, and align their timelines"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    d->menu_playback->addSeparator();
    // Rewind
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.Rewind", tr("Rewind"),
                                                        QKeySequence(tr("PgUp")), QList<int>(), il.load("rewind"));
    command->action()->setStatusTip(tr("Rewind to the previous time instant or time ruler notch"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Fast forward
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.FastForward", tr("Fast Forward"),
                                                        QKeySequence(tr("PgDown")), QList<int>(), il.load("ffwd"));
    command->action()->setStatusTip(tr("Fast-forward to the next time instant or time ruler notch"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    d->menu_playback->addSeparator();
    // Rewind to similar
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.RewindSimilar", tr("Rewind to Similar Point"),
                                                        QKeySequence(tr("Shift+PgUp")), QList<int>());
    command->action()->setStatusTip(tr("Rewind to the previous similarly valued time instant"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Fast forward to similar
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.FastForwardSimilar", tr("Fast Forward to Similar Point"),
                                                        QKeySequence(tr("Shift+PgDown")), QList<int>());
    command->action()->setStatusTip(tr("Fast-forward to the next similarly valued time instant"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    d->menu_playback->addSeparator();
    // Rewind to start
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.RewindStart", tr("Rewind to Start"),
                                                        QKeySequence(tr("Home")), QList<int>(), il.load("rewind-start"));
    command->action()->setStatusTip(tr("Rewind to the start"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Fast forward to end
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.FastForwardEnd", tr("Fast Forward to End"),
                                                        QKeySequence(tr("End")), QList<int>(), il.load("ffwd-end"));
    command->action()->setStatusTip(tr("Fast-forward to the end"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    d->menu_playback->addSeparator();
    // Speed up
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.SpeedUp", tr("Speed Up"),
                                                        QKeySequence(tr("Ctrl+Alt+PgUp")), QList<int>());
    command->action()->setStatusTip(tr("Time-stretch playback to speed it up without changing pitch"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Slow down
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.SlowDown", tr("Slow Down"),
                                                        QKeySequence(tr("Ctrl+Alt+PgDown")), QList<int>());
    command->action()->setStatusTip(tr("Time-stretch playback to slow it down without changing pitch"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
    // Restore speed
    command = ACTION_MANAGER->registerActionPlaceHolder("Playback.RestoreNormalSpeed", tr("Restore Normal Speed"),
                                                        QKeySequence(tr("Ctrl+Alt+Home")), QList<int>());
    command->action()->setStatusTip(tr("Restore non-time-stretched playback"));
    command->setCategory(QtilitiesCategory("Playback"));
    d->menu_playback->addAction(command);
}

void PraalineMainWindow::setupWindowMenu()
{
    QAction *action;
    Command *command;
    QList<int> std_context;
    std_context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    // Window menu - these have to be added after all modes
    // Activity log
    action = new QAction(tr("Acti&vity Log"), this);
    action->setStatusTip(tr("Open a window listing interactions and other events"));
    connect(action, SIGNAL(triggered()), this, SLOT(showActivityLog()));
    command = ACTION_MANAGER->registerAction("Window.Activity Log", action, std_context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    d->menu_window->addAction(command);
    // Unit converter
    action = new QAction(tr("&Unit Converter"), this);
    action->setStatusTip(tr("Open a window of pitch and timing conversion utilities"));
    connect(action, SIGNAL(triggered()), this, SLOT(showUnitConverter()));
    command = ACTION_MANAGER->registerAction("Window.UnitConverter", action, std_context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    d->menu_window->addAction(command);
}

void PraalineMainWindow::setupHelpMenu()
{
    IconLoader il;
    QAction *action;
    Command *command;
    QList<int> std_context;
    std_context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    // Help menu
    d->keyReference->setCategory(tr("Help"));
    QString name = QApplication::applicationName();
    // Help reference
    action = new QAction(il.load("help"), tr("&Help Reference"), this);
    action->setShortcut(tr("F1"));
    action->setStatusTip(tr("Open the %1 reference manual").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(showHelp()));
    d->keyReference->registerShortcut(action);
    command = ACTION_MANAGER->registerAction("Help.Reference", action, std_context);
    command->setCategory(QtilitiesCategory("Help"));
    d->menu_help->addAction(command);
    // Key and Mouse reference
    action = new QAction(tr("&Key and Mouse Reference"), this);
    action->setShortcut(tr("F12"));
    action->setStatusTip(tr("Open a window showing the keystrokes you can use in %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(showKeyReference()));
    d->keyReference->registerShortcut(action);
    command = ACTION_MANAGER->registerAction("Help.KeyReference", action, std_context);
    command->setCategory(QtilitiesCategory("Help"));
    d->menu_help->addAction(command);
    // Website
    action = new QAction(tr("%1 on the &Web").arg(name), this);
    action->setStatusTip(tr("Open the %1 website").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(showWebsite()));
    command = ACTION_MANAGER->registerAction("Help.Website", action, std_context);
    command->setCategory(QtilitiesCategory("Help"));
    d->menu_help->addAction(command);
    // About
    action = new QAction(tr("&About %1").arg(name), this);
    action->setStatusTip(tr("Show information about %1").arg(name));
    connect(action, SIGNAL(triggered()), this, SLOT(showAbout()));
    command = ACTION_MANAGER->registerAction("Help.About", action, std_context);
    command->setCategory(QtilitiesCategory("Help"));
    d->menu_help->addAction(command);
}

// ==============================================================================================================================
// HELP AND ABOUT
// ==============================================================================================================================

void PraalineMainWindow::showHelp()
{
    m_mainWindow->modeManager()->setActiveMode(MODE_HELP_ID);
}

void PraalineMainWindow::showAbout()
{
    bool debug = false;
    QString version = "(unknown version)";

#ifdef QT_DEBUG
    debug = true;
#endif
    version = tr("Release %1").arg(QApplication::instance()->applicationVersion());

    QString aboutText;

    aboutText += tr("<h3>About Praaline</h3>");
    aboutText += tr("<p>Praaline is an open-source tool for language corpus management, annotation, visualisation and analysis.</p>");
    aboutText += tr("<p>%1 : %2 configuration</p>")
        .arg(version)
        .arg(debug ? tr("Debug") : tr("Release"));
    aboutText += tr("<p>Using Qt framework version %1.</p>")
        .arg(QT_VERSION_STR);

    aboutText +=
        "<p>Copyright &copy; 2012&ndash;2015 George Christodoulides</p>"
        "<p>Additional copyright notices go here</p>"
        "<p>Copyright &copy; 2005&ndash;2014 Chris Cannam, Queen Mary University of London</p>"
        "<p>This program is free software; you can redistribute it and/or "
        "modify it under the terms of the GNU General Public License as "
        "published by the Free Software Foundation; either version 2 of the "
        "License, or (at your option) any later version.<br>See the file "
        "COPYING included with this distribution for more information.</p>";

    QMessageBox::about(m_mainWindow, tr("About %1").arg(QApplication::applicationName()), aboutText);
}

void PraalineMainWindow::showWebsite()
{
    QString url = tr("http://www.praaline.org");
    QDesktopServices::openUrl(QUrl(url));
}

void PraalineMainWindow::showKeyReference()
{
    d->keyReference->show();
}

void PraalineMainWindow::showActivityLog()
{
    d->activityLog->show();
    d->activityLog->raise();
    d->activityLog->scrollToEnd();
}

void PraalineMainWindow::showUnitConverter()
{
    d->unitConverter->show();
    d->unitConverter->raise();
}


