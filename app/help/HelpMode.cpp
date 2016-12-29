#include <QObject>
#include "HelpMode.h"
#include "HelpModeConfig.h"
#include "HelpModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct HelpModePrivateData {
    HelpModePrivateData() : widget(0), config(0), mainWindow(0) {}

    QPointer<HelpModeWidget> widget;
    QPointer<HelpModeConfig> config;
    QPointer<QtilitiesMainWindow> mainWindow;
};

HelpMode::HelpMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new HelpModePrivateData();
    setObjectName(tr("Help Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    // Create configuration page
    d->config = new HelpModeConfig();
    OBJECT_MANAGER->registerObject(d->config, QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));

    // Create widget
    if (!d->widget) d->widget = new HelpModeWidget();
    connect(d->widget, SIGNAL(activateMode()), this, SLOT(activateMode()));
}

HelpMode::~HelpMode()
{
    delete d;
}

QWidget* HelpMode::modeWidget() {
    if (!d->widget)
        d->widget = new HelpModeWidget();
    return d->widget;
}

void HelpMode::initializeMode() {
    if (!d->widget)
        d->widget = new HelpModeWidget();
}

QIcon HelpMode::modeIcon() const {
    return QIcon(ICON_HELP_MODE);
}

QString HelpMode::modeName() const {
    return tr("Help");
}

void HelpMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_HELP_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void HelpMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
