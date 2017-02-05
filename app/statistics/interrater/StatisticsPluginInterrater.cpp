#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <QWidget>
#include <ExtensionSystemConstants>

#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"

#include "KappaStatisticsWidget.h"
#include "StatisticsPluginInterrater.h"
using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginInterrater {

struct StatisticsPluginInterraterData {
    StatisticsPluginInterraterData()
    {}

    int command;
};

StatisticsPluginInterrater::StatisticsPluginInterrater(QObject* parent) :
    QObject(parent), d(new StatisticsPluginInterraterData)
{
    setObjectName(pluginName());
}

StatisticsPluginInterrater::~StatisticsPluginInterrater()
{
    delete d;
}

bool StatisticsPluginInterrater::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool StatisticsPluginInterrater::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void StatisticsPluginInterrater::finalize() {
}

QString StatisticsPluginInterrater::pluginName() const {
    return "Interrater Agreement";
}

QtilitiesCategory StatisticsPluginInterrater::pluginCategory() const {
    return QtilitiesCategory("Statistical Analysis");
}

Qtilities::Core::VersionInformation StatisticsPluginInterrater::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString StatisticsPluginInterrater::pluginPublisher() const {
    return "George Christodoulides";
}

QString StatisticsPluginInterrater::pluginPublisherWebsite() const {
    return "http://www.praaline.org";
}

QString StatisticsPluginInterrater::pluginPublisherContact() const {
    return "info@praaline.org";
}

QString StatisticsPluginInterrater::pluginDescription() const {
    return tr("Interrater agreement analysis.");
}

QString StatisticsPluginInterrater::pluginCopyright() const {
    return QString(tr("Copyright") + " 2016-2017, George Christodoulides");
}

QString StatisticsPluginInterrater::pluginLicense() const {
    return tr("GPL v.3");
}

QStringList StatisticsPluginInterrater::analyserIDs()
{
    QStringList analyserIDs;
    analyserIDs << "Interrater";
    return analyserIDs;
}

QString StatisticsPluginInterrater::analyserName(const QString &analyserID)
{
    if (analyserID == "Interrater") return "Calculate inter-rater agreement";
    return QString();
}

QWidget *StatisticsPluginInterrater::analyser(const QString &analyserID, CorpusRepository *repository, QWidget *parent)
{
    Q_UNUSED(analyserID)
    return new KappaStatisticsWidget(repository, parent);
}


} // namespace StatisticsPluginInterrater
} // namespace Plugins
} // namespace Praaline

