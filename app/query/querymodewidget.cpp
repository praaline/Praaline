#include "querymodewidget.h"
#include "ui_querymodewidget.h"

#include "advancedquerieswidget.h"
#include "concordancerwidget.h"
#include "createdatasetwidget.h"

#include "concordancerquickwidget.h"

#include "extractsoundbiteswidget.h"

struct QueryModeWidgetData {
    QueryModeWidgetData() {}

    QAction *actionShowCorcondancer;
    QAction *actionShowCreateDataset;
    QAction *actionShowAdvancedQueries;
    QAction *actionShowExtractSamples;

    ConcordancerQuickWidget *widgetConcordancer;
    CreateDatasetWidget *widgetCreateDataset;
    AdvancedQueriesWidget *widgetAdvancedQueries;
    ExtractSoundBitesWidget *widgetExtractSamples;
};

QueryModeWidget::QueryModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QueryModeWidget)
{
    ui->setupUi(this);
    d = new QueryModeWidgetData();

    d->widgetConcordancer = new ConcordancerQuickWidget(this);
    d->widgetCreateDataset = new CreateDatasetWidget(this);
    d->widgetAdvancedQueries = new AdvancedQueriesWidget(this);
    d->widgetExtractSamples = new ExtractSoundBitesWidget(this);

    ui->gridLayoutConcordancer->addWidget(d->widgetConcordancer);
    ui->gridLayoutCreateDataset->addWidget(d->widgetCreateDataset);
    ui->gridLayoutAdvancedQueries->addWidget(d->widgetAdvancedQueries);
    ui->gridLayoutExtractSoundBites->addWidget(d->widgetExtractSamples);

    setupActions();

    connect(ui->commandConcordancer, SIGNAL(clicked()), this, SLOT(showConcordancer()));
    connect(ui->commandCreateDataset, SIGNAL(clicked()), this, SLOT(showCreateDataset()));
    connect(ui->commandAdvancedQueries, SIGNAL(clicked()), this, SLOT(showAdvancedQueries()));
    connect(ui->commandExtractSamples, SIGNAL(clicked(bool)), this, SLOT(showExtractSamples()));

    ui->stackedWidget->setCurrentIndex(0);
}

QueryModeWidget::~QueryModeWidget()
{
    delete ui;
    delete d;
}

void QueryModeWidget::setupActions()
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
    d->actionShowCorcondancer = new QAction("Show Concordancer", this);
    connect(d->actionShowCorcondancer, SIGNAL(triggered()), SLOT(showConcordancer()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowConcordancer", d->actionShowCorcondancer, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    d->actionShowCreateDataset = new QAction("Show Create Dataset window", this);
    connect(d->actionShowCreateDataset, SIGNAL(triggered()), SLOT(showCreateDataset()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowCreateDataset", d->actionShowCreateDataset, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    d->actionShowAdvancedQueries = new QAction("Show Advanced Queries window", this);
    connect(d->actionShowAdvancedQueries, SIGNAL(triggered()), SLOT(showAdvancedQueries()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowAdvancedQueries", d->actionShowAdvancedQueries, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    d->actionShowExtractSamples = new QAction("Show Extract Samples window", this);
    connect(d->actionShowExtractSamples, SIGNAL(triggered()), SLOT(showExtractSamples()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowExtractSamples", d->actionShowExtractSamples, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    view_menu->addSeperator();
}

void QueryModeWidget::showConcordancer()
{
    ui->stackedWidget->setCurrentIndex(0);
    emit activateMode();
}

void QueryModeWidget::showCreateDataset()
{
    ui->stackedWidget->setCurrentIndex(1);
    emit activateMode();
}

void QueryModeWidget::showAdvancedQueries()
{
    ui->stackedWidget->setCurrentIndex(2);
    emit activateMode();
}

void QueryModeWidget::showExtractSamples()
{
    ui->stackedWidget->setCurrentIndex(3);
    emit activateMode();
}
