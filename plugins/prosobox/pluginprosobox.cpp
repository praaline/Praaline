#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "pluginprosobox.h"
#include "prosoboxscript.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Prosobox::PluginProsoboxPrivateData {
    PluginProsoboxPrivateData() {}
};

Praaline::Plugins::Prosobox::PluginProsobox::PluginProsobox(QObject* parent) : QObject(parent)
{
    d = new PluginProsoboxPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Prosobox::PluginProsobox::~PluginProsobox()
{
    delete d;
}

bool Praaline::Plugins::Prosobox::PluginProsobox::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Prosobox::PluginProsobox::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Prosobox::PluginProsobox::finalize() {

}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginName() const {
    return "Prosobox";
}

QtilitiesCategory Praaline::Plugins::Prosobox::PluginProsobox::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Prosobox::PluginProsobox::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginPublisher() const {
    return "Jean-Philippe Goldman, George Christodoulides";
}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginDescription() const {
    return tr("Prosobox is a collection of tools for prosodic analysis.");
}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, Jean-Philipee Goldman / George Christodoulides");
}

QString Praaline::Plugins::Prosobox::PluginProsobox::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Prosobox::PluginProsobox::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    return parameters;
}

void Praaline::Plugins::Prosobox::PluginProsobox::setParameters(const QHash<QString, QVariant> &parameters)
{
    Q_UNUSED(parameters)
}

QString temporalVariables(CorpusCommunication *com, QString annotationID, QString speakerID)
{
    AnnotationTierGroup *tiers = com->repository()->annotations()->getTiers(annotationID, speakerID);
    if (!tiers) return QString();
    IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
    IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
    IntervalTier *tier_timeline = com->repository()->annotations()->getSpeakerTimeline("", annotationID, "segment");
    if (tier_timeline) {
        tier_timeline->replace("", speakerID, "L1");
        tier_timeline->replace("", "L1+L2", "L1");
        tier_timeline->replace("", "L2+L1", "L1");
        tier_timeline->fillEmptyWith("", "_");
        tier_timeline->mergeIdenticalAnnotations(QString(), QStringList() << "_");
        tier_timeline->setName("speaker");
    }
    double timeSpeech = 0.0;
    double timeArticulation = 0.0;
    double timePause = 0.0;
    double timeFilledPause = 0.0;
    int numberOfPauses = 0;
    int numberOfArticulatedSyllables = 0;
    int numberOfFilledPauses = 0;
    foreach (Interval *turn, tier_timeline->intervals()) {
        if (turn->text() != "L1") continue;
        QList<Interval *> syllables = tier_syll->getIntervalsContainedIn(turn);
        timeSpeech += turn->duration().toDouble();
        foreach (Interval *syll, syllables) {
            Interval *correspondingTokMin = tier_tok_min->intervalAtTime(syll->tCenter());
            if (syll->isPauseSilent()) {
                numberOfPauses++;
                timePause += syll->duration().toDouble();
            } else {
                if (correspondingTokMin && correspondingTokMin->text() == "euh") {
                    numberOfFilledPauses++;
                    timeFilledPause += syll->duration().toDouble();
                } else {
                    numberOfArticulatedSyllables++;
                    timeArticulation += syll->duration().toDouble();
                }
            }
        }
    }
    double ratioArticulation = timeArticulation / timeSpeech * 100.0;
    double ratioPause = timePause / timeSpeech * 100.0;
    double ratioFilledPause = timeFilledPause / timeSpeech * 100.0;
    double rateSpeech = ((double)numberOfArticulatedSyllables) / timeSpeech;
    double rateArticulation = ((double)numberOfArticulatedSyllables) / timeArticulation;
    QString result;
    result.append(annotationID).append("\t").append(speakerID).append("\t");
    result.append(QString::number(timeSpeech)).append("\t");
    result.append(QString::number(timeArticulation)).append("\t");
    result.append(QString::number(timePause)).append("\t");
    result.append(QString::number(timeFilledPause)).append("\t");
    result.append(QString::number(ratioArticulation)).append("\t");
    result.append(QString::number(ratioPause)).append("\t");
    result.append(QString::number(ratioFilledPause)).append("\t");
    result.append(QString::number(rateSpeech)).append("\t");
    result.append(QString::number(rateArticulation));
    return result;
}

void Praaline::Plugins::Prosobox::PluginProsobox::process(const QList<CorpusCommunication *> &communications)
{
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                if (speakerID == "L2") continue;
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                // ProsoboxScript::createTextGrid(corpus->baseMediaPath(), corpus, annotationID, speakerID);
                // QString t = temporalVariables(corpus, annotationID, speakerID);
                // emit printMessage(t);
            }
            qDeleteAll(tiersAll);
        }
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Prosobox;
    Q_EXPORT_PLUGIN2(PluginProsobox, PluginProsobox)
#endif
