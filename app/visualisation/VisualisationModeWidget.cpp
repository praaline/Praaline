#include "VisualisationModeWidget.h"
#include "ui_VisualisationModeWidget.h"

#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "TimelineVisualisationWidget.h"
#include "GlobalVisualisationWidget.h"

struct VisualisationModeWidgetData {
    VisualisationModeWidgetData() :
        widgetTimelineVisualisation(0), actionShowGlobalVisualisation(0)
    { }

    QAction *actionShowTimelineVisualisation;
    QAction *actionShowGlobalVisualisation;

    TimelineVisualisationWidget *widgetTimelineVisualisation;
    GlobalVisualisationWidget *widgetGlobalVisualisation;
};


VisualisationModeWidget::VisualisationModeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VisualisationModeWidget), d(new VisualisationModeWidgetData)
{
    ui->setupUi(this);

    d->widgetTimelineVisualisation = new TimelineVisualisationWidget(this);
    d->widgetGlobalVisualisation = new GlobalVisualisationWidget(this);

    ui->gridLayoutTimelineVisualisation->addWidget(d->widgetTimelineVisualisation);
    ui->gridLayoutGlobalVisualisation->addWidget(d->widgetGlobalVisualisation);

    setupActions();

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->commandTimelineVisualisation, &QAbstractButton::clicked, this, &VisualisationModeWidget::showTimelineVisualisation);
    connect(ui->commandGlobalVisualisation, &QAbstractButton::clicked, this, &VisualisationModeWidget::showGlobalVisualisation);
}

VisualisationModeWidget::~VisualisationModeWidget()
{
    delete ui;
    if (d->widgetTimelineVisualisation) delete d->widgetTimelineVisualisation;
    delete d;
}

void VisualisationModeWidget::setupActions()
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
    d->actionShowTimelineVisualisation = new QAction("Timeline Visualiser", this);
    connect(d->actionShowTimelineVisualisation, &QAction::triggered, this, &VisualisationModeWidget::showTimelineVisualisation);
    command = ACTION_MANAGER->registerAction("Annotation.ShowTimeline", d->actionShowTimelineVisualisation, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    d->actionShowGlobalVisualisation = new QAction("Corpus-level Visualiser", this);
    connect(d->actionShowGlobalVisualisation, &QAction::triggered, this, &VisualisationModeWidget::showGlobalVisualisation);
    command = ACTION_MANAGER->registerAction("Annotation.ShowGlobal", d->actionShowGlobalVisualisation, context);
    command->setCategory(QtilitiesCategory(tr("Active Window Selection")));
    menu_window->addAction(command);

    menu_window->addSeparator();
}

void VisualisationModeWidget::showTimelineVisualisation()
{
    ui->stackedWidget->setCurrentIndex(0);
    emit activateMode();
}

void VisualisationModeWidget::showGlobalVisualisation()
{
    ui->stackedWidget->setCurrentIndex(1);
    emit activateMode();
}


