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
#include "PFCAlignmentEvaluation.h"
#include "PFCReports.h"

#include "valibelprocessor.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::FloralPFC::PluginFloralPFCPrivateData {
    PluginFloralPFCPrivateData() :
        pfc_preprocessor_check_transcription_speakers(false), pfc_preprocessor_diarise_tokenise(false),
        pfc_phonetiser_phonetise(false),
        pfc_aligner_htk(false), pfc_aligner_mfa_individual(false), pfc_aligner_mfa_regionstyle(false),
        pfc_aligner_cross_text(false),
        pfc_evaluate(false),
        pfc_reports_corpuscoverage(false)
    {}

    // steps
    bool pfc_preprocessor_check_transcription_speakers;
    bool pfc_preprocessor_diarise_tokenise;
    bool pfc_phonetiser_phonetise;
    bool pfc_aligner_htk;
    bool pfc_aligner_mfa_individual;
    bool pfc_aligner_mfa_regionstyle;
    bool pfc_aligner_cross_text;
    bool pfc_evaluate;
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
    parameters << PluginParameter("pfc_preprocessor_check_transcription_speakers",
                                  "Pre-processor Step 1: Check transcription and num speakers", QVariant::Bool, d->pfc_preprocessor_check_transcription_speakers);
    parameters << PluginParameter("pfc_preprocessor_diarise_tokenise",
                                  "Pre-processor Step 2: Diarise and Tokenise", QVariant::Bool, d->pfc_preprocessor_diarise_tokenise);
    parameters << PluginParameter("pfc_phonetiser_phonetise",
                                  "Phonetiser: Phonetise tok_min tier", QVariant::Bool, d->pfc_phonetiser_phonetise);
    parameters << PluginParameter("pfc_aligner_htk",
                                  "Aligner: HTK basic alignment", QVariant::Bool, d->pfc_aligner_htk);
    parameters << PluginParameter("pfc_aligner_mfa_individual",
                                  "Aligner: MFA create alignment files - Individual", QVariant::Bool, d->pfc_aligner_mfa_individual);
    parameters << PluginParameter("pfc_aligner_mfa_regionstyle",
                                  "Aligner: MFA create alignment files - Region & Style", QVariant::Bool, d->pfc_aligner_mfa_regionstyle);
    parameters << PluginParameter("pfc_aligner_cross_text",
                                  "Aligner: Cross align text", QVariant::Bool, d->pfc_aligner_cross_text);
    parameters << PluginParameter("pfc_evaluate",
                                  "Evaluate: Alignment MFA", QVariant::Bool, d->pfc_evaluate);
    parameters << PluginParameter("pfc_reports_corpuscoverage",
                                  "Reports: Corpus coverage", QVariant::Bool, d->pfc_reports_corpuscoverage);

    parameters << PluginParameter("path", "Path to files", QVariant::String, d->path);
    parameters << PluginParameter("filename", "Filename", QVariant::String, d->filename);
    return parameters;
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("pfc_preprocessor_check_transcription_speakers")) d->pfc_preprocessor_check_transcription_speakers = parameters.value("pfc_preprocessor_check_transcription_speakers").toBool();
    if (parameters.contains("pfc_preprocessor_diarise_tokenise")) d->pfc_preprocessor_diarise_tokenise = parameters.value("pfc_preprocessor_diarise_tokenise").toBool();
    if (parameters.contains("pfc_phonetiser_phonetise")) d->pfc_phonetiser_phonetise = parameters.value("pfc_phonetiser_phonetise").toBool();
    if (parameters.contains("pfc_aligner_htk")) d->pfc_aligner_htk = parameters.value("pfc_aligner_htk").toBool();
    if (parameters.contains("pfc_aligner_mfa_individual")) d->pfc_aligner_mfa_individual = parameters.value("pfc_aligner_mfa_individual").toBool();
    if (parameters.contains("pfc_aligner_mfa_regionstyle")) d->pfc_aligner_mfa_regionstyle = parameters.value("pfc_aligner_mfa_regionstyle").toBool();
    if (parameters.contains("pfc_aligner_cross_text")) d->pfc_aligner_cross_text = parameters.value("pfc_aligner_cross_text").toBool();
    if (parameters.contains("pfc_evaluate")) d->pfc_evaluate = parameters.value("pfc_evaluate").toBool();
    if (parameters.contains("pfc_reports_corpuscoverage")) d->pfc_reports_corpuscoverage = parameters.value("pfc_reports_corpuscoverage").toBool();

    if (parameters.contains("path")) d->path = parameters.value("path").toString();
    if (parameters.contains("filename")) d->filename = parameters.value("filename").toString();
}

bool writeStringListToFile(const QString &filename, const QStringList &strings)
{
    QFile fileOut(filename);
    if ( !fileOut.open( QIODevice::WriteOnly | QIODevice::Text ) ) return false;
    QTextStream out(&fileOut);
    out.setCodec("UTF-8");
    foreach (QString string, strings) {
        out << string;
        if (!string.endsWith("\n")) out << "\n";
    }
    fileOut.close();
    return true;
}

void Praaline::Plugins::FloralPFC::PluginFloralPFC::process(const QList<QPointer<CorpusCommunication> > &communications)
{
    PFCPreprocessor preprocessor;
    PFCPhonetiser phonetiser;
    PFCAligner aligner;
    PFCAlignmentEvaluation evaluation;
    PFCReports reports;

    if (d->pfc_preprocessor_check_transcription_speakers) {
        foreach (QPointer<CorpusCommunication> com, communications) {
            QString m = preprocessor.prepareTranscription(com);
            m = m.append("\t").append(preprocessor.checkSpeakers(com));
            if (!m.isEmpty()) printMessage(m);
        }
    }
    if (d->pfc_preprocessor_diarise_tokenise) {
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
            QString m = aligner.align(com, "htk");
            if (!m.isEmpty()) printMessage(m);
        }
    }
    if (d->pfc_aligner_mfa_individual) {
        aligner.setOutputWaveFiles(false);
        foreach (QPointer<CorpusCommunication> com, communications) {
            if (com->ID().endsWith("m")) continue;
            QString m = aligner.align(com, "mfa_individual");
            aligner.dictionaryMFAClose(com->ID());
            if (!m.isEmpty()) printMessage(m);
        }
    }
    if (d->pfc_aligner_mfa_regionstyle) {
        aligner.setOutputWaveFiles(true);
        QMap<QString, QList<QPointer<CorpusCommunication> > > groups;
        foreach (QPointer<CorpusCommunication> com, communications) {
            if (com->ID().endsWith("m")) continue;
            QString region = com->ID().left(3);
            QString style = (com->ID().endsWith("t")) ? "text" : "conv";
            groups[region + "_" + style].append(com);
        }
        foreach (QString groupID, groups.keys()) {
            foreach (QPointer<CorpusCommunication> com, groups.value(groupID)) {
                aligner.align(com, "mfa_regionstyle");
            }
            aligner.dictionaryMFAClose(groupID);
            QString command = QString("./mfa_train_and_align /mnt/hgfs/DATA/PFCALIGN/MFA_region_style/%1 "
                                      "/mnt/hgfs/DATA/PFCALIGN/MFA_region_style/%1/%1.dic "
                                      "/mnt/hgfs/DATA/PFCALIGN/MFA_region_style/%1/align_%1 "
                                      "-o /mnt/hgfs/DATA/PFCALIGN/MFA_region_style/%1/model_%1 "
                                      "-t /mnt/hgfs/DATA/PFCALIGN/MFA_temp_region_style -f -q -c").arg(groupID);
            printMessage(command);
        }
    }
    if (d->pfc_aligner_cross_text) {
        QString cross = aligner.scriptCrossAlignment();
        // QString cross = aligner.combineDictionaries();
        printMessage(cross);
    }
    if (d->pfc_evaluate) {
        evaluation.pivotReset();
        foreach (QPointer<CorpusCommunication> com, communications) {
            if (com->ID().endsWith("m")) continue;
            // QString m = evaluation.evaluate_Individual_RegionStyle(com);
            QString m = evaluation.evaluate_RegionStyle_RegionStyle(com);
            if (!m.isEmpty()) printMessage(m);
        }
        writeStringListToFile("/mnt/hgfs/DATA/PFCALIGN/MFA_evaluate/crosseval_list_text.txt", evaluation.pivotList("text"));
        writeStringListToFile("/mnt/hgfs/DATA/PFCALIGN/MFA_evaluate/crosseval_list_conv.txt", evaluation.pivotList("conv"));
        writeStringListToFile("/mnt/hgfs/DATA/PFCALIGN/MFA_evaluate/crosseval_table_20ms_text.txt", evaluation.pivotTable("text", 20));
        writeStringListToFile("/mnt/hgfs/DATA/PFCALIGN/MFA_evaluate/crosseval_table_20ms_conv.txt", evaluation.pivotTable("conv", 20));
        writeStringListToFile("/mnt/hgfs/DATA/PFCALIGN/MFA_evaluate/crosseval_table_40ms_text.txt", evaluation.pivotTable("text", 40));
        writeStringListToFile("/mnt/hgfs/DATA/PFCALIGN/MFA_evaluate/crosseval_table_40ms_conv.txt", evaluation.pivotTable("conv", 40));
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
