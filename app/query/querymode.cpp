#include <QObject>
#include "querymode.h"
#include "querymodewidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct QueryModePrivateData {
    QueryModePrivateData() : widget(0) {}

    QPointer<QueryModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

QueryMode::QueryMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new QueryModePrivateData();
    setObjectName(tr("Query Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new QueryModeWidget();
    connect(d->widget, SIGNAL(activateMode()), this, SLOT(activateMode()));
}

QueryMode::~QueryMode()
{
    delete d;
}

QWidget* QueryMode::modeWidget() {
    if (!d->widget)
        d->widget = new QueryModeWidget();
    return d->widget;
}

void QueryMode::initializeMode() {
    if (!d->widget)
        d->widget = new QueryModeWidget();
}

QIcon QueryMode::modeIcon() const {
    return QIcon(":/icons/modes/query_mode_48x48.png");
}

QString QueryMode::modeName() const {
    return tr("Query");
}

void QueryMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_QUERY_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void QueryMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
