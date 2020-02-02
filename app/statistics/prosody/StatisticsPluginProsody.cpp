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

#include "GlobalProsodicProfileWidget.h"
#include "AnalyserMacroprosodyWidget.h"
#include "StatisticsPluginProsody.h"
using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginProsody {

struct StatisticsPluginProsodyData {
    StatisticsPluginProsodyData()
    {}

    int command;
};

StatisticsPluginProsody::StatisticsPluginProsody(QObject* parent) :
    QObject(parent), d(new StatisticsPluginProsodyData)
{
    setObjectName(pluginName());
}

StatisticsPluginProsody::~StatisticsPluginProsody()
{
    delete d;
}

bool StatisticsPluginProsody::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool StatisticsPluginProsody::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void StatisticsPluginProsody::finalize() {
}

QString StatisticsPluginProsody::pluginName() const {
    return "Prosodic Measures";
}

QtilitiesCategory StatisticsPluginProsody::pluginCategory() const {
    return QtilitiesCategory("Statistical Analysis");
}

Qtilities::Core::VersionInformation StatisticsPluginProsody::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString StatisticsPluginProsody::pluginPublisher() const {
    return "George Christodoulides";
}

QString StatisticsPluginProsody::pluginPublisherWebsite() const {
    return "http://www.praaline.org";
}

QString StatisticsPluginProsody::pluginPublisherContact() const {
    return "info@praaline.org";
}

QString StatisticsPluginProsody::pluginDescription() const {
    return tr("Statistical analysis of prosodic measures.");
}

QString StatisticsPluginProsody::pluginCopyright() const {
    return QString(tr("Copyright") + " 2016-2017, George Christodoulides");
}

QString StatisticsPluginProsody::pluginLicense() const {
    return tr("GPL v.3");
}

QStringList StatisticsPluginProsody::analyserIDs()
{
    QStringList analyserIDs;
    analyserIDs << "GlobalProfile" << "MacroUnits";
    return analyserIDs;
}

QString StatisticsPluginProsody::analyserName(const QString &analyserID)
{
    if (analyserID == "GlobalProfile")  return "Global Prosodic Profile";
    if (analyserID == "MacroUnits")     return "Macro-Prosodic Measures";
    return QString();
}

QWidget *StatisticsPluginProsody::analyser(const QString &analyserID, CorpusRepository *repository, QWidget *parent)
{
    if (analyserID == "GlobalProfile")  return new GlobalProsodicProfileWidget(repository, parent);
    if (analyserID == "MacroUnits")     return new AnalyserMacroprosodyWidget(repository, parent);
    return new QWidget(parent);
}


} // namespace StatisticsPluginProsody
} // namespace Plugins
} // namespace Praaline

