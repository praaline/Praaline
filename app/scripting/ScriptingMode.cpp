#include <QObject>
#include "ScriptingMode.h"
#include "ScriptingModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct ScriptingModePrivateData {
    ScriptingModePrivateData() : widget(0) {}

    QPointer<ScriptingModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

ScriptingMode::ScriptingMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new ScriptingModePrivateData();
    setObjectName(tr("Scripting Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new ScriptingModeWidget();
    connect(d->widget.data(), &ScriptingModeWidget::activateMode, this, &ScriptingMode::activateMode);
}

ScriptingMode::~ScriptingMode()
{
    delete d;
}

QWidget* ScriptingMode::modeWidget() {
    if (!d->widget)
        d->widget = new ScriptingModeWidget();
    return d->widget;
}

void ScriptingMode::initializeMode() {
    if (!d->widget)
        d->widget = new ScriptingModeWidget();
}

QIcon ScriptingMode::modeIcon() const {
    return QIcon(":/icons/modes/scripting_mode_48x48.png");
}

QString ScriptingMode::modeName() const {
    return tr("Scripting");
}

void ScriptingMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_SCRIPTING_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void ScriptingMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
