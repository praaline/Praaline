#include <QList>
#include <QWidget>
#include <QAction>

#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace QtilitiesCoreGui;

#include "ScriptingModeWidget.h"
#include "ui_ScriptingModeWidget.h"

#include "ScriptEditorWidget.h"


struct ScriptingModeWidgetData {
    ScriptingModeWidgetData() {}

    QAction *actionShowScriptEditor;
    QAction *actionShowInteractiveConsole;

    ScriptEditorWidget *widgetScriptEditor;
};

ScriptingModeWidget::ScriptingModeWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::ScriptingModeWidget), d(new ScriptingModeWidgetData())
{
    ui->setupUi(this);

    d->widgetScriptEditor = new ScriptEditorWidget(this);

    ui->gridLayoutScriptEditor->addWidget(d->widgetScriptEditor);

    setupActions();

    connect(ui->commandScriptEditor, &QAbstractButton::clicked, this, &ScriptingModeWidget::showScriptEditor);
    connect(ui->commandInteractiveConsole, &QAbstractButton::clicked, this, &ScriptingModeWidget::showInteractiveConsole);

    ui->stackedWidget->setCurrentIndex(0);
}

ScriptingModeWidget::~ScriptingModeWidget()
{
    delete ui;
    delete d;
}

void ScriptingModeWidget::setupActions()
{
    // Context and menu bar
    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));

    bool existed;
    Command* command;

    ActionContainer* menubar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // WINDOW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowScriptEditor = new QAction(tr("Script Editor"), this);
    connect(d->actionShowScriptEditor, &QAction::triggered, this, &ScriptingModeWidget::showScriptEditor);
    command = ACTION_MANAGER->registerAction("Corpus.ShowScriptEditor", d->actionShowScriptEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowInteractiveConsole = new QAction(tr("Interactive Console"), this);
    connect(d->actionShowInteractiveConsole, &QAction::triggered, this, &ScriptingModeWidget::showInteractiveConsole);
    command = ACTION_MANAGER->registerAction("Corpus.ShowInteractiveConsole", d->actionShowInteractiveConsole, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();
}

void ScriptingModeWidget::showScriptEditor()
{
    ui->stackedWidget->setCurrentIndex(0);
    emit activateMode();
}

void ScriptingModeWidget::showInteractiveConsole()
{
    ui->stackedWidget->setCurrentIndex(1);
    emit activateMode();
}
