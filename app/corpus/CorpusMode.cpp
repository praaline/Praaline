#include <QObject>
#include "CorpusMode.h"
#include "CorpusModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct CorpusModePrivateData {
    CorpusModePrivateData() : widget(0), mainWindow(0) {}

    QPointer<CorpusModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

CorpusMode::CorpusMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new CorpusModePrivateData();
    setObjectName(tr("Corpus Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new CorpusModeWidget();
    connect(d->widget.data(), &CorpusModeWidget::activateMode, this, &CorpusMode::activateMode);
}

CorpusMode::~CorpusMode()
{
    delete d;
}

QWidget* CorpusMode::modeWidget() {
    if (!d->widget)
        d->widget = new CorpusModeWidget();
    return d->widget;
}

void CorpusMode::initializeMode() {
    if (!d->widget)
        d->widget = new CorpusModeWidget();
}

QIcon CorpusMode::modeIcon() const {
    return QIcon(":/icons/modes/corpus_mode_48x48.png");
}

QString CorpusMode::modeName() const {
    return tr("Corpus");
}

void CorpusMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_CORPUS_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void CorpusMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
