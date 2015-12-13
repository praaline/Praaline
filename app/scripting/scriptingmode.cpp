#include <QObject>
#include "scriptingmode.h"
#include "scriptingmodewidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct ScriptingModePrivateData {
    ScriptingModePrivateData() : widget(0) {}

    QPointer<ScriptingModeWidget> widget;
};

ScriptingMode::ScriptingMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new ScriptingModePrivateData();
    setObjectName(tr("Scripting Mode"));
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

}
