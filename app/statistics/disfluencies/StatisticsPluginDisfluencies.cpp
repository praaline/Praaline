#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <QWidget>
#include <ExtensionSystemConstants>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"

#include "AnalyserDisfluencies.h"

#include "StatisticsPluginDisfluencies.h"
#include "AnalyserDisfluenciesWidget.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginDisfluencies {

struct StatisticsPluginDisfluenciesData {
    StatisticsPluginDisfluenciesData() :
        analyser(0)
    {}

    AnalyserDisfluencies *analyser;
};

StatisticsPluginDisfluencies::StatisticsPluginDisfluencies(QObject* parent) :
    QObject(parent), d(new StatisticsPluginDisfluenciesData)
{
    setObjectName(pluginName());
    d->analyser = new AnalyserDisfluencies(this);
}

StatisticsPluginDisfluencies::~StatisticsPluginDisfluencies()
{
    delete d;
}

bool StatisticsPluginDisfluencies::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool StatisticsPluginDisfluencies::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void StatisticsPluginDisfluencies::finalize() {
}

QString StatisticsPluginDisfluencies::pluginName() const {
    return "Fluency Profile";
}

QtilitiesCategory StatisticsPluginDisfluencies::pluginCategory() const {
    return QtilitiesCategory("Statistical Analysis");
}

Qtilities::Core::VersionInformation StatisticsPluginDisfluencies::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString StatisticsPluginDisfluencies::pluginPublisher() const {
    return "George Christodoulides";
}

QString StatisticsPluginDisfluencies::pluginPublisherWebsite() const {
    return "http://www.praaline.org";
}

QString StatisticsPluginDisfluencies::pluginPublisherContact() const {
    return "info@praaline.org";
}

QString StatisticsPluginDisfluencies::pluginDescription() const {
    return tr("Statistical analysis of Disfluencies speech measures.");
}

QString StatisticsPluginDisfluencies::pluginCopyright() const {
    return QString(tr("Copyright") + " 2016-2017, George Christodoulides");
}

QString StatisticsPluginDisfluencies::pluginLicense() const {
    return tr("GPL v.3");
}

QStringList StatisticsPluginDisfluencies::analyserIDs()
{
    QStringList analyserIDs;
    analyserIDs << "MeasuresTable";
    return analyserIDs;
}

QString StatisticsPluginDisfluencies::analyserName(const QString &analyserID)
{
    if (analyserID == "MeasuresTable") return "Disfluency Measures";
    return QString();
}

QWidget *StatisticsPluginDisfluencies::analyser(const QString &analyserID, CorpusRepository *repository, QWidget *parent)
{
    if (analyserID == "MeasuresTable") return new AnalyserDisfluenciesWidget(repository, d->analyser, parent);
    return new QWidget(parent);
}

} // namespace StatisticsPluginDisfluencies
} // namespace Plugins
} // namespace Praaline

