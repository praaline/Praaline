#include "CorpusModeWidget.h"
#include "ui_CorpusModeWidget.h"

#include "../external/qtilities/include/QtilitiesCore/QtilitiesCore"
#include "../external/qtilities/include/QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "CorpusExplorerWidget.h"
#include "CorpusExplorerTableWidget.h"
#include "corpusstructureeditor/CorpusStructureEditorWidget.h"

struct CorpusModeWidgetData {
    CorpusModeWidgetData()
    { }

    QAction *actionShowCorpusExplorer;
    QAction *actionShowCorpusExplorerTables;
    QAction *actionShowCorpusStructureEditor;

    CorpusExplorerWidget *widgetCorpusExplorer;
    CorpusExplorerTableWidget *widgetCorpusExplorerTables;
    CorpusStructureEditorWidget *widgetCorpusStructureEditor;
};

CorpusModeWidget::CorpusModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CorpusModeWidget), d(new CorpusModeWidgetData)
{
    ui->setupUi(this);

    d->widgetCorpusExplorer = new CorpusExplorerWidget(this);
    d->widgetCorpusExplorerTables = new CorpusExplorerTableWidget(this);
    d->widgetCorpusStructureEditor = new CorpusStructureEditorWidget(this);

    ui->gridLayoutCorpusExplorer->addWidget(d->widgetCorpusExplorer);
    ui->gridLayoutCorpusExplorerTables->addWidget(d->widgetCorpusExplorerTables);
    ui->gridLayoutCorpusStructureEditor->addWidget(d->widgetCorpusStructureEditor);

    setupActions();

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->commandCorpusExplorer, SIGNAL(clicked()), this, SLOT(showCorpusExplorer()));
    connect(ui->commandCorpusExplorerTables, SIGNAL(clicked()), this, SLOT(showCorpusExplorerTables()));
    connect(ui->commandCorpusStructureEditor, SIGNAL(clicked()), this, SLOT(showCorpusStructureEditor()));
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
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, tr("&Window"));

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorpusExplorer = new QAction("Show Corpus Explorer", this);
    connect(d->actionShowCorpusExplorer, SIGNAL(triggered()), SLOT(showCorpusExplorer()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorer", d->actionShowCorpusExplorer, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCorpusExplorerTables = new QAction("Show Corpus Explorer (tabular)", this);
    connect(d->actionShowCorpusExplorerTables, SIGNAL(triggered()), SLOT(showCorpusExplorerTables()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusExplorerTables", d->actionShowCorpusExplorerTables, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCorpusStructureEditor = new QAction("Show Corpus Structure Editor", this);
    connect(d->actionShowCorpusStructureEditor, SIGNAL(triggered()), SLOT(showCorpusStructureEditor()));
    command = ACTION_MANAGER->registerAction("Window.ShowCorpusStructureEditor", d->actionShowCorpusStructureEditor, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeperator();
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

void CorpusModeWidget::showCorpusStructureEditor()
{
    ui->stackedWidget->setCurrentIndex(2);
    emit activateMode();
}

