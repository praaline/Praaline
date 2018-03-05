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
using namespace Praaline::Core;

#include "StatisticsPluginPCA.h"
using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

#include "PCAPlotWidget.h"

namespace Praaline {
namespace Plugins {
namespace StatisticsPluginPCA {

struct StatisticsPluginPCAData {
    StatisticsPluginPCAData()
    {}

    int command;
};

StatisticsPluginPCA::StatisticsPluginPCA(QObject* parent) :
    QObject(parent), d(new StatisticsPluginPCAData)
{
    setObjectName(pluginName());
}

StatisticsPluginPCA::~StatisticsPluginPCA()
{
    delete d;
}

bool StatisticsPluginPCA::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool StatisticsPluginPCA::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void StatisticsPluginPCA::finalize() {
}

QString StatisticsPluginPCA::pluginName() const {
    return "Principal Component Analysis";
}

QtilitiesCategory StatisticsPluginPCA::pluginCategory() const {
    return QtilitiesCategory("Statistical Analysis");
}

Qtilities::Core::VersionInformation StatisticsPluginPCA::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString StatisticsPluginPCA::pluginPublisher() const {
    return "George Christodoulides";
}

QString StatisticsPluginPCA::pluginPublisherWebsite() const {
    return "http://www.praaline.org";
}

QString StatisticsPluginPCA::pluginPublisherContact() const {
    return "info@praaline.org";
}

QString StatisticsPluginPCA::pluginDescription() const {
    return tr("Principal Component Analysis of Communications based on metadata values.");
}

QString StatisticsPluginPCA::pluginCopyright() const {
    return QString(tr("Copyright") + " 2018, George Christodoulides");
}

QString StatisticsPluginPCA::pluginLicense() const {
    return tr("GPL v.3");
}

QStringList StatisticsPluginPCA::analyserIDs()
{
    QStringList analyserIDs;
    analyserIDs << "PCA";
    return analyserIDs;
}

QString StatisticsPluginPCA::analyserName(const QString &analyserID)
{
    if (analyserID == "PCA") return "PCA Plot";
    return QString();
}

QWidget *StatisticsPluginPCA::analyser(const QString &analyserID, CorpusRepository *repository, QWidget *parent)
{
    Q_UNUSED(analyserID)
    return new PCAPlotWidget(repository, parent);
}


} // namespace StatisticsPluginPCA
} // namespace Plugins
} // namespace Praaline

