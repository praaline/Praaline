#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

#include "AnonymiserScript.h"
#include "PluginAnonymiser.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Anonymiser::PluginAnonymiserPrivateData {
    PluginAnonymiserPrivateData() :
        levelToAnonymise("segment"), anonymisationLabel("#"), anonymisedRecordingSuffix("_anon"),
        pitchAnalysisDuration(30), pitchAnalysisTimestep(0.01), automaticMinMaxF0(true),
        minF0(60.0), maxF0(700.0), scaleIntensity(1.0)
    {}

    QString levelToAnonymise;
    QString anonymisationLabel;
    QString anonymisedRecordingSuffix;
    // Anonymiser script parameters
    double pitchAnalysisDuration;
    double pitchAnalysisTimestep;
    bool automaticMinMaxF0;
    double minF0;
    double maxF0;
    double scaleIntensity;
};

Praaline::Plugins::Anonymiser::PluginAnonymiser::PluginAnonymiser(QObject* parent) :
    QObject(parent), d(new PluginAnonymiserPrivateData)
{
    setObjectName("Anonymiser");
}

Praaline::Plugins::Anonymiser::PluginAnonymiser::~PluginAnonymiser()
{
    delete d;
}

bool Praaline::Plugins::Anonymiser::PluginAnonymiser::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool Praaline::Plugins::Anonymiser::PluginAnonymiser::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::finalize() {

}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginName() const {
    return "Anonymiser";
}

QtilitiesCategory Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginDescription() const {
    return tr("Anonymiser: An intonation-preserving anonymiser for sound files using a script by Daniel Hirst.");
}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginCopyright() const {
    return QString(tr("Copyright") + " 2021, George Christodoulides");
}

QString Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Anonymiser::PluginAnonymiser::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("levelToAnonymise", "Annotation Level with anonymisation instructions", QVariant::String, d->levelToAnonymise);
    parameters << PluginParameter("anonymisationLabel", "Anonymise all intervals containing this text", QVariant::String, d->anonymisationLabel);
    parameters << PluginParameter("anonymisedRecordingSuffix", "Anonymised audio file suffix", QVariant::String, d->anonymisedRecordingSuffix);
    parameters << PluginParameter("pitchAnalysisDuration", "Pitch analysis: interval duration (seconds)", QVariant::Double, d->pitchAnalysisDuration);
    parameters << PluginParameter("pitchAnalysisTimestep", "Pitch analysis: time step (seconds)", QVariant::Double, d->pitchAnalysisTimestep);
    parameters << PluginParameter("automaticMinMaxF0", "Pitch analysis: automatic min and max F0", QVariant::Bool, d->automaticMinMaxF0);
    parameters << PluginParameter("minF0", "Pitch analysis: minimum F0 in Hz (ignored if auto selected above)", QVariant::Double, d->minF0);
    parameters << PluginParameter("maxF0", "Pitch analysis: maximum F0 in Hz (ignore if auto selected above)", QVariant::Double, d->maxF0);
    parameters << PluginParameter("scaleIntensity", "Scale sound intensity (relative to 1)", QVariant::Double, d->scaleIntensity);
    return parameters;
}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("levelToAnonymise")) d->levelToAnonymise = parameters.value("levelToAnonymise").toString();
    if (parameters.contains("anonymisationLabel")) d->anonymisationLabel = parameters.value("anonymisationLabel").toString();
    if (parameters.contains("anonymisedRecordingSuffix")) d->anonymisedRecordingSuffix = parameters.value("anonymisedRecordingSuffix").toString();
    if (parameters.contains("pitchAnalysisDuration")) d->pitchAnalysisDuration = parameters.value("pitchAnalysisDuration").toDouble();
    if (parameters.contains("pitchAnalysisTimestep")) d->pitchAnalysisTimestep = parameters.value("pitchAnalysisTimestep").toDouble();
    if (parameters.contains("automaticMinMaxF0")) d->automaticMinMaxF0 = parameters.value("automaticMinMaxF0").toBool();
    if (parameters.contains("minF0")) d->minF0 = parameters.value("minF0").toDouble();
    if (parameters.contains("maxF0")) d->maxF0 = parameters.value("maxF0").toDouble();
    if (parameters.contains("scaleIntensity")) d->scaleIntensity = parameters.value("scaleIntensity").toDouble();
}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::scriptSentMessage(const QString &message)
{
    emit printMessage(message);
}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::scriptFinished(int exitcode)
{
    if (exitcode == 0)
        emit printMessage("Finished succesfully.");
    else
        emit printMessage(QString("Finished with errors. The error code was %1").arg(exitcode));
}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::process(const QList<CorpusCommunication *> &communications)
{
    AnonymiserScript *anonymiser = new AnonymiserScript(this);
    connect(anonymiser, &AnnotationPluginPraatScript::logOutput, this, &PluginAnonymiser::scriptSentMessage);
    connect(anonymiser, &AnnotationPluginPraatScript::finished, this, &PluginAnonymiser::scriptFinished);
    // Pass parameters from the plug-in to anonymiser script object
    anonymiser->pitchAnalysisDuration = d->pitchAnalysisDuration;
    anonymiser->pitchAnalysisTimestep = d->pitchAnalysisTimestep;
    anonymiser->automaticMinMaxF0 = d->automaticMinMaxF0;
    anonymiser->minF0 = d->minF0;
    anonymiser->maxF0 = d->maxF0;
    anonymiser->scaleIntensity = d->scaleIntensity;
    // Process
    int countDone = 0;
    emit madeProgress(0);
    emit printMessage("Audio File Anonymiser using Daniel Hirst's script");
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;
            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (!annot) continue;
                emit printMessage(QString("Communication ID: %1\tRecording ID: %2\tAnnotation ID:%3").arg(com->ID(), rec->ID(), annot->ID()));
                // Create tier with anonymisation instructions (buzz tier)
                SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID(), QStringList() << d->levelToAnonymise);
                QList<Interval *> buzzIntervals;
                RealTime buzzTier_tMin, buzzTier_tMax;
                foreach (QString speakerID, tiersAll.keys()) {
                    AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                    if (!tiers) continue;
                    IntervalTier *tierWithAnonLabels = tiers->getIntervalTierByName(d->levelToAnonymise);
                    if (!tierWithAnonLabels) continue;
                    if (buzzTier_tMin > tierWithAnonLabels->tMin()) buzzTier_tMin = tierWithAnonLabels->tMin();
                    if (buzzTier_tMax < tierWithAnonLabels->tMax()) buzzTier_tMax = tierWithAnonLabels->tMax();
                    foreach (Interval *intv, tierWithAnonLabels->intervals()) {
                        if (intv->text().contains(d->anonymisationLabel))
                            buzzIntervals << new Interval(intv->tMin(), intv->tMax(), "buzz");
                    }
                }
                IntervalTier *buzzTier = new IntervalTier("buzz", buzzIntervals, buzzTier_tMin, buzzTier_tMax, this);
                // Figure out where to store the anonymised audio file
                QString anonymisedFilename = QString(rec->filePath()).replace(".wav", d->anonymisedRecordingSuffix + ".wav");
                // Run script
                anonymiser->run(rec, buzzTier, anonymisedFilename);
                delete buzzTier;
            }
        }
        countDone++;
        emit madeProgress(countDone * 100 / communications.count());
    }
    disconnect(anonymiser, &AnnotationPluginPraatScript::logOutput, this, &PluginAnonymiser::scriptSentMessage);
    disconnect(anonymiser, &AnnotationPluginPraatScript::finished, this, &PluginAnonymiser::scriptFinished);
    delete anonymiser;
    emit madeProgress(100);
    emit printMessage("Finished anonymisation.");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Anonymiser;
    Q_EXPORT_PLUGIN2(PluginAnonymiser, PluginAnonymiser)
#endif
