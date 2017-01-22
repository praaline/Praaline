#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <ExtensionSystemConstants>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusBookmark.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/serialisers/xml/XMLSerialiserCorpusBookmark.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "ProsodicBoundariesAnnotator.h"
#include "SyllableProminenceAnnotator.h"
#include "SpeechRateEstimator.h"
#include "PluginPromise.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Promise::PluginPromisePrivateData {
    PluginPromisePrivateData() :
        command(0), createLevels(true), overwrite(true),
        levelPhone("phone"), levelSyllable("syll"), levelToken("tok_min"), attributePOS("pos_min"),
        usePOS(true), createFeatureTable(false), createCRFData(false),
        modelProminence_POS("cross_pos.model"), modelProminence_NoPOS("cross_nopos.model"),  modelBoundaries("boundaries.model"),
        speechrateWindowLeft(250), speechrateWindowRight(250)
    {}

    int command;
    bool createLevels;
    bool overwrite;
    QString levelPhone;
    QString levelSyllable;
    QString levelToken;
    QString attributePOS;
    bool usePOS;
    bool createFeatureTable;
    bool createCRFData;
    QString modelProminence_POS;
    QString modelProminence_NoPOS;
    QString modelBoundaries;
    int speechrateWindowLeft;
    int speechrateWindowRight;
};

Praaline::Plugins::Promise::PluginPromise::PluginPromise(QObject* parent) : QObject(parent)
{
    d = new PluginPromisePrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Promise::PluginPromise::~PluginPromise()
{
    delete d;
}

bool Praaline::Plugins::Promise::PluginPromise::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Promise::PluginPromise::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Promise::PluginPromise::finalize() {

}

QString Praaline::Plugins::Promise::PluginPromise::pluginName() const {
    return "Promise";
}

QtilitiesCategory Praaline::Plugins::Promise::PluginPromise::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Promise::PluginPromise::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Promise::PluginPromise::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Promise::PluginPromise::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Promise::PluginPromise::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Promise::PluginPromise::pluginDescription() const {
    return tr("Promise is a prosodic prominence annotator.");
}

QString Praaline::Plugins::Promise::PluginPromise::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::Promise::PluginPromise::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Promise::PluginPromise::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("command", "Function to apply", QVariant::Int, d->command, QStringList() <<
                                  "Annotate syllabic prominence" <<
                                  "Annotate prosodic boundaries" <<
                                  "Speech rate estimator");
    parameters << PluginParameter("createLevels", "Create annotation attributes when they do not exist?", QVariant::Bool, d->createLevels);
    parameters << PluginParameter("overwrite", "Overwrite existing annotations/values?", QVariant::Bool, d->overwrite);
    parameters << PluginParameter("levelPhone", "Annotation level: Phones", QVariant::String, d->levelPhone);
    parameters << PluginParameter("levelSyllable", "Annotation level: Syllables", QVariant::String, d->levelSyllable);
    parameters << PluginParameter("levelToken", "Annotation level: Tokens (minimal)", QVariant::String, d->levelToken);
    parameters << PluginParameter("attributePOS", "Annotation attribute: Part-of-Speech of tokens", QVariant::String, d->attributePOS);
    parameters << PluginParameter("createFeatureTable", "Create feature table? (for statistical analysis)", QVariant::Bool, d->createFeatureTable);
    parameters << PluginParameter("createCRFData", "Create CRF data file? (for training/analysis)", QVariant::Bool, d->createCRFData);
    parameters << PluginParameter("modelProminence_NoPOS", "Model for prominence without POS", QVariant::String, d->modelProminence_NoPOS);
    parameters << PluginParameter("modelProminence_POS", "Model for prominence with POS", QVariant::String, d->modelProminence_POS);
    parameters << PluginParameter("modelBoundaries", "Model for boundaries", QVariant::String, d->modelBoundaries);
    parameters << PluginParameter("speechrateWindowLeft", "Speech rate: window left (ms)", QVariant::Int, d->speechrateWindowLeft);
    parameters << PluginParameter("speechrateWindowRight", "Speech rate: window right (ms)", QVariant::Int, d->speechrateWindowRight);
    return parameters;
}

void Praaline::Plugins::Promise::PluginPromise::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("command"))                 d->command = parameters.value("command").toInt();
    if (parameters.contains("createLevels"))            d->createLevels = parameters.value("createLevels").toBool();
    if (parameters.contains("overwrite"))               d->overwrite = parameters.value("overwrite").toBool();
    if (parameters.contains("levelSyllable"))           d->levelSyllable = parameters.value("levelSyllable").toString();
    if (parameters.contains("levelToken"))              d->levelToken = parameters.value("levelToken").toString();
    if (parameters.contains("attributePOS"))            d->attributePOS = parameters.value("attributePOS").toString();
    if (parameters.contains("createFeatureTable"))      d->createFeatureTable = parameters.value("createFeatureTable").toBool();
    if (parameters.contains("createCRFData"))           d->createCRFData = parameters.value("createCRFData").toBool();
    if (parameters.contains("modelProminence_NoPOS"))   d->modelProminence_NoPOS = parameters.value("modelProminence_NoPOS").toString();
    if (parameters.contains("modelProminence_POS"))     d->modelProminence_POS = parameters.value("modelProminence_POS").toString();
    if (parameters.contains("modelBoundaries"))         d->modelBoundaries = parameters.value("modelBoundaries").toString();
}


void Praaline::Plugins::Promise::PluginPromise::runSpeechRateEstimator(const QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Promise: Speech Rate Estimator");
    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->repository()) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllable);
                if (!tier_syll) { printMessage("   Tier not found: syll (syllables). Aborting."); continue; }
                QList<Point *> points;
                for (RealTime centre = tier_syll->tMin(); centre <= tier_syll->tMax(); centre = centre + RealTime::fromMilliseconds(10)) {
                    points << new Point(centre, "");
                }
                PointTier *tier_speechrate = new PointTier("speech_rate", points);
                SpeechRateEstimator::calculate(tier_speechrate, tier_syll,
                                               RealTime::fromMilliseconds(d->speechrateWindowLeft),
                                               RealTime::fromMilliseconds(d->speechrateWindowRight), "rate_syll");
                IntervalTier *tier_phone = tiers->getIntervalTierByName(d->levelPhone);
                if (tier_phone) {
                    SpeechRateEstimator::calculate(tier_speechrate, tier_phone,
                                                   RealTime::fromMilliseconds(d->speechrateWindowLeft),
                                                   RealTime::fromMilliseconds(d->speechrateWindowRight), "rate_phone");
                }
                com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_speechrate);
                delete tier_speechrate;
            }
            qDeleteAll(tiersAll);
            QApplication::processEvents();
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::Promise::PluginPromise::runSyllableProminenceAnnotator(const QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Promise Prosodic Prominence Annotator ver. 1.0 running");

    SyllableProminenceAnnotator *promise = new SyllableProminenceAnnotator();
    promise->setModelsPath(QCoreApplication::applicationDirPath() + "/plugins/promise/");
    promise->setModelFilenameWithoutPOS(d->modelProminence_NoPOS);
    promise->setModelFilenameWithPOS(d->modelProminence_POS);

    if (d->createFeatureTable) {
        QString filenameFeaturesTable = QDir::homePath() + "/promise_prominence_features.txt";
        if (promise->openFeaturesTableFile(filenameFeaturesTable))
            printMessage(QString("Features table will be saved in: %1").arg(filenameFeaturesTable));
    }
    if (d->createCRFData) {
        QString filenameCRFData = QDir::homePath() + "/promise_prominence_crf.txt";
        if (promise->openCRFDataFile(filenameCRFData))
            printMessage(QString("CRF data will be saved in: %1").arg(filenameCRFData));
    }

    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->repository()) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                QPointer<IntervalTier> tier_syll = tiers->getIntervalTierByName(d->levelSyllable);
                if (!tier_syll) { printMessage(QString("   Annotation level for syllables %1 not found. Aborting.").arg(d->levelSyllable)); continue; }
                QPointer<IntervalTier> tier_token = tiers->getIntervalTierByName(d->levelToken);
                if (!tier_token) { printMessage(QString("   Annotation level for tokens %1 not found. Aborting.").arg(d->levelToken)); continue; }

                IntervalTier *tier_promise_nopos = promise->annotate(annot->ID(), speakerID, "promise_nopos", tier_syll, tier_token, false);
                IntervalTier *tier_promise_pos = promise->annotate(annot->ID(), speakerID, "promise_pos", tier_syll, tier_token, true);
                for (int i = 0; i < tier_syll->count(); ++i) {
                    if (tier_promise_nopos && i < tier_promise_nopos->count()) {
                        tier_syll->interval(i)->setAttribute("promise", tier_promise_nopos->interval(i)->text());
                    }
                    if (tier_promise_pos && i < tier_promise_pos->count()) {
                        tier_syll->interval(i)->setAttribute("promise_pos", tier_promise_pos->interval(i)->text());
                    }
                }

                com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_syll);
                qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);
                delete tier_promise_nopos;
                delete tier_promise_pos;
            }
            qDeleteAll(tiersAll);
            QApplication::processEvents();
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    if (d->createFeatureTable) promise->closeFeaturesTableFile();
    if (d->createCRFData) promise->closeCRFDataFile();
    delete promise;
}

void Praaline::Plugins::Promise::PluginPromise::runProsodicBoundariesAnnotator(const QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Promise Prosodic Boundaries Annotator ver. 1.0 running");

    ProsodicBoundariesAnnotator *promise = new ProsodicBoundariesAnnotator();
    promise->setModelsPath(QCoreApplication::applicationDirPath() + "/plugins/promise/");
    promise->setModelFilename(d->modelBoundaries);

    if (d->createFeatureTable) {
        QString filenameFeaturesTable = QDir::homePath() + "/promise_boundary_features.txt";
        if (promise->openFeaturesTableFile(filenameFeaturesTable))
            printMessage(QString("Features table will be saved in: %1").arg(filenameFeaturesTable));
    }
    if (d->createCRFData) {
        QString filenameCRFData = QDir::homePath() + "/promise_boundary_crf.txt";
        if (promise->openCRFDataFile(filenameCRFData))
            printMessage(QString("CRF data will be saved in: %1").arg(filenameCRFData));
    }

    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->repository()) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                QPointer<IntervalTier> tier_syll = tiers->getIntervalTierByName(d->levelSyllable);
                if (!tier_syll) { printMessage(QString("   Annotation level for syllables %1 not found. Aborting.").arg(d->levelSyllable)); continue; }
                QPointer<IntervalTier> tier_token = tiers->getIntervalTierByName(d->levelToken);
                if (!tier_token) { printMessage(QString("   Annotation level for tokens %1 not found. Aborting.").arg(d->levelToken)); continue; }

                IntervalTier *tier_promise_boundary = promise->annotate(annot->ID(), speakerID, "promise_boundary", tier_syll, tier_token);

                for (int i = 0; i < tier_syll->count(); ++i) {
                    if (tier_promise_boundary && i < tier_promise_boundary->count()) {
                        tier_syll->interval(i)->setAttribute("promise_boundary", tier_promise_boundary->interval(i)->text());
                    }
                }

                com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_syll);
                qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);
                delete tier_promise_boundary;
            }
            qDeleteAll(tiersAll);
            QApplication::processEvents();
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    if (d->createFeatureTable) promise->closeFeaturesTableFile();
    if (d->createCRFData) promise->closeCRFDataFile();
    delete promise;
}

void Praaline::Plugins::Promise::PluginPromise::process(const QList<QPointer<CorpusCommunication> > &communications)
{    
    if      (d->command == 0) runSyllableProminenceAnnotator(communications);
    else if (d->command == 1) runProsodicBoundariesAnnotator(communications);
    else if (d->command == 2) runSpeechRateEstimator(communications);

    madeProgress(100);
    printMessage("Promise plugin finished.");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Promise;
    Q_EXPORT_PLUGIN2(PluginPromise, PluginPromise)
#endif



