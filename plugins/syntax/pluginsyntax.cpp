#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "pluginsyntax.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

#include "CoNLLUReader.h"
#include "CorpusImporter.h"
#include "SentencesSplitter.h"
#include "LatexTikzDependencyExporter.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Syntax::PluginSyntaxPrivateData {
    PluginSyntaxPrivateData() :
        operationExportSentenceBreakFile(false), operationImportSentenceBreakFile(false), operationCreateSentenceTier(false)
    {}
    bool operationExportSentenceBreakFile;
    bool operationImportSentenceBreakFile;
    bool operationCreateSentenceTier;
};

Praaline::Plugins::Syntax::PluginSyntax::PluginSyntax(QObject* parent) : QObject(parent)
{
    d = new PluginSyntaxPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Syntax::PluginSyntax::~PluginSyntax()
{
    delete d;
}

bool Praaline::Plugins::Syntax::PluginSyntax::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Syntax::PluginSyntax::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Syntax::PluginSyntax::finalize() {

}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginName() const {
    return "Syntax";
}

QtilitiesCategory Praaline::Plugins::Syntax::PluginSyntax::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Syntax::PluginSyntax::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginDescription() const {
    return tr("Syntax is a part-of-speech tagger, multi-word unit and disfluency detector for spoken language transcriptions.");
}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginCopyright() const {
    return QString(tr("Copyright") + " 2015, George Christodoulides");
}

QString Praaline::Plugins::Syntax::PluginSyntax::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Syntax::PluginSyntax::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("operationExportSentenceBreakFile", "Export Sentence Break File", QVariant::Bool, d->operationExportSentenceBreakFile);
    parameters << PluginParameter("operationImportSentenceBreakFile", "Import Sentence Break File", QVariant::Bool, d->operationImportSentenceBreakFile);
    parameters << PluginParameter("operationCreateSentenceTier", "Create Sentence Tier", QVariant::Bool, d->operationCreateSentenceTier);
    return parameters;
}

void Praaline::Plugins::Syntax::PluginSyntax::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("operationExportSentenceBreakFile")) d->operationExportSentenceBreakFile = parameters.value("operationExportSentenceBreakFile").toBool();
    if (parameters.contains("operationImportSentenceBreakFile")) d->operationImportSentenceBreakFile = parameters.value("operationImportSentenceBreakFile").toBool();
    if (parameters.contains("operationCreateSentenceTier")) d->operationCreateSentenceTier = parameters.value("operationCreateSentenceTier").toBool();
}

void readUDCorpus(const QList<CorpusCommunication *> &communications)
{
    Q_UNUSED(communications)
//    QString filename = "/media/george/Elements/universal-dependencies-1.2/UD_French/fr-ud-train.conllu"; // "D:/CORPORA/universal-dependencies-1.2/UD_French/fr-ud-train.conllu";
//    foreach (CorpusCommunication *com, communications) {
//        if (!com) continue;
//        CorpusAnnotation *annot = new CorpusAnnotation(com->ID());
//        com->addAnnotation(annot);
//        AnnotationTierGroup *group = new AnnotationTierGroup();
//        CoNLLUReader::readCoNLLUtoIntervalTier(filename, group);
//        com->repository()->annotations()->saveTiers(com->ID(), "ud", group);
//    }
}

void readPerceoCorpus(const QList<CorpusCommunication *> &communications)
{
    QString filename = "D:/CORPORA/C-PERCEO/corpus_perceo_oral.txt";
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        CorpusAnnotation *annot = new CorpusAnnotation(com->ID());
        com->addAnnotation(annot);
        SpeakerAnnotationTierGroupMap tiers;
        CorpusImporter::readPerceo(filename, tiers);
       com->repository()->annotations()->saveTiersAllSpeakers(com->ID(), tiers);
    }
}

void Praaline::Plugins::Syntax::PluginSyntax::process(const QList<CorpusCommunication *> &communications)
{
    QString m;

    LatexTikzDependencyExporter latex;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        m = latex.process(com);
        if (!m.isEmpty()) printMessage(m);
    }
    return;


    SentencesSplitter sentenceSplitter;
    if (d->operationImportSentenceBreakFile) {
        printMessage(sentenceSplitter.readBreaksFile("/mnt/hgfs/Dropbox/CORPORA/Phonogenre/phonogenre_sentences.txt"));
        foreach (CorpusCommunication *com, communications) {
            if (!com) continue;
            printMessage(sentenceSplitter.importBreaks(com));
        }
    }
    if (d->operationExportSentenceBreakFile) {
        foreach (CorpusCommunication *com, communications) {
            if (!com) continue;
            printMessage(sentenceSplitter.exportSentences(com));
        }
    }
    if (d->operationCreateSentenceTier) {
        foreach (CorpusCommunication *com, communications) {
            if (!com) continue;
            printMessage(sentenceSplitter.createSentenceTier(com));
        }
    }

}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Syntax;
    Q_EXPORT_PLUGIN2(PluginSyntax, PluginSyntax)
#endif
