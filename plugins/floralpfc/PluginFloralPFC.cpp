#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>


#include "PluginFloralPFC.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"

#include "PFCPreprocessor.h"
#include "PFCPhonetiser.h"
#include "PFCAligner.h"
#include "PFCReports.h"

#include "valibelprocessor.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::FloralPFC::PluginFloralPFCPrivateData {
    PluginFloralPFCPrivateData() : command("")
    {}
    QString corpusType;
    QString command;
    QString path;
    QString filename;
};

Praaline::Plugins::FloralPFC::PluginFloralPFC::PluginFloralPFC(QObject* parent) : QObject(parent)
{
    d = new PluginFloralPFCPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::FloralPFC::PluginFloralPFC::~PluginFloralPFC()
{
    delete d;
}

bool Praaline::Plugins::FloralPFC::PluginFloralPFC::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::FloralPFC::PluginFloralPFC::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::finalize() {

}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginName() const {
    return "FloralPFC";
}

QtilitiesCategory Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginDescription() const {
    return tr("The FloralPFC plugin contains scripts to process the PFC corpus.");
}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginCopyright() const {
    return QString(tr("Copyright") + " 2014-2015, George Christodoulides");
}

QString Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginLicense() const {
    return tr("GPL v.3");
}










QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::FloralPFC::PluginFloralPFC::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("corpusType", "Corpus type", QVariant::String, d->corpusType);
    parameters << PluginParameter("command", "Command", QVariant::String, d->command);
    parameters << PluginParameter("path", "Path to files", QVariant::String, d->path);
    parameters << PluginParameter("filename", "Filename", QVariant::String, d->filename);
    return parameters;
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("command")) d->command = parameters.value("command").toString();
    if (parameters.contains("corpusType")) d->corpusType = parameters.value("corpusType").toString();
    if (parameters.contains("path")) d->path = parameters.value("path").toString();
    if (parameters.contains("filename")) d->filename = parameters.value("filename").toString();
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::process(const QList<QPointer<CorpusCommunication> > &communications)
{
//    PFCPreprocessor p;
//    QString m = p.renameTextgridTiers("/mnt/hgfs/DATA/PFCALIGN/Textgrids");
//    printMessage(m);
//    return;

//    PFCPreprocessor p;
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        QString m = p.prepareTranscription(com);
//        m = m.append("\t").append(p.checkSpeakers(com));
//        if (!m.isEmpty()) printMessage(m);
//    }
//    return;

    if (communications.isEmpty()) return;

    if (false) {
        PFCPreprocessor p;
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = p.separateSpeakers(com);
            m = m.append("\t").append(p.tokenise(com));
            m = m.append("\t").append(p.tokmin_punctuation(com));
            if (!m.isEmpty()) printMessage(m);
        }

        PFCPhonetiser phon;
        phon.loadPhonetisationDictionary();
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = phon.phonetiseFromDictionary(com);
            if (!m.isEmpty()) printMessage(m);
        }
        printMessage(phon.writeListOfWordsOOV("/mnt/hgfs/DATA/PFCALIGN/phonetisation/oov.txt"));
        printMessage(phon.writeListOfWordsFalseStarts("/mnt/hgfs/DATA/PFCALIGN/phonetisation/falsestarts.txt"));

        PFCAligner aligner;
        foreach (QPointer<CorpusCommunication> com, communications) {
            if (!com->ID().endsWith("t")) continue;
            QString m = aligner.align(com);
            if (!m.isEmpty()) printMessage(m);
        }

        PFCReports report;
    //    QString m = report.corpusCoverageStatistics(communications.first()->corpus());
    //    if (!m.isEmpty()) printMessage(m);
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = report.reportCorrections(com);
            if (!m.isEmpty()) printMessage(m);
        }
    }

    PFCPhonetiser phon;
    phon.loadPhonetisationDictionary();
    foreach (QPointer<CorpusCommunication> com, communications) {
        QString m = phon.phonetiseFromDictionary(com);
        if (!m.isEmpty()) printMessage(m);
    }
    printMessage(phon.writeListOfWordsOOV("/mnt/hgfs/Dropbox/CORPORA/NCCFR/oov.txt"));
    printMessage(phon.writeListOfWordsFalseStarts("/mnt/hgfs/Dropbox/CORPORA/NCCFR/falsestarts.txt"));


    if (d->corpusType == "valibel") {
//        if (d->command.contains("import")) ValibelProcessor::importValibelFile(communications.first()->corpus(), d->path + "/" + d->filename);
//        if (d->command.contains("tokenise")) ValibelProcessor::tokenise(communications);
//        if (d->command.contains("pauses")) ValibelProcessor::pauses(communications);
    }
    else if (d->corpusType == "pfc") {
//        if (d->command.contains("prepareTranscriptions")) prepareTranscriptions(communications);
//        if (d->command.contains("checkSpeakers")) checkSpeakers(communications);
//        if (d->command.contains("separateSpeakers")) separateSpeakers(communications);
//        if (d->command.contains("tokenise")) tokenise(communications);
//        if (d->command.contains("tokmin_punctuation")) tokmin_punctuation(communications);
//        if (d->command.contains("liaisonCoding")) liaisonCoding(communications);
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::FloralPFC;
    Q_EXPORT_PLUGIN2(PluginFloralPFC, PluginFloralPFC)
#endif
