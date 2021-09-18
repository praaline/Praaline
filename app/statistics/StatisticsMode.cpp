#include <QObject>
#include "StatisticsMode.h"
#include "StatisticsModeWidget.h"
#include "QtilitiesLogging/Logger.h"
#include "QtilitiesCore/QtilitiesCore"
#include "QtilitiesCoreGui/QtilitiesCoreGui"

#include "basic/StatisticsPluginBasic.h"
#include "interrater/StatisticsPluginInterrater.h"
#include "temporal/StatisticsPluginTemporal.h"
#include "prosody/StatisticsPluginProsody.h"
#include "disfluencies/StatisticsPluginDisfluencies.h"
#include "pca/StatisticsPluginPCA.h"

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

    // Register built-in plugins
    OBJECT_MANAGER->registerObject(new Praaline::Plugins::StatisticsPluginBasic::StatisticsPluginBasic(), QtilitiesCategory("Statistics"));
    OBJECT_MANAGER->registerObject(new Praaline::Plugins::StatisticsPluginInterrater::StatisticsPluginInterrater(), QtilitiesCategory("Statistics"));
    OBJECT_MANAGER->registerObject(new Praaline::Plugins::StatisticsPluginTemporal::StatisticsPluginTemporal(), QtilitiesCategory("Statistics"));
    OBJECT_MANAGER->registerObject(new Praaline::Plugins::StatisticsPluginProsody::StatisticsPluginProsody(), QtilitiesCategory("Statistics"));
    OBJECT_MANAGER->registerObject(new Praaline::Plugins::StatisticsPluginDisfluencies::StatisticsPluginDisfluencies(), QtilitiesCategory("Statistics"));
    OBJECT_MANAGER->registerObject(new Praaline::Plugins::StatisticsPluginPCA::StatisticsPluginPCA(), QtilitiesCategory("Statistics"));

    if (!d->widget) d->widget = new StatisticsModeWidget();
    connect(d->widget.data(), &StatisticsModeWidget::activateMode, this, &StatisticsMode::activateMode);
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
