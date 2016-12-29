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

struct CorpusModeWidgetData {
    CorpusModeWidgetData()
    { }

    QAction *actionShowCorpusExplorer;
    QAction *actionShowCorpusExplorerTables;
    QAction *actionShowMetadataStructureEditor;
    QAction *actionShowAnnotationStructureEditor;

    CorpusExplorerWidget *widgetCorpusExplorer;
    CorpusExplorerTableWidget *widgetCorpusExplorerTables;
    MetadataStructureEditor *widgetMetadataStructureEditor;
    AnnotationStructureEditor *widgetAnnotationStructureEditor;
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

    setupActions();

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->commandCorpusExplorer, SIGNAL(clicked()), this, SLOT(showCorpusExplorer()));
    connect(ui->commandCorpusExplorerTables, SIGNAL(clicked()), this, SLOT(showCorpusExplorerTables()));
    connect(ui->commandMetadataStructureEditor, SIGNAL(clicked()), this, SLOT(showMetadataStructureEditor()));
    connect(ui->commandAnnotationStructureEditor, SIGNAL(clicked()), this, SLOT(showAnnotationStructureEditor()));
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
