#include "corpusmodewidget.h"
#include "ui_corpusmodewidget.h"

#include "../external/qtilities/include/QtilitiesCore/QtilitiesCore"
#include "../external/qtilities/include/QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "corpusexplorerwidget.h"
#include "corpusexplorertablewidget.h"
#include "corpusstructureeditor/corpusstructureeditorwidget.h"

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

    ActionContainer* menu_bar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    if (!existed) menu_bar->addMenu(view_menu, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorpusExplorer = new QAction("Show Corpus Explorer", this);
    connect(d->actionShowCorpusExplorer, SIGNAL(triggered()), SLOT(showCorpusExplorer()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowCorpusExplorer", d->actionShowCorpusExplorer, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    d->actionShowCorpusExplorerTables = new QAction("Show Corpus Explorer (tabular)", this);
    connect(d->actionShowCorpusExplorerTables, SIGNAL(triggered()), SLOT(showCorpusExplorerTables()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowCorpusExplorerTables", d->actionShowCorpusExplorerTables, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    d->actionShowCorpusStructureEditor = new QAction("Show Corpus Structure Editor", this);
    connect(d->actionShowCorpusStructureEditor, SIGNAL(triggered()), SLOT(showCorpusStructureEditor()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowCorpusStructureEditor", d->actionShowCorpusStructureEditor, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    view_menu->addSeperator();
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

