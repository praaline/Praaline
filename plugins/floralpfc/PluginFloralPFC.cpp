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
    PluginFloralPFCPrivateData() :
        pfc_preprocessor_prepare(false), pfc_phonetiser_phonetise(false), pfc_aligner_htk(false),
        pfc_reports_corpuscoverage(false)
    {}

    // steps
    bool pfc_preprocessor_prepare;
    bool pfc_phonetiser_phonetise;
    bool pfc_aligner_htk;
    bool pfc_reports_corpuscoverage;

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
    parameters << PluginParameter("pfc_preprocessor_prepare",   "PFC Pre-processor: Prepare transcription", QVariant::Bool, d->pfc_preprocessor_prepare);
    parameters << PluginParameter("pfc_phonetiser_phonetise",   "PFC Phonetiser: Phonetise tok_min tier", QVariant::Bool, d->pfc_phonetiser_phonetise);
    parameters << PluginParameter("pfc_aligner_htk",            "PFC Aligner: HTK basic alignment", QVariant::Bool, d->pfc_aligner_htk);
    parameters << PluginParameter("pfc_reports_corpuscoverage", "PFC Reports: Corpus coverage", QVariant::Bool, d->pfc_reports_corpuscoverage);

    parameters << PluginParameter("path", "Path to files", QVariant::String, d->path);
    parameters << PluginParameter("filename", "Filename", QVariant::String, d->filename);
    return parameters;
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("pfc_preprocessor_prepare")) d->pfc_preprocessor_prepare = parameters.value("pfc_preprocessor_prepare").toBool();
    if (parameters.contains("pfc_phonetiser_phonetise")) d->pfc_phonetiser_phonetise = parameters.value("pfc_phonetiser_phonetise").toBool();
    if (parameters.contains("pfc_aligner_htk")) d->pfc_aligner_htk = parameters.value("pfc_aligner_htk").toBool();
    if (parameters.contains("pfc_reports_corpuscoverage")) d->pfc_reports_corpuscoverage = parameters.value("pfc_reports_corpuscoverage").toBool();

    if (parameters.contains("path")) d->path = parameters.value("path").toString();
    if (parameters.contains("filename")) d->filename = parameters.value("filename").toString();
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::process(const QList<QPointer<CorpusCommunication> > &communications)
{
    PFCPreprocessor preprocessor;
    PFCPhonetiser phonetiser;
    PFCAligner aligner;
    PFCReports reports;

    if (d->pfc_preprocessor_prepare) {
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = preprocessor.prepareTranscription(com);
            m = m.append("\t").append(preprocessor.checkSpeakers(com));
            if (!m.isEmpty()) printMessage(m);
        }
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = preprocessor.separateSpeakers(com);
            m = m.append("\t").append(preprocessor.tokenise(com));
            m = m.append("\t").append(preprocessor.tokmin_punctuation(com));
            if (!m.isEmpty()) printMessage(m);
        }
    }
    if (d->pfc_phonetiser_phonetise) {
        phonetiser.loadPhonetisationDictionary();
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = phonetiser.phonetiseFromDictionary(com);
            if (!m.isEmpty()) printMessage(m);
        }
        printMessage(phonetiser.writeListOfWordsOOV("/mnt/hgfs/DATA/PFCALIGN/phonetisation/oov.txt"));
        printMessage(phonetiser.writeListOfWordsFalseStarts("/mnt/hgfs/DATA/PFCALIGN/phonetisation/falsestarts.txt"));
    }
    if (d->pfc_aligner_htk) {
        foreach (QPointer<CorpusCommunication> com, communications) {
            if (com->ID().endsWith("m")) continue;
            QString m = aligner.align(com);
            if (!m.isEmpty()) printMessage(m);
        }
    }
    if (d->pfc_reports_corpuscoverage) {
        QString m;
        if (!communications.isEmpty()) m = reports.corpusCoverageStatistics(communications.first()->corpus());
        if (!m.isEmpty()) printMessage(m);
    }

//    PFCPreprocessor p;
//    QString m = p.renameTextgridTiers("/mnt/hgfs/DATA/PFCALIGN/Textgrids");
//    printMessage(m);
//    return;


//    QString m = report.corpusCoverageStatistics(communications.first()->corpus());
//    if (!m.isEmpty()) printMessage(m);
//        foreach (QPointer<CorpusCommunication> com, communications) {
//            QString m = report.reportCorrections(com);
//            if (!m.isEmpty()) printMessage(m);
//        }
//    }



//    if (d->corpusType == "valibel") {
//        if (d->command.contains("import")) ValibelProcessor::importValibelFile(communications.first()->corpus(), d->path + "/" + d->filename);
//        if (d->command.contains("tokenise")) ValibelProcessor::tokenise(communications);
//        if (d->command.contains("pauses")) ValibelProcessor::pauses(communications);
//    }
//    else if (d->corpusType == "pfc") {
//        if (d->command.contains("prepareTranscriptions")) prepareTranscriptions(communications);
//        if (d->command.contains("checkSpeakers")) checkSpeakers(communications);
//        if (d->command.contains("separateSpeakers")) separateSpeakers(communications);
//        if (d->command.contains("tokenise")) tokenise(communications);
//        if (d->command.contains("tokmin_punctuation")) tokmin_punctuation(communications);
//        if (d->command.contains("liaisonCoding")) liaisonCoding(communications);

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::FloralPFC;
    Q_EXPORT_PLUGIN2(PluginFloralPFC, PluginFloralPFC)
#endif
