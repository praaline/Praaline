#include <QMainWindow>
#include <QHelpContentItem>
#include <QHelpContentModel>
#include <QHelpContentWidget>
#include <QHelpEngine>
#include <QHelpIndexWidget>
#include <QHelpSearchEngine>
#include <QHelpSearchQueryWidget>
#include <QHelpSearchResultWidget>
#include <QUrl>

#include "HelpMode.h"
#include "ContentWidgetFactory.h"
#include "IndexWidgetFactory.h"
#include "SearchWidgetFactory.h"
#include "HelpBrowser.h"

#include "HelpModeWidget.h"

#include <QtilitiesCoreGui>
using namespace QtilitiesCoreGui;

struct HelpModeWidgetData
{
    HelpModeWidgetData() :
        initialized(false),
        side_viewer_dock(0),
        side_viewer_widget(0),
        actionShowDock(0),
        content_widget(0),
        index_widget(0),
        search_widget(0),
        browser(0)
    {}

    bool initialized;
    QDockWidget* side_viewer_dock;
    DynamicSideWidgetViewer* side_viewer_widget;
    QAction* actionShowDock;
    ContentWidgetFactory* content_widget;
    IndexWidgetFactory* index_widget;
    SearchWidgetFactory* search_widget;
    HelpBrowser* browser;
};

HelpModeWidget::HelpModeWidget(QWidget *parent) :
    QMainWindow(parent),
    d(new HelpModeWidgetData)
{
    // Create and dock the dynamic side widget viewer
    d->side_viewer_dock = new QDockWidget(tr("Help Widgets"));
    d->side_viewer_widget = new DynamicSideWidgetViewer(MODE_HELP_ID);
    connect(d->side_viewer_widget,  &Qtilities::CoreGui::DynamicSideWidgetViewer::toggleVisibility,  this, &HelpModeWidget::toggleDock);
    d->side_viewer_dock->setWidget(d->side_viewer_widget);
    Qt::DockWidgetAreas allowed_areas = 0;
    allowed_areas |= Qt::LeftDockWidgetArea;
    allowed_areas |= Qt::RightDockWidgetArea;
    d->side_viewer_dock->setAllowedAreas(allowed_areas);
    addDockWidget(Qt::LeftDockWidgetArea, d->side_viewer_dock);
    d->side_viewer_dock->installEventFilter(this);

    // Actions
    d->actionShowDock = new QAction(QIcon(), tr("Help Widgets"), this);
    d->actionShowDock->setCheckable(true);
    d->actionShowDock->setChecked(true);
    connect(d->side_viewer_widget, &Qtilities::CoreGui::DynamicSideWidgetViewer::toggleVisibility, d->actionShowDock, &QAction::setChecked);
    connect(d->actionShowDock, &QAction::triggered, this, &HelpModeWidget::toggleDock);

    QList<int> context;
    context.push_front(CONTEXT_MANAGER->contextID(qti_def_CONTEXT_STANDARD));
    Command* command = ACTION_MANAGER->registerAction("HelpMode.DynamicDockWidget", d->actionShowDock, context);
    bool existed;
    ActionContainer* view_menu = ACTION_MANAGER->createMenu(qti_action_VIEW, existed);
    if (!existed) {
        ActionContainer* menu_bar = ACTION_MANAGER->createMenuBar(qti_action_MENUBAR_STANDARD, existed);
        menu_bar->addMenu(view_menu, qti_action_ABOUT);
        view_menu->addAction(command);
    } else
        view_menu->addAction(command);

    d->browser = new HelpBrowser(HELP_MANAGER->helpEngine(), QUrl(), this);
    setCentralWidget(d->browser);

    // - Register Contents Widget Factory
    d->content_widget = new ContentWidgetFactory(HELP_MANAGER->helpEngine());
    d->content_widget->setObjectName("Help Plugin: Content Widget");
    d->content_widget->setObjectOriginID("Help Plugin");
    connect(d->content_widget, &ContentWidgetFactory::newWidgetCreated, this, &HelpModeWidget::handleNewHelpWidget);
    OBJECT_MANAGER->registerObject(d->content_widget, QtilitiesCategory("GUI::Side Viewer Widgets (ISideViewerWidget)", "::"));

    // - Register Index Widget Factory
    d->index_widget = new IndexWidgetFactory(HELP_MANAGER->helpEngine());
    d->index_widget->setObjectName("Help Plugin: Index Widget");
    d->index_widget->setObjectOriginID("Help Plugin");
    connect(d->index_widget, &IndexWidgetFactory::newWidgetCreated, this, &HelpModeWidget::handleNewHelpWidget);
    OBJECT_MANAGER->registerObject(d->index_widget, QtilitiesCategory("GUI::Side Viewer Widgets (ISideViewerWidget)", "::"));

    // - Register Search Widget Factory
    QHelpSearchEngine* helpSearchEngine = HELP_MANAGER->helpEngine()->searchEngine();
    d->search_widget = new SearchWidgetFactory(helpSearchEngine);
    d->search_widget->setObjectOriginID("Help Plugin");
    d->search_widget->setObjectName("Help Plugin: Search Engine Widget");
    connect(d->search_widget, &SearchWidgetFactory::newWidgetCreated, this, &HelpModeWidget::handleNewHelpWidget);
    OBJECT_MANAGER->registerObject(d->search_widget, QtilitiesCategory("GUI::Side Viewer Widgets (ISideViewerWidget)", "::"));

    // Load the home page:
    if (HELP_MANAGER->homePage().isValid())
        d->browser->helpTextBrowser()->setSource(HELP_MANAGER->homePage());

    connect(HELP_MANAGER, &Qtilities::CoreGui::HelpManager::forwardRequestUrlDisplay, this, &HelpModeWidget::handleUrlRequest, Qt::UniqueConnection);
    connect(HELP_MANAGER, &Qtilities::CoreGui::HelpManager::homePageChanged, this, &HelpModeWidget::handleHomePageChanged);

    // Check all objects in the global object pool.
    QMap<QString, ISideViewerWidget*> text_iface_map;
    QList<QObject*> widgets = OBJECT_MANAGER->registeredInterfaces("ISideViewerWidget");
    for (int i = 0; i < widgets.count(); ++i) {
        ISideViewerWidget* side_viewer_widget = qobject_cast<ISideViewerWidget*> (widgets.at(i));
        if (side_viewer_widget) {
            text_iface_map[side_viewer_widget->widgetLabel()] = side_viewer_widget;
        }
    }

    d->side_viewer_widget->setIFaceMap(text_iface_map, true);
}

HelpModeWidget::~HelpModeWidget()
{
    delete d;
}

bool HelpModeWidget::eventFilter(QObject *object,  QEvent *event) {
    if (object == d->side_viewer_dock && event->type() == QEvent::Close) {
        d->actionShowDock->setChecked(false);
    }
    return false;
}

void HelpModeWidget::toggleDock(bool toggle) {
    if (toggle) {
        d->side_viewer_dock->show();
    } else {
        d->side_viewer_dock->hide();
    }
}

void HelpModeWidget::handleNewHelpWidget(QWidget* widget) {
    // Check which widget was created:
    QHelpContentWidget* content_widget = qobject_cast<QHelpContentWidget*> (widget);
    if (content_widget) {
        connect(content_widget, &QHelpContentWidget::linkActivated, this, &HelpModeWidget::handleUrl, Qt::UniqueConnection);
        return;
    }
    QHelpIndexWidget* index_widget = qobject_cast<QHelpIndexWidget*> (widget);
    if (index_widget) {
        connect(index_widget, &QHelpIndexWidget::linkActivated, this, &HelpModeWidget::handleUrl, Qt::UniqueConnection);
        return;
    }
    QHelpSearchResultWidget* result_widget = qobject_cast<QHelpSearchResultWidget*> (widget);
    if (result_widget) {
        connect(result_widget, &QHelpSearchResultWidget::requestShowLink, this, &HelpModeWidget::handleUrl, Qt::UniqueConnection);
        return;
    }
}

void HelpModeWidget::handleUrl(const QUrl& url) {
    d->browser->helpTextBrowser()->setSource(url);
}

void HelpModeWidget::handleUrlRequest(const QUrl &url,  bool ensure_visible) {
    handleUrl(url);
    if (ensure_visible) emit activateMode();
}

void HelpModeWidget::handleHomePageChanged(const QUrl &url) {
    if (d->browser && url.isValid())
        d->browser->helpTextBrowser()->setSource(url);
}
