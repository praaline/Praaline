#include <QObject>
#include "WelcomeMode.h"
#include "WelcomeModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct WelcomeModePrivateData {
    WelcomeModePrivateData() : widget(0), mainWindow(0) {}

    QPointer<WelcomeModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

WelcomeMode::WelcomeMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new WelcomeModePrivateData();
    setObjectName(tr("Welcome Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new WelcomeModeWidget();
    connect(d->widget, SIGNAL(activateMode()), this, SLOT(activateMode()));
}

WelcomeMode::~WelcomeMode()
{
    delete d;
}

QWidget* WelcomeMode::modeWidget() {
    if (!d->widget)
        d->widget = new WelcomeModeWidget();
    return d->widget;
}

void WelcomeMode::initializeMode() {
    if (!d->widget)
        d->widget = new WelcomeModeWidget();
}

QIcon WelcomeMode::modeIcon() const {
    return QIcon(":/icons/modes/welcome_mode_48x48.png");
}

QString WelcomeMode::modeName() const {
    return tr("Welcome");
}

void WelcomeMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_WELCOME_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void WelcomeMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
