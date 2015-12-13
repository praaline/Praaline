#include "visualisationmodewidget.h"
#include "ui_visualisationmodewidget.h"

#include "../external/qtilities/include/QtilitiesCore/QtilitiesCore"
#include "../external/qtilities/include/QtilitiesCoreGui/QtilitiesCoreGui"
using namespace Qtilities;
using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

// Contained widgets
#include "timelinevisualisationwidget.h"
#include "globalvisualisationwidget.h"

#include "svcore/base/UnitDatabase.h"
#include "svgui/layer/ColourDatabase.h"
#include "svcore/base/Preferences.h"

struct VisualisationModeWidgetData {
    VisualisationModeWidgetData()
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

    Preferences *pref = Preferences::getInstance();
    pref->setResampleOnLoad(true);
    pref->setFixedSampleRate(44100);
    pref->setSpectrogramSmoothing(Preferences::SpectrogramInterpolated);
    pref->setNormaliseAudio(true);

    UnitDatabase *udb = UnitDatabase::getInstance();
    udb->registerUnit("Hz");
    udb->registerUnit("dB");
    udb->registerUnit("s");

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

    setupActions();

    ui->stackedWidget->setCurrentIndex(0);

    connect(ui->commandTimelineVisualisation, SIGNAL(clicked()), this, SLOT(showTimelineVisualisation()));
    connect(ui->commandGlobalVisualisation, SIGNAL(clicked()), this, SLOT(showGlobalVisualisation()));
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

    ActionContainer* menu_bar = ACTION_MANAGER->menuBar(qti_action_MENUBAR_STANDARD);
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    if (!existed) menu_bar->addMenu(view_menu, qti_action_HELP);

    // ------------------------------------------------------------------------------------------------------
    // VIEW MENU
    // ------------------------------------------------------------------------------------------------------
    d->actionShowTimelineVisualisation = new QAction("Show Timeline Visualiser", this);
    connect(d->actionShowTimelineVisualisation, SIGNAL(triggered()), SLOT(showTimelineVisualisation()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowTimeline", d->actionShowTimelineVisualisation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    d->actionShowGlobalVisualisation = new QAction("Show Corpus-level Visualiser", this);
    connect(d->actionShowGlobalVisualisation, SIGNAL(triggered()), SLOT(showGlobalVisualisation()));
    command = ACTION_MANAGER->registerAction("Annotation.ShowGlobal", d->actionShowGlobalVisualisation, context);
    command->setCategory(QtilitiesCategory(QApplication::applicationName()));
    view_menu->addAction(command);

    view_menu->addSeperator();
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


