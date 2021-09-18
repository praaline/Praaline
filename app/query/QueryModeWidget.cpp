#include <QList>
#include <QWidget>
#include <QAction>

#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace QtilitiesCoreGui;

#include "QueryModeWidget.h"
#include "ui_QueryModeWidget.h"

#include "ConcordancerWidget.h"
#include "ConcordancerQuickWidget.h"
#include "CreateDatasetWidget.h"
#include "AdvancedQueriesWidget.h"
#include "ExtractSoundBitesWidget.h"

struct QueryModeWidgetData {
    QueryModeWidgetData() {}

    QAction *actionShowCorcondancer;
    QAction *actionShowCreateDataset;
    QAction *actionShowAnnotationBrowser;
    QAction *actionShowAdvancedQueries;
    QAction *actionShowExtractSamples;

    ConcordancerQuickWidget *widgetConcordancer;
    CreateDatasetWidget *widgetCreateDataset;
    AdvancedQueriesWidget *widgetAdvancedQueries;
    ExtractSoundBitesWidget *widgetExtractSamples;
};

QueryModeWidget::QueryModeWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::QueryModeWidget), d(new QueryModeWidgetData())
{
    ui->setupUi(this);

    d->widgetConcordancer = new ConcordancerQuickWidget(this);
    d->widgetCreateDataset = new CreateDatasetWidget(this);
    d->widgetAdvancedQueries = new AdvancedQueriesWidget(this);
    d->widgetExtractSamples = new ExtractSoundBitesWidget(this);

    ui->gridLayoutConcordancer->addWidget(d->widgetConcordancer);
    ui->gridLayoutCreateDataset->addWidget(d->widgetCreateDataset);
    ui->gridLayoutAdvancedQueries->addWidget(d->widgetAdvancedQueries);
    ui->gridLayoutExtractSoundBites->addWidget(d->widgetExtractSamples);

    setupActions();

    connect(ui->commandConcordancer, &QAbstractButton::clicked, this, &QueryModeWidget::showConcordancer);
    connect(ui->commandCreateDataset, &QAbstractButton::clicked, this, &QueryModeWidget::showCreateDataset);
    connect(ui->commandAdvancedQueries, &QAbstractButton::clicked, this, &QueryModeWidget::showAdvancedQueries);
    connect(ui->commandExtractSamples, &QAbstractButton::clicked, this, &QueryModeWidget::showExtractSamples);

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
    // WINDOW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowCorcondancer = new QAction(tr("Concordancer"), this);
    connect(d->actionShowCorcondancer, &QAction::triggered, this, &QueryModeWidget::showConcordancer);
    command = ACTION_MANAGER->registerAction("Corpus.ShowConcordancer", d->actionShowCorcondancer, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowCreateDataset = new QAction(tr("Dataset Creator"), this);
    connect(d->actionShowCreateDataset, &QAction::triggered, this, &QueryModeWidget::showCreateDataset);
    command = ACTION_MANAGER->registerAction("Corpus.ShowCreateDataset", d->actionShowCreateDataset, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowAdvancedQueries = new QAction(tr("Advanced Queries"), this);
    connect(d->actionShowAdvancedQueries, &QAction::triggered, this, &QueryModeWidget::showAdvancedQueries);
    command = ACTION_MANAGER->registerAction("Corpus.ShowAdvancedQueries", d->actionShowAdvancedQueries, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowExtractSamples = new QAction(tr("Sample Extraction Tool"), this);
    connect(d->actionShowExtractSamples, &QAction::triggered, this, &QueryModeWidget::showExtractSamples);
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
