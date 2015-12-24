#include <QString>
#include <QIcon>
#include <QApplication>
#include <QStyleFactory>
#include <QDebug>

#include "svgui/layer/SVLayersInitialiser.h"
#include "pngui/layer/PraalineLayersInitialiser.h"

#include "../external/qtilities/include/QtilitiesLogging/QtilitiesLogging"
#include "../external/qtilities/include/QtilitiesCore/QtilitiesCore"
#include "../external/qtilities/include/QtilitiesCoreGui/QtilitiesCoreGui"
#include "../external/qtilities/include/QtilitiesExtensionSystem/QtilitiesExtensionSystem"
#include "../external/qtilities/include/QtilitiesProjectManagement/QtilitiesProjectManagement"

// Application Modes
#include "corpus/corpusmode.h"
#include "annotation/annotationmode.h"
#include "visualisation/visualisationmode.h"
#include "query/querymode.h"
#include "statistics/statisticsmode.h"
#include "scripting/scriptingmode.h"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;
using namespace QtilitiesExtensionSystem;
using namespace QtilitiesProjectManagement;
using namespace Praaline;

#include "corporamanager.h"

int main(int argc, char *argv[])
{
    // Initialize application object, using Qtilities instead of QApplication
    QtilitiesApplication a(argc, argv);
    QtilitiesApplication::setOrganizationName("praaline.org");
    QtilitiesApplication::setOrganizationDomain("Praaline");
    QtilitiesApplication::setApplicationName("Praaline");
    QtilitiesApplication::setApplicationVersion("0.1");

//    qApp->setStyle(QStyleFactory::create("Fusion"));
//    QPalette darkPalette;
//    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
//    darkPalette.setColor(QPalette::WindowText, Qt::white);
//    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
//    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
//    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
//    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
//    darkPalette.setColor(QPalette::Text, Qt::white);
//    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
//    darkPalette.setColor(QPalette::ButtonText, Qt::white);
//    darkPalette.setColor(QPalette::BrightText, Qt::red);
//    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
//    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
//    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
//    qApp->setPalette(darkPalette);
//    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    // Create a QtilitiesMainWindow. This window will show Praaline's different modes (e.g. Corpus, Annotation...)
    QtilitiesMainWindow praalineMainWindow(QtilitiesMainWindow::ModesLeft);
    QtilitiesApplication::setMainWindow(&praalineMainWindow);

    // Set application icon
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/icons/praaline-alpha.png"), QSize(), QIcon::Normal, QIcon::Off);
    praalineMainWindow.setWindowIcon(icon);

    // Create the configuration widget
    ConfigurationWidget config_widget;
    config_widget.setCategorizedTabDisplay(true);
    QtilitiesApplication::setConfigWidget(&config_widget);

    // Initialize the logger
    Log->setLoggerSessionConfigPath(QtilitiesApplication::applicationSessionPath());
    LOG_INITIALIZE();
    Log->setIsQtMessageHandler(false);

    // Add a formatting hint to the RichText formatting engine.
    AbstractFormattingEngine* rich_text_engine = Log->formattingEngineReference(qti_def_FORMATTING_ENGINE_RICH_TEXT);
    if (rich_text_engine) {
        QRegExp reg_exp_success_color = QRegExp(QObject::tr("Successfully") + "*", Qt::CaseInsensitive, QRegExp::Wildcard);
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

    // Initialize the clipboard manager:
    CLIPBOARD_MANAGER->initialize();

    // Create the main menu
    bool existed;
    ActionContainer* menu_bar = ACTION_MANAGER->createMenuBar(qti_action_MENUBAR_STANDARD, existed);
    praalineMainWindow.setMenuBar(menu_bar->menuBar());
    ActionContainer* file_menu = ACTION_MANAGER->createMenu(qti_action_FILE, existed);
    ActionContainer* edit_menu = ACTION_MANAGER->createMenu(qti_action_EDIT, existed);
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    ActionContainer* help_menu = ACTION_MANAGER->createMenu(qti_action_HELP, existed);
    menu_bar->addMenu(file_menu);
    menu_bar->addMenu(edit_menu);
    menu_bar->addMenu(view_menu);
    menu_bar->addMenu(help_menu);

    // Get the standard context
    QList<int> std_context;
    std_context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    // Register action place holders for this application. This allows control of the menu structure:
    // File menu
    Command* command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_FILE_SETTINGS, QObject::tr("Settings"), QKeySequence(), std_context);
    QObject::connect(command->action(), SIGNAL(triggered()), &config_widget, SLOT(show()));
    file_menu->addAction(command);
    file_menu->addSeperator();
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_FILE_EXIT, QObject::tr("Exit"), QKeySequence(QKeySequence::Quit), std_context);
    QObject::connect(command->action(), SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));
    file_menu->addAction(command);
    // Edit menu
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_UNDO, QObject::tr("Undo"), QKeySequence(QKeySequence::Undo));
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_REDO, QObject::tr("Redo"), QKeySequence(QKeySequence::Redo));
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    edit_menu->addSeperator();
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_COPY, QObject::tr("Copy"), QKeySequence(QKeySequence::Copy));
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_CUT, QObject::tr("Cut"), QKeySequence(QKeySequence::Cut));
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    command = ACTION_MANAGER->command(qti_action_EDIT_PASTE);
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    edit_menu->addSeperator();
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_SELECT_ALL, QObject::tr("Select All"), QKeySequence(QKeySequence::SelectAll));
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_CLEAR, QObject::tr("Clear"));
    command->setCategory(QtilitiesCategory("Editing"));
    edit_menu->addAction(command);
    edit_menu->addSeperator();
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_EDIT_FIND, QObject::tr("Find"), QKeySequence(QKeySequence::Find));
    edit_menu->addAction(command);
    // View menu is empty
    // Help menu
    command = ACTION_MANAGER->registerActionPlaceHolder(qti_action_ABOUT, QObject::tr("About Praaline"), QKeySequence(), std_context);
    help_menu->addAction(command);

    #ifdef QT_NO_DEBUG
    // splash->clearMessage();
    #endif

    // CORPORA MANAGER AND MAIN WINDOW
    // ============================================================================================
    CorporaManager *corporaManager = new CorporaManager();
    OBJECT_MANAGER->registerObject(corporaManager, QtilitiesCategory("CorporaManager"));

    // Register main window in object manager
    OBJECT_MANAGER->registerObject(&praalineMainWindow, QtilitiesCategory("PraalineMainWindow"));

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
//    VisualisationMode* visualisation_mode = new VisualisationMode();
//    CONTEXT_MANAGER->registerContext(visualisation_mode->contextString());
//    OBJECT_MANAGER->registerObject(visualisation_mode, QtilitiesCategory("GUI::Application Modes (IMode)", "::"));

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

    // ============================================================================================

    // Now that all the modes have been loaded from the plugins, add them to the main window:
    praalineMainWindow.modeManager()->initialize();
    QStringList mode_order;
    mode_order << "Corpus" << "Annotation" << "Visualisation" << "Query" << "Statistics" << "Scripting";
    mode_order << "Session Log";
    praalineMainWindow.modeManager()->setPreferredModeOrder(mode_order);
    praalineMainWindow.modeManager()->setActiveMode("Corpus");

    // Register command editor config page.
    OBJECT_MANAGER->registerObject(ACTION_MANAGER->commandEditor(), QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));
    // Register extension system config page.
    OBJECT_MANAGER->registerObject(EXTENSION_SYSTEM->configWidget(), QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));
    // Initialize the config widget:
    config_widget.initialize();

    // Load the previous session's keyboard mapping file.
    QString shortcut_mapping_file = QString("%1/%2").arg(QtilitiesApplication::applicationSessionPath()).arg(qti_def_PATH_SHORTCUTS_FILE);
    ACTION_MANAGER->loadShortcutMapping(shortcut_mapping_file);

//    #if cocoa
//    Melder_setTracing (true);
//    #endif
//    praat_setLogo (130, 80, logo);
//    praat_init ("Praaline", argc, argv);
//    INCLUDE_LIBRARY (praat_uvafon_init)
//    INCLUDE_LIBRARY (praat_contrib_Ola_KNN_init)
//    praat_run ();

    // Show the main window:
    praalineMainWindow.readSettings();
    praalineMainWindow.showMaximized();

    // Initialize the project manager:
    PROJECT_MANAGER_INITIALIZE();

    ACTION_MANAGER->commandObserver()->endProcessingCycle(false);
    ACTION_MANAGER->actionContainerObserver()->endProcessingCycle(false);
    OBJECT_MANAGER->objectPool()->endProcessingCycle(false);

    // RUN THE APPLICATION
    int result = a.exec();

    // AFTER APPLICATION SHUTDOWN

    // Save main window state:
    praalineMainWindow.writeSettings();

    // Save the current keyboard mapping for the next session.
    ACTION_MANAGER->saveShortcutMapping(shortcut_mapping_file);

    PROJECT_MANAGER_FINALIZE();
    LOG_FINALIZE();
    EXTENSION_SYSTEM->finalize();
    return result;
}
