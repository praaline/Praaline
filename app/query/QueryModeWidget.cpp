#include "QueryModeWidget.h"
#include "ui_QueryModeWidget.h"

#include "AdvancedQueriesWidget.h"
#include "ConcordancerWidget.h"
#include "CreateDatasetWidget.h"
#include "ConcordancerQuickWidget.h"
#include "ExtractSoundBitesWidget.h"

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

    ActionContainer* menubar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* menu_window = ACTION_MANAGER->createMenu(tr("&Window"), existed);
    if (!existed) menubar->addMenu(menu_window, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorcondancer = new QAction(tr("Concordancer"), this);
    connect(d->actionShowCorcondancer, SIGNAL(triggered()), SLOT(showConcordancer()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowConcordancer", d->actionShowCorcondancer, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCreateDataset = new QAction(tr("Dataset Creator"), this);
    connect(d->actionShowCreateDataset, SIGNAL(triggered()), SLOT(showCreateDataset()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowCreateDataset", d->actionShowCreateDataset, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowAdvancedQueries = new QAction(tr("Advanced Queries"), this);
    connect(d->actionShowAdvancedQueries, SIGNAL(triggered()), SLOT(showAdvancedQueries()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowAdvancedQueries", d->actionShowAdvancedQueries, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowExtractSamples = new QAction(tr("Sample Extraction Tool"), this);
    connect(d->actionShowExtractSamples, SIGNAL(triggered()), SLOT(showExtractSamples()));
    command = ACTION_MANAGER->registerAction("Corpus.ShowExtractSamples", d->actionShowExtractSamples, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();
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
