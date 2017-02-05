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

#include "StatisticsPluginBasicWidget.h"
#include "StatisticsPluginBasic.h"
using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginBasic {

struct StatisticsPluginBasicData {
    StatisticsPluginBasicData()
    {}

    int command;
};

StatisticsPluginBasic::StatisticsPluginBasic(QObject* parent) :
    QObject(parent), d(new StatisticsPluginBasicData)
{
    setObjectName(pluginName());
}

StatisticsPluginBasic::~StatisticsPluginBasic()
{
    delete d;
}

bool StatisticsPluginBasic::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool StatisticsPluginBasic::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void StatisticsPluginBasic::finalize() {
}

QString StatisticsPluginBasic::pluginName() const {
    return "Basic Corpus Statistics";
}

QtilitiesCategory StatisticsPluginBasic::pluginCategory() const {
    return QtilitiesCategory("Statistical Analysis");
}

Qtilities::Core::VersionInformation StatisticsPluginBasic::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString StatisticsPluginBasic::pluginPublisher() const {
    return "George Christodoulides";
}

QString StatisticsPluginBasic::pluginPublisherWebsite() const {
    return "http://www.praaline.org";
}

QString StatisticsPluginBasic::pluginPublisherContact() const {
    return "info@praaline.org";
}

QString StatisticsPluginBasic::pluginDescription() const {
    return tr("Basic corpus statistics.");
}

QString StatisticsPluginBasic::pluginCopyright() const {
    return QString(tr("Copyright") + " 2016-2017, George Christodoulides");
}

QString StatisticsPluginBasic::pluginLicense() const {
    return tr("GPL v.3");
}

QStringList StatisticsPluginBasic::analyserIDs()
{
    QStringList analyserIDs;
    analyserIDs << "Basic";
    return analyserIDs;
}

QString StatisticsPluginBasic::analyserName(const QString &analyserID)
{
    if (analyserID == "Basic") return "Basic Information";
    return QString();
}

QWidget *StatisticsPluginBasic::analyser(const QString &analyserID, CorpusRepository *repository, QWidget *parent)
{
    Q_UNUSED(analyserID)
    return new StatisticsPluginBasicWidget(repository, parent);
}


} // namespace StatisticsPluginBasic
} // namespace Plugins
} // namespace Praaline

