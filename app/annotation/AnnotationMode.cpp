#include <QObject>
#include "AnnotationMode.h"
#include "AnnotationModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct AnnotationModePrivateData {
    AnnotationModePrivateData() : widget(0) {}

    QPointer<AnnotationModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

AnnotationMode::AnnotationMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new AnnotationModePrivateData();
    setObjectName(tr("Annotation Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new AnnotationModeWidget();
    connect(d->widget, SIGNAL(activateMode()), this, SLOT(activateMode()));
}

AnnotationMode::~AnnotationMode()
{
    delete d;
}

QWidget* AnnotationMode::modeWidget() {
    if (!d->widget)
        d->widget = new AnnotationModeWidget();
    return d->widget;
}

void AnnotationMode::initializeMode() {
    if (!d->widget)
        d->widget = new AnnotationModeWidget();
}

QIcon AnnotationMode::modeIcon() const {
    return QIcon(":/icons/modes/annotation_mode_48x48.png");
}

QString AnnotationMode::modeName() const {
    return tr("Annotation");
}

void AnnotationMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_ANNOTATION_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void AnnotationMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
