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
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/serialisers/xml/XMLSerialiserCorpusBookmark.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "ProsodicBoundariesAnnotator.h"
#include "SyllableProminenceAnnotator.h"
#include "SpeechRateEstimator.h"
#include "ProsodicUnitsAnnotator.h"
#include "PluginPromise.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Promise::PluginPromisePrivateData {
    PluginPromisePrivateData() :
        command(0), createLevels(true), overwrite(true),
        levelPhone("phone"), levelSyllable("syll"), levelToken("tok_min"), attributePOS("pos_min"),
        attributeProminence_POS("promise_pos"), attributeProminence_NoPOS("promise_nopos"),
        attributeBoundaries("promise_boundary"), attributeContour("promise_contour"),
        modelProminence_POS("cross_pos.model"), modelProminence_NoPOS("cross_nopos.model"),
        modelBoundary("promise_boundary.model"), modelBoundaryContours("promise_contour.model"),
        createFeatureTable(false), createCRFData(false),
        speechrateWindowLeft(250), speechrateWindowRight(250),
        createEvaluationCRFData(false), evaluationNumberOfFolds(5)
    {}

    int command;
    bool createLevels;
    bool overwrite;
    // Input attributes
    QString levelPhone;
    QString levelSyllable;
    QString levelToken;
    QString attributePOS;
    // Output attributes
    QString attributeProminence_POS;
    QString attributeProminence_NoPOS;
    QString attributeBoundaries;
    QString attributeContour;
    // Statistical models
    QString modelPath;
    QString modelProminence_POS;
    QString modelProminence_NoPOS;
    QString modelBoundary;
    QString modelBoundaryContours;
    // Options
    bool usePOS;
    bool createFeatureTable;
    bool createCRFData;
    // Speech rate
    int speechrateWindowLeft;
    int speechrateWindowRight;
    // Evaluation
    bool createEvaluationCRFData;
    int evaluationNumberOfFolds;
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
    return QString(tr("Copyright") + " 2014-2017, George Christodoulides");
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
                                  "Speech rate estimator" <<
                                  "Create Prosodic Unit annotation");
    parameters << PluginParameter("createLevels", "Create annotation attributes when they do not exist?", QVariant::Bool, d->createLevels);
    parameters << PluginParameter("overwrite", "Overwrite existing annotations/values?", QVariant::Bool, d->overwrite);
    // Input attributes
    parameters << PluginParameter("levelPhone",     "Input annotation level: Phones", QVariant::String, d->levelPhone);
    parameters << PluginParameter("levelSyllable",  "Input annotation level: Syllables", QVariant::String, d->levelSyllable);
    parameters << PluginParameter("levelToken",     "Input annotation level: Tokens (minimal)", QVariant::String, d->levelToken);
    parameters << PluginParameter("attributePOS",   "Input annotation attribute: Part-of-Speech of tokens", QVariant::String, d->attributePOS);
    // Output attributes
    parameters << PluginParameter("attributeProminence_POS",    "Output annotation attribute: Syllabic prominence (using POS)", QVariant::String, d->attributeProminence_POS);
    parameters << PluginParameter("attributeProminence_NoPOS",  "Output annotation attribute: Syllabic prominence (without POS)", QVariant::String, d->attributeProminence_NoPOS);
    parameters << PluginParameter("attributeBoundaries",        "Output annotation attribute: Prosodic boundaries", QVariant::String, d->attributeBoundaries);
    parameters << PluginParameter("attributeContour",           "Output annotation attribute: Prosodic boundary contours", QVariant::String, d->attributeContour);
    // Statistical models
    parameters << PluginParameter("modelProminence_POS",    "Model for prominence using POS", QVariant::String, d->modelProminence_POS);
    parameters << PluginParameter("modelProminence_NoPOS",  "Model for prominence without POS", QVariant::String, d->modelProminence_NoPOS);
    parameters << PluginParameter("modelBoundaries",        "Model for boundaries", QVariant::String, d->modelBoundary);
    parameters << PluginParameter("modelBoundaryContours",  "Model for boundary contours", QVariant::String, d->modelBoundaryContours);
    // Options
    parameters << PluginParameter("createFeatureTable", "Create feature table? (for statistical analysis)", QVariant::Bool, d->createFeatureTable);
    parameters << PluginParameter("createCRFData", "Create CRF data file? (for training/analysis)", QVariant::Bool, d->createCRFData);

    // Speech rate estimator
    parameters << PluginParameter("speechrateWindowLeft", "Speech rate: window left (ms)", QVariant::Int, d->speechrateWindowLeft);
    parameters << PluginParameter("speechrateWindowRight", "Speech rate: window right (ms)", QVariant::Int, d->speechrateWindowRight);
    return parameters;
}

void Praaline::Plugins::Promise::PluginPromise::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("command"))                 d->command = parameters.value("command").toInt();
    if (parameters.contains("createLevels"))            d->createLevels = parameters.value("createLevels").toBool();
    if (parameters.contains("overwrite"))               d->overwrite = parameters.value("overwrite").toBool();
    // Input attributes
    if (parameters.contains("levelPhone"))              d->levelPhone = parameters.value("levelPhone").toString();
    if (parameters.contains("levelSyllable"))           d->levelSyllable = parameters.value("levelSyllable").toString();
    if (parameters.contains("levelToken"))              d->levelToken = parameters.value("levelToken").toString();
    if (parameters.contains("attributePOS"))            d->attributePOS = parameters.value("attributePOS").toString();
    // Output attributes
    if (parameters.contains("attributeProminence_POS"))     d->attributeProminence_POS = parameters.value("attributeProminence_POS").toString();
    if (parameters.contains("attributeProminence_NoPOS"))   d->attributeProminence_NoPOS = parameters.value("attributeProminence_NoPOS").toString();
    if (parameters.contains("attributeBoundaries"))         d->attributeBoundaries = parameters.value("attributeBoundaries").toString();
    if (parameters.contains("attributeContour"))            d->attributeContour = parameters.value("attributeContour").toString();
    // Statistical models
    if (parameters.contains("modelProminence_POS"))     d->modelProminence_POS = parameters.value("modelProminence_POS").toString();
    if (parameters.contains("modelProminence_NoPOS"))   d->modelProminence_NoPOS = parameters.value("modelProminence_NoPOS").toString();
    if (parameters.contains("modelBoundaries"))         d->modelBoundary = parameters.value("modelBoundaries").toString();
    if (parameters.contains("modelBoundaryContours"))   d->modelBoundaryContours = parameters.value("modelBoundaryContours").toString();
    // Options
    if (parameters.contains("createFeatureTable"))      d->createFeatureTable = parameters.value("createFeatureTable").toBool();
    if (parameters.contains("createCRFData"))           d->createCRFData = parameters.value("createCRFData").toBool();

}

void Praaline::Plugins::Promise::PluginPromise::createAttribute(CorpusRepository *repository, AnnotationStructureLevel *level, const QString &prefix,
                                                                const QString &ID, const QString &name /*= QString()*/, const QString &description /*= QString()*/,
                                                                const DataType &datatype /*= DataType(DataType::VarChar, 256)*/, int order /*= 0*/,
                                                                bool indexed /*= false*/, const QString &nameValueList /*= QString()*/)
{
    if (level->hasAttribute(ID)) return;
    AnnotationStructureAttribute *attr = new AnnotationStructureAttribute(prefix + ID, name, description, datatype,
                                                                          order, indexed, nameValueList);
    if (repository->annotations()->createAnnotationAttribute(level->ID(), attr))
        level->addAttribute(attr);
}

void Praaline::Plugins::Promise::PluginPromise::createPromiseSyllableInfoStructure(CorpusRepository *repository)
{
    if (!repository) return;
    // If need be, create syllables level
    AnnotationStructureLevel *level_syll = repository->annotationStructure()->level(d->levelSyllable);
    if (!level_syll) {
        level_syll = new AnnotationStructureLevel(d->levelSyllable, AnnotationStructureLevel::IndependentIntervalsLevel, "Syllables", "");
        if (!repository->annotations()->createAnnotationLevel(level_syll)) return;
        repository->annotationStructure()->addLevel(level_syll);
    }
    // Create syllable attributes where necessary
    // ...syllabic prosodic prominence
    createAttribute(repository, level_syll, "", d->attributeProminence_NoPOS, "Prominence (Promise, No POS)", "Syllabic prominence based on acoustic data only (no POS)");
    createAttribute(repository, level_syll, "", d->attributeProminence_POS, "Prominence (Promise)", "Syllabic prominence based on acoustic data and part-of-speech annotation");
    // ... prosodic boundaries
    createAttribute(repository, level_syll, "", d->attributeBoundaries, "Prosodic Boundary (Promise)", "Prosodic boundary strength");
    createAttribute(repository, level_syll, "", d->attributeContour, "Prosodic Boundary Contour (Promise)", "Prosodic boundary contour");
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
                for (RealTime centre = tier_syll->tMin(); centre <= tier_syll->tMax(); centre = centre + RealTime::fromMilliseconds(100)) {
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

    QList<QPointer<CorpusRepository> > repositoriesWithAnnotationStructure;

    SyllableProminenceAnnotator *promise = new SyllableProminenceAnnotator();
    promise->setModelsPath(QDir::homePath() + "/Praaline/plugins/promise/");
    promise->setModelFilenameWithoutPOS(d->modelProminence_NoPOS);
    promise->setModelFilenameWithPOS(d->modelProminence_POS);
    promise->setAttributeProminenceTrain("prom");
    promise->setAttributeDelivery("delivery");

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
        // Create annotation levels and attributes if requested to do so
        if (d->createLevels && !repositoriesWithAnnotationStructure.contains(com->repository())) {
            // Create and/or add attributes to syllable level
            createPromiseSyllableInfoStructure(com->repository());
            // This repository has been processed
            repositoriesWithAnnotationStructure << com->repository();
        }
        // Check that the attribute the syllable level and the attribute to store results exists
        if (!com->repository()->annotationStructure()->hasLevel(d->levelSyllable)) {
            printMessage(QString("Communication ID %1: Annotation level for syllables %2 not found. Aborting.")
                         .arg(com->ID()).arg(d->levelSyllable));
            continue;
        } else if (com->repository()->annotationStructure()->level(d->levelSyllable)) {
            if ((!com->repository()->annotationStructure()->level(d->levelSyllable)->hasAttribute(d->attributeProminence_NoPOS)) &&
                (!d->attributeProminence_NoPOS.isEmpty())) {
                printMessage(QString("Communication ID %1: Annotation attribute to store results (%2) not found. "
                                     "Please create this attribute using the Annotation Structure editor before running Promise. Aborting.")
                             .arg(com->ID()).arg(d->attributeProminence_NoPOS));
                continue;
            }
            if ((!com->repository()->annotationStructure()->level(d->levelSyllable)->hasAttribute(d->attributeProminence_POS)) &&
                (!d->attributeProminence_POS.isEmpty())) {
                printMessage(QString("Communication ID %1: Annotation attribute to store results (%2) not found. "
                                     "Please create this attribute using the Annotation Structure editor before running Promise. Aborting.")
                             .arg(com->ID()).arg(d->attributeProminence_POS));
                continue;
            }
        }
        // Start annotation
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                QPointer<IntervalTier> tier_syll = tiers->getIntervalTierByName(d->levelSyllable);
                if (!tier_syll)  { printMessage(QString("   Annotation level for syllables %1 not found. Aborting.").arg(d->levelSyllable)); continue; }
                QPointer<IntervalTier> tier_token = tiers->getIntervalTierByName(d->levelToken);
                if (!tier_token) { printMessage(QString("   Annotation level for tokens %1 not found. Aborting.").arg(d->levelToken)); continue; }

                IntervalTier *tier_promise_nopos(0), *tier_promise_pos(0);
                if (!d->attributeProminence_NoPOS.isEmpty())
                    tier_promise_nopos = promise->annotate(annot->ID(), speakerID, d->attributeProminence_NoPOS, tier_syll, tier_token, false);
                if (!d->attributeProminence_POS.isEmpty())
                    tier_promise_pos = promise->annotate(annot->ID(), speakerID, d->attributeProminence_POS, tier_syll, tier_token, true);

                // Save automatic annotation if asked to do so
                if (d->overwrite) {
                    for (int i = 0; i < tier_syll->count(); ++i) {
                        if (tier_promise_nopos && (i < tier_promise_nopos->count()) && (!d->attributeProminence_NoPOS.isEmpty())) {
                            tier_syll->interval(i)->setAttribute(d->attributeProminence_NoPOS, tier_promise_nopos->interval(i)->text());
                        }
                        if (tier_promise_pos && (i < tier_promise_pos->count()) && (!d->attributeProminence_POS.isEmpty())) {
                            tier_syll->interval(i)->setAttribute(d->attributeProminence_POS, tier_promise_pos->interval(i)->text());
                        }
                    }
                    // Save tier
                    if (com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_syll)) {
                        qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);
                    }
                }

                // Clean up
                if (tier_promise_nopos) delete tier_promise_nopos;
                if (tier_promise_pos) delete tier_promise_pos;
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

    QList<QPointer<CorpusRepository> > repositoriesWithAnnotationStructure;

    ProsodicBoundariesAnnotator *promise = new ProsodicBoundariesAnnotator();
    promise->setModelsPath(QDir::homePath() + "/Praaline/plugins/promise/");
    promise->setModelFilenameBoundary(d->modelBoundary);
    promise->setModelFilenameBoundaryCountours(d->modelBoundaryContours);
    promise->setAttributeBoundaryTrain(d->attributeBoundaries);
    promise->setAttributeBoundaryContourTrain(d->attributeContour);

    if (d->createFeatureTable) {
        QString filenameFeaturesTable = QDir::homePath() + "/promise_boundary_features.txt";
        if (promise->openFeaturesTableFile(filenameFeaturesTable))
            printMessage(QString("Features table will be saved in: %1").arg(filenameFeaturesTable));
        else
            printMessage(QString("Error opening file for writing the features table: %1").arg(filenameFeaturesTable));
    }
    if (d->createCRFData) {
        QString filenameCRFData = QDir::homePath() + "/promise_boundary_crf.txt";
        if (promise->openCRFDataFile(filenameCRFData))
            printMessage(QString("CRF data will be saved in: %1").arg(filenameCRFData));
        else
            printMessage(QString("Error opening file for writing CRF data: %1").arg(filenameCRFData));
    }

    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        if (!com->repository()) continue;
        // Create annotation levels and attributes if requested to do so
        if (d->createLevels && !repositoriesWithAnnotationStructure.contains(com->repository())) {
            // Create and/or add attributes to syllable level
            createPromiseSyllableInfoStructure(com->repository());
            // This repository has been processed
            repositoriesWithAnnotationStructure << com->repository();
        }
        // Check that the attribute the syllable level and the attribute to store results exists
        if (!com->repository()->annotationStructure()->hasLevel(d->levelSyllable)) {
            printMessage(QString("Communication ID %1: Annotation level for syllables %2 not found. Aborting.")
                         .arg(com->ID()).arg(d->levelSyllable));
            continue;
        } else if (com->repository()->annotationStructure()->level(d->levelSyllable)) {
            if ((!com->repository()->annotationStructure()->level(d->levelSyllable)->hasAttribute(d->attributeBoundaries)) &&
                (!d->attributeBoundaries.isEmpty())) {
                printMessage(QString("Communication ID %1: Annotation attribute to store results (%2) not found. "
                                     "Please create this attribute using the Annotation Structure editor before running Promise. Aborting.")
                             .arg(com->ID()).arg(d->attributeBoundaries));
                continue;
            }
            if ((!com->repository()->annotationStructure()->level(d->levelSyllable)->hasAttribute(d->attributeContour)) &&
                (!d->attributeContour.isEmpty())) {
                printMessage(QString("Communication ID %1: Annotation attribute to store results (%2) not found. "
                                     "Please create this attribute using the Annotation Structure editor before running Promise. Aborting.")
                             .arg(com->ID()).arg(d->attributeContour));
                continue;
            }
        }
        // Start annotation
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

                IntervalTier *tier_promise_boundary(0), *tier_promise_contour(0);
                if (!d->attributeBoundaries.isEmpty())
                    tier_promise_boundary = promise->annotate(annot->ID(), speakerID, d->attributeBoundaries, tier_syll, tier_token, false);
                if (!d->attributeContour.isEmpty())
                    tier_promise_contour = promise->annotate(annot->ID(), speakerID, d->attributeContour, tier_syll, tier_token, true);

                // Save automatic annotation if asked to do so
                if (d->overwrite) {
                    for (int i = 0; i < tier_syll->count(); ++i) {
                        if (tier_promise_boundary && (i < tier_promise_boundary->count()) && (!d->attributeBoundaries.isEmpty())) {
                            tier_syll->interval(i)->setAttribute(d->attributeBoundaries, tier_promise_boundary->interval(i)->text());
                        }
                        if (tier_promise_contour && (i < tier_promise_contour->count()) && (!d->attributeContour.isEmpty())) {
                            tier_syll->interval(i)->setAttribute(d->attributeContour, tier_promise_contour->interval(i)->text());
                        }
                    }
                    // Save tier
                    if (com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_syll)) {
                        qDebug() << QString("Annotated %1, speaker %2").arg(annot->ID()).arg(speakerID);
                    }
                }

                // Clean up
                if (tier_promise_boundary) delete tier_promise_boundary;
                if (tier_promise_contour) delete tier_promise_contour;
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

void Praaline::Plugins::Promise::PluginPromise::runProsodicUnitsAnnotator(const QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Promise: Prosodic Units annotation");
    ProsodicUnitsAnnotator units;
    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(units.createProsodicUnitsTierBoundaries(com, "B3", false));
        QApplication::processEvents();
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::Promise::PluginPromise::process(const QList<QPointer<CorpusCommunication> > &communications)
{    
    if      (d->command == 0) runSyllableProminenceAnnotator(communications);
    else if (d->command == 1) runProsodicBoundariesAnnotator(communications);
    else if (d->command == 2) runSpeechRateEstimator(communications);
    else if (d->command == 3) runProsodicUnitsAnnotator(communications);

    madeProgress(100);
    printMessage("Promise plugin finished.");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Promise;
    Q_EXPORT_PLUGIN2(PluginPromise, PluginPromise)
#endif



