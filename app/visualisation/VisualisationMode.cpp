#include <QObject>
#include "VisualisationMode.h"
#include "VisualisationModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct VisualisationModePrivateData {
    VisualisationModePrivateData() : widget(0) {}

    QPointer<VisualisationModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

VisualisationMode::VisualisationMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new VisualisationModePrivateData();
    setObjectName(tr("Visualisation Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new VisualisationModeWidget();
    connect(d->widget.data(), &VisualisationModeWidget::activateMode, this, &VisualisationMode::activateMode);
}

VisualisationMode::~VisualisationMode()
{
    delete d;
}

QWidget* VisualisationMode::modeWidget() {
    if (!d->widget)
        d->widget = new VisualisationModeWidget();
    return d->widget;
}

void VisualisationMode::initializeMode() {
    if (!d->widget)
        d->widget = new VisualisationModeWidget();
}

QIcon VisualisationMode::modeIcon() const {
    return QIcon(":/icons/modes/visualisation_mode_48x48.png");
}

QString VisualisationMode::modeName() const {
    return tr("Visualisation");
}

void VisualisationMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_VISUALISATION_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void VisualisationMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
