#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "pluginlongalign.h"
#include "pncore/corpus/corpus.h"
#include "pncore/annotation/intervaltier.h"
#include "languagemodel.h"
#include "recognitionresult.h"
#include "pnlib/diff/diffintervals.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::LongAlign::PluginLongAlignPrivateData {
    PluginLongAlignPrivateData() : command("")
    {}
    QString command;
};

Praaline::Plugins::LongAlign::PluginLongAlign::PluginLongAlign(QObject* parent) : QObject(parent)
{
    d = new PluginLongAlignPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::LongAlign::PluginLongAlign::~PluginLongAlign()
{
    delete d;
}

bool Praaline::Plugins::LongAlign::PluginLongAlign::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::LongAlign::PluginLongAlign::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::LongAlign::PluginLongAlign::finalize() {

}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginName() const {
    return "LongAlign";
}

QtilitiesCategory Praaline::Plugins::LongAlign::PluginLongAlign::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::LongAlign::PluginLongAlign::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginDescription() const {
    return tr("The LongAlign plugin contains scripts to process the PFC corpus.");
}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginCopyright() const {
    return QString(tr("Copyright") + " 2014-2015, George Christodoulides");
}

QString Praaline::Plugins::LongAlign::PluginLongAlign::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::LongAlign::PluginLongAlign::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("command", "Command", QVariant::String, d->command);
    return parameters;
}

void Praaline::Plugins::LongAlign::PluginLongAlign::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("command")) d->command = parameters.value("command").toString();
}

void Praaline::Plugins::LongAlign::PluginLongAlign::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{     
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->hasRecordings()) continue;
        // QPointer<CorpusRecording> rec = com->recordings().first();
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
//            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);

//            QList<Interval *> tokens;

//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;
//                IntervalTier *tier_token = tiers->getIntervalTierByName("tok_min");
//                if (!tier_token) continue;
//                foreach (Interval *token, tier_token->intervals()) {
//                    token->setAttribute("speakerID", speakerID);
//                    tokens << token;
//                }
//            }

//            QString filenameBase = QString("d:/SPHINX/valibel_lm/%1").arg(annotationID);

//            LanguageModel::createSentencesAndVocabularyFromTokens(filenameBase, tokens);
//            LanguageModel::createModel(filenameBase, true);

//            printMessage(annotationID);
//            tokens.clear();

//            qDeleteAll(tiersAll);

            printMessage(QString("pocketsphinx_continuous -hmm sphinx/lium_french_f0 -dict sphinx/frenchWords62K.dic ") +
                         QString("-lm valibel_lm/%1.lm.dmp -time yes -infile valibel_wav16k/%1.wav > valibel_1/%1.align").arg(annotationID));
        }
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::LongAlign;
    Q_EXPORT_PLUGIN2(PluginLongAlign, PluginLongAlign)
#endif
