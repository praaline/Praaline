#include <QObject>
#include "StatisticsMode.h"
#include "StatisticsModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

using namespace QtilitiesCore;
using namespace QtilitiesCoreGui;

namespace Praaline {

struct StatisticsModePrivateData {
    StatisticsModePrivateData() : widget(0) {}

    QPointer<StatisticsModeWidget> widget;
    QPointer<QtilitiesMainWindow> mainWindow;
};

StatisticsMode::StatisticsMode(QObject *parent) :
    QObject(parent), IMode()
{
    d = new StatisticsModePrivateData();
    setObjectName(tr("Statistics Mode"));

    // Find reference to main window
    QList<QObject *> list;
    list = OBJECT_MANAGER->registeredInterfaces("Qtilities::CoreGui::QtilitiesMainWindow");
    foreach (QObject* obj, list) {
        QtilitiesMainWindow *window = qobject_cast<QtilitiesMainWindow *>(obj);
        if (window) d->mainWindow = window;
    }

    if (!d->widget) d->widget = new StatisticsModeWidget();
    connect(d->widget, SIGNAL(activateMode()), this, SLOT(activateMode()));
}

StatisticsMode::~StatisticsMode()
{
    delete d;
}

QWidget* StatisticsMode::modeWidget() {
    if (!d->widget)
        d->widget = new StatisticsModeWidget();
    return d->widget;
}

void StatisticsMode::initializeMode() {
    if (!d->widget)
        d->widget = new StatisticsModeWidget();
}

QIcon StatisticsMode::modeIcon() const {
    return QIcon(":/icons/modes/statistics_mode_48x48.png");
}

QString StatisticsMode::modeName() const {
    return tr("Statistics");
}

void StatisticsMode::modeManagerActiveModeChanged(int new_mode_id, int old_mode_id)
{
    Q_UNUSED(old_mode_id)
    if (new_mode_id == MODE_STATISTICS_ID)
        emit modeActivated();
    else
        emit modeDeactivated();
}

void StatisticsMode::activateMode()
{
    if (!d->mainWindow) return;
    d->mainWindow->modeManager()->setActiveMode(modeName());
}

}
