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

#include "AnalyserTemporal.h"

#include "StatisticsPluginTemporal.h"
#include "AnalyserTemporalWidgetWidget.h"
#include "PauseLengthDistributionWIdget.h"
using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginTemporal {

struct StatisticsPluginTemporalData {
    StatisticsPluginTemporalData() :
        analyser(0)
    {}

    AnalyserTemporal *analyser;
};

StatisticsPluginTemporal::StatisticsPluginTemporal(QObject* parent) :
    QObject(parent), d(new StatisticsPluginTemporalData)
{
    setObjectName(pluginName());
    d->analyser = new AnalyserTemporal(this);
}

StatisticsPluginTemporal::~StatisticsPluginTemporal()
{
    delete d;
}

bool StatisticsPluginTemporal::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool StatisticsPluginTemporal::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void StatisticsPluginTemporal::finalize() {
}

QString StatisticsPluginTemporal::pluginName() const {
    return "Temporal Measures";
}

QtilitiesCategory StatisticsPluginTemporal::pluginCategory() const {
    return QtilitiesCategory("Statistical Analysis");
}

Qtilities::Core::VersionInformation StatisticsPluginTemporal::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString StatisticsPluginTemporal::pluginPublisher() const {
    return "George Christodoulides";
}

QString StatisticsPluginTemporal::pluginPublisherWebsite() const {
    return "http://www.praaline.org";
}

QString StatisticsPluginTemporal::pluginPublisherContact() const {
    return "info@praaline.org";
}

QString StatisticsPluginTemporal::pluginDescription() const {
    return tr("Statistical analysis of temporal speech measures.");
}

QString StatisticsPluginTemporal::pluginCopyright() const {
    return QString(tr("Copyright") + " 2016-2017, George Christodoulides");
}

QString StatisticsPluginTemporal::pluginLicense() const {
    return tr("GPL v.3");
}

QStringList StatisticsPluginTemporal::analyserIDs()
{
    QStringList analyserIDs;
    analyserIDs << "MeasuresTable" << "PauseLengthDistribution";
    return analyserIDs;
}

QString StatisticsPluginTemporal::analyserName(const QString &analyserID)
{
    if (analyserID == "MeasuresTable") return "Temporal Measures Tables";
    if (analyserID == "PauseLengthDistribution") return "Pause Length Distribution";
    return QString();
}

QWidget *StatisticsPluginTemporal::analyser(const QString &analyserID, CorpusRepository *repository, QWidget *parent)
{
    if (analyserID == "MeasuresTable") return new AnalyserTemporalWidget(repository, d->analyser, parent);
    if (analyserID == "PauseLengthDistribution") return new PauseLengthDistributionWidget(repository, d->analyser, parent);
    return new QWidget(parent);
}

} // namespace StatisticsPluginTemporal
} // namespace Plugins
} // namespace Praaline

