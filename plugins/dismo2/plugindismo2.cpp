#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "plugindismo2.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::DisMo2::PluginDisMo2PrivateData {
    PluginDisMo2PrivateData() :
        alreadyTokenised(false), tokenisedOnlyToMinimal(false),
        levelToAnnotate("segment"), levelTokMin("tok_min"), levelTokMWU("tok_mwu"),
        levelPhones("phone"), attributePrefix("")
    {
        attributeNames["pos_min"] = "pos_min";
        attributeNames["pos_ext_min"] = "pos_ext_min";
        attributeNames["lemma_min"] = "lemma_min";
        attributeNames["disfluency"] = "disfluency";
        attributeNames["pos_mwu"] = "pos_mwu";
        attributeNames["pos_ext_mwu"] = "pos_ext_mwu";
        attributeNames["lemma_mwu"] = "lemma_mwu";
        attributeNames["discourse"] = "discourse";
    }

    bool alreadyTokenised;
    bool tokenisedOnlyToMinimal;
    QString levelToAnnotate;
    QString levelTokMin;
    QString levelTokMWU;
    QString levelPhones;
    QString attributePrefix;
    QHash<QString, QString> attributeNames;
};

Praaline::Plugins::DisMo2::PluginDisMo2::PluginDisMo2(QObject* parent) : QObject(parent)
{
    d = new PluginDisMo2PrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::DisMo2::PluginDisMo2::~PluginDisMo2()
{
}

bool Praaline::Plugins::DisMo2::PluginDisMo2::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::DisMo2::PluginDisMo2::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::DisMo2::PluginDisMo2::finalize() {
    delete d;
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginName() const {
    return "DisMo Annotator version 2";
}

QtilitiesCategory Praaline::Plugins::DisMo2::PluginDisMo2::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::DisMo2::PluginDisMo2::pluginVersionInformation() const {
    VersionInformation version_info(2, 0, 0);
    return version_info;
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginDescription() const {
    return tr("DisMo is a part-of-speech tagger, multi-word unit and disfluency detector for spoken language transcriptions.");
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::DisMo2::PluginDisMo2::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::DisMo2::PluginDisMo2::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("alreadyTokenised", "Already tokenised?", QVariant::Bool, d->alreadyTokenised);
    parameters << PluginParameter("tokenisedOnlyToMinimal", "Tokenised only to minimal tokens?", QVariant::Bool, d->tokenisedOnlyToMinimal);
    parameters << PluginParameter("levelToAnnotate", "Level to annotate", QVariant::String, d->levelToAnnotate);
    parameters << PluginParameter("levelTokMin", "Level for tokens (minimal)", QVariant::String, d->levelTokMin);
    parameters << PluginParameter("levelTokMWU", "Level for tokens (multi-word units)", QVariant::String, d->levelTokMWU);
    parameters << PluginParameter("levelPhones", "Level for phones (optional)", QVariant::String, d->levelPhones);
    parameters << PluginParameter("attributePrefix", "Prefix attribute names", QVariant::String, d->attributePrefix);
    return parameters;
}

void Praaline::Plugins::DisMo2::PluginDisMo2::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("alreadyTokenised")) d->alreadyTokenised = parameters.value("alreadyTokenised").toBool();
    if (parameters.contains("tokenisedOnlyToMinimal")) d->tokenisedOnlyToMinimal = parameters.value("tokenisedOnlyToMinimal").toBool();
    if (parameters.contains("levelToAnnotate")) d->levelToAnnotate = parameters.value("levelToAnnotate").toString();
    if (parameters.contains("levelTokMin")) d->levelTokMin = parameters.value("levelTokMin").toString();
    if (parameters.contains("levelTokMWU")) d->levelTokMWU = parameters.value("levelTokMWU").toString();
    if (parameters.contains("levelPhones")) d->levelPhones = parameters.value("levelPhones").toString();
    if (parameters.contains("attributePrefix")) d->attributePrefix = parameters.value("attributePrefix").toString();
}

void Praaline::Plugins::DisMo2::PluginDisMo2::process(const QList<CorpusCommunication *> &communications)
{
//    DisMoAnnotator::DismoAnnotator *DISMO = new DisMoAnnotator::DismoAnnotator("fr");
//    QPointer<IntervalTier> tier_tok_min;
//    QPointer<IntervalTier> tier_tok_mwu;

//    int countDone = 0;
//    madeProgress(0);
//    printMessage("DisMo Annotator ver. 1.0 running");
//    foreach(CorpusCommunication *com, communications) {
//        if (!com) continue;
//        printMessage(QString("Annotating %1").arg(com->ID()));
//        foreach (CorpusAnnotation *annot, com->annotations()) {
//            if (!annot) continue;
//            SpeakerAnnotationTierGroupMap tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
//            foreach (QString speakerID, tiersAll.keys()) {
//                printMessage(QString("   speaker %1").arg(speakerID));
//                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;
//                if (!d->alreadyTokenised) {
//                    QPointer<IntervalTier> tier_input = tiers->getIntervalTierByName(d->levelToAnnotate);
//                    QPointer<IntervalTier> tier_phone = tiers->getIntervalTierByName(d->levelPhones);
//                    tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
//                    tier_tok_mwu = tiers->getIntervalTierByName(d->levelTokMWU);
//                    if (!tier_tok_min) tier_tok_min = new IntervalTier(d->levelTokMin, tiers->tMin(), tiers->tMax());
//                    if (!tier_tok_mwu) tier_tok_mwu = new IntervalTier(d->levelTokMWU, tiers->tMin(), tiers->tMax());
//                    DISMO->annotate(tier_input, tier_tok_min, tier_tok_mwu, tier_phone);
//                }
//                else {
//                    if (d->tokenisedOnlyToMinimal) {
//                        tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
//                        if (!tier_tok_min) continue;
//                        tier_tok_min->fillEmptyAnnotationsWith("_");
//                        tier_tok_mwu = new IntervalTier(d->levelTokMWU, tiers->tMin(), tiers->tMax());
//                        DISMO->annotateTokenizedToMinimal(tier_tok_min, tier_tok_mwu);
//                    }
//                    else {
//                        tier_tok_min = tiers->getIntervalTierByName(d->levelTokMin);
//                        tier_tok_mwu = tiers->getIntervalTierByName(d->levelTokMWU);
//                        if (!tier_tok_min || !tier_tok_mwu) continue;
//                        tier_tok_min->fillEmptyAnnotationsWith("_");
//                        tier_tok_mwu->fillEmptyAnnotationsWith("_");
//                        QHash<QString, QString> attr;
//                        foreach (QString attrName, d->attributeNames.keys()) {
//                            attr.insert(d->attributePrefix + attrName, d->attributeNames.value(attrName));
//                        }
//                        DISMO->annotateTokenized(tier_tok_min, tier_tok_mwu, attr);
//                    }
//                }
//                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_min);
//                corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_tok_mwu);
//                qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);
//                delete tier_tok_min;
//                delete tier_tok_mwu;
//            }
//            qDeleteAll(tiersAll);
//            QApplication::processEvents();
//        }
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
//    }
//    delete DISMO;
    madeProgress(100);
    printMessage("DisMo finished.");
}
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::DisMo2;
    Q_EXPORT_PLUGIN2(PluginDisMo2, PluginDisMo2)
#endif
