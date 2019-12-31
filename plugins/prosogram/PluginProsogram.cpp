#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "PluginProsogram.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/structure/AnnotationStructure.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "Prosogram.h"
#include "IntonationAnnotator.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Prosogram::PluginProsogramPrivateData {
    PluginProsogramPrivateData() :
        createLevels(true), overwrite(true), autosyllablePauseThreshold(0.250),
        timeRangeFrom(0.0), timeRangeTo(0.0),
        f0DetectionMin(60), f0DetectionMax(450), framePeriod(0.015),
        keepIntermediateFiles(true), createImageFiles(false),
        plottingStyle(0), plottingIntervalPerStrip(2.0), plottingMultiStrip(true)
    {
        // Sensible defaults
        segmentationMethod = 3;
        levelPhone = "phone";
        levelSyllable = "syll";
        levelTonalSegments = "prosogram_tonal_segments";
        levelVUV = "prosogram_vuv";
        glissandoThreshold = 0;
        plottingTiersToShow = "phone, syll";
        plottingOutputDirectory = QDir::homePath() + "/<basename>_";
    }

    int command;
    bool createLevels;
    bool overwrite;
    double autosyllablePauseThreshold;
    double timeRangeFrom;
    double timeRangeTo;
    double f0DetectionMin;
    double f0DetectionMax;
    double framePeriod;
    int segmentationMethod; // enum
    QString levelPhone;
    QString levelSyllable;
    QString levelSegmentation;
    QString levelTonalSegments;
    QString levelVUV;
    int glissandoThreshold; // enum
    bool keepIntermediateFiles;
    QString attributePrefix;
    // Plotting
    bool createImageFiles;
    int plottingStyle; // enum
    double plottingIntervalPerStrip;
    QString plottingTiersToShow;
    bool plottingMultiStrip;
    int plottingFileFormat; // enum
    QString plottingOutputDirectory;
};

Praaline::Plugins::Prosogram::PluginProsogram::PluginProsogram(QObject* parent) : QObject(parent)
{
    d = new PluginProsogramPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Prosogram::PluginProsogram::~PluginProsogram()
{
    delete d;
}

bool Praaline::Plugins::Prosogram::PluginProsogram::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    // Register Prosogram layer with the visualiser sub-system.
    return true;
}

bool Praaline::Plugins::Prosogram::PluginProsogram::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Prosogram::PluginProsogram::finalize() {

}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginName() const {
    return "Prosogram";
}

QtilitiesCategory Praaline::Plugins::Prosogram::PluginProsogram::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Prosogram::PluginProsogram::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginPublisher() const {
    return "Piet Mertens";
}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginDescription() const {
    return tr("Prosogram - pitch stylisation and feature extraction.");
}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginCopyright() const {
    return QString(tr("Copyright") + " Piet Mertens");
}

QString Praaline::Plugins::Prosogram::PluginProsogram::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Prosogram::PluginProsogram::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;

    parameters << PluginParameter("command", "Function to apply", QVariant::Int, d->command, QStringList() <<
                                  "ProsoGram v. 2.9" <<
                                  "Create utterance segmentation from auto syllables" <<
                                  "Automatic Intonation Annotation");
    parameters << PluginParameter("createLevels", "Create annotation levels when they do not exist?", QVariant::Bool, d->createLevels);
    parameters << PluginParameter("overwrite", "Overwrite existing annotations/values?", QVariant::Bool, d->overwrite);
    parameters << PluginParameter("autosyllablePauseThreshold", "Autosyllable pause threshold (sec)", QVariant::Double, d->autosyllablePauseThreshold);

    parameters << PluginParameter("timeRangeFrom", "Time range (s) From", QVariant::Double, d->timeRangeFrom);
    parameters << PluginParameter("timeRangeTo", "Time range (s) To", QVariant::Double, d->timeRangeFrom);
    parameters << PluginParameter("f0DetectionMin", "F0 detection range (Hz) Min", QVariant::Double, d->f0DetectionMin);
    parameters << PluginParameter("f0DetectionMax", "F0 detection range (Hz) Max", QVariant::Double, d->f0DetectionMax);
    parameters << PluginParameter("framePeriod", "Frame period (s)", QVariant::Double, d->framePeriod);
    parameters << PluginParameter("segmentationMethod", "Segmentation Method", QVariant::Int, d->segmentationMethod, QStringList() <<
                                  "Automatic acoustic syllables" <<
                                  "Nuclei in vowels in phones annotation" <<
                                  "Nuclei in rhyme from phones and syllables annotation" <<
                                  "Nuclei in syllables from phones and syllables annotation" <<
                                  "Nuclei in syllables from syllables annotation and local peak" <<
                                  "Based on segmentation annotation level");
    parameters << PluginParameter("levelPhone",        "Annotation level: Phonemes", QVariant::String, d->levelPhone);
    parameters << PluginParameter("levelSyllable",     "Annotation level: Syllables", QVariant::String, d->levelSyllable);
    parameters << PluginParameter("levelSegmentation", "Annotation level: Segmentation", QVariant::String, d->levelSegmentation);
    parameters << PluginParameter("glissandoThreshold", "Glissando threshold", QVariant::Int, d->glissandoThreshold, QStringList() <<
                                  "G=0.16/T^2, DG=20, dmin=0.035" <<
                                  "G=0.24/T^2, DG=20, dmin=0.035" <<
                                  "G=0.32/T^2, DG=20, dmin=0.035" <<
                                  "G=0.32/T^2, DG=30, dmin=0.050" <<
                                  "G=0.24-0.32/T^2 (adaptive), DG=30, dmin=0.050" <<
                                  "G=0.16-0.32/T^2 (adaptive), DG=30, dmin=0.050");
    // Plotting
    parameters << PluginParameter("createImageFiles", "Create image files", QVariant::Bool, d->createImageFiles);
    parameters << PluginParameter("plottingStyle", "Plotting style", QVariant::Int, d->plottingStyle, QStringList() <<
                                  "Compact" << "Compact rich" <<
                                  "Wide" << "Wide rich" <<
                                  "Wide rich, with pitch targets" <<
                                  "Wide rich, with pitch range");
    parameters << PluginParameter("plottingIntervalPerStrip", "Plotting: time interval per strip (seconds)", QVariant::Double, d->plottingIntervalPerStrip);
    parameters << PluginParameter("plottingTiersToShow", "Plotting: tiers (level/attribute) to show in plots", QVariant::String, d->plottingTiersToShow);
    parameters << PluginParameter("plottingMultiStrip", "Plotting: multiple strips per page", QVariant::Bool, d->plottingMultiStrip);
    parameters << PluginParameter("plottingFileFormat", "Plotting: output file format", QVariant::Int, d->plottingFileFormat, QStringList() <<
                                  "EPS (Encapsulated Postscript)" <<
                                  "EMF (Windows Enhanced Metafile)" <<
                                  "EPS and EMF" <<
                                  "PDF");
    parameters << PluginParameter("plottingOutputDirectory", "Plotting: output directory", QVariant::String, d->plottingOutputDirectory);
    return parameters;
}

void Praaline::Plugins::Prosogram::PluginProsogram::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("command"))                     d->command = parameters.value("command").toInt();
    if (parameters.contains("createLevels"))                d->createLevels = parameters.value("createLevels").toBool();
    if (parameters.contains("overwrite"))                   d->overwrite = parameters.value("overwrite").toBool();
    if (parameters.contains("autosyllablePauseThreshold"))  d->autosyllablePauseThreshold = parameters.value("autosyllablePauseThreshold").toDouble();
    if (parameters.contains("timeRangeFrom"))               d->timeRangeFrom = parameters.value("timeRangeFrom").toDouble();
    if (parameters.contains("timeRangeTo"))                 d->timeRangeTo = parameters.value("timeRangeTo").toDouble();
    if (parameters.contains("f0DetectionMin"))              d->f0DetectionMin = parameters.value("f0DetectionMin").toDouble();
    if (parameters.contains("f0DetectionMax"))              d->f0DetectionMax = parameters.value("f0DetectionMax").toDouble();
    if (parameters.contains("framePeriod"))                 d->framePeriod = parameters.value("framePeriod").toDouble();
    if (parameters.contains("segmentationMethod"))          d->segmentationMethod = parameters.value("segmentationMethod").toInt();
    if (parameters.contains("levelPhone"))                  d->levelPhone = parameters.value("levelPhone").toString();
    if (parameters.contains("levelSyllable"))               d->levelSyllable = parameters.value("levelSyllable").toString();
    if (parameters.contains("levelSegmentation"))           d->levelSegmentation = parameters.value("levelSegmentation").toString();
    if (parameters.contains("glissandoThreshold"))          d->glissandoThreshold = parameters.value("glissandoThreshold").toInt();
    // if (parameters.contains("keepIntermediateFiles"))       d->keepIntermediateFiles = parameters.value("keepIntermediateFiles").toBool();
    // Plotting
    if (parameters.contains("createImageFiles"))            d->createImageFiles = parameters.value("createImageFiles").toBool();
    if (parameters.contains("plottingStyle"))               d->plottingStyle = parameters.value("plottingStyle").toInt();
    if (parameters.contains("plottingIntervalPerStrip"))    d->plottingIntervalPerStrip = parameters.value("plottingIntervalPerStrip").toDouble();
    if (parameters.contains("plottingTiersToShow"))         d->plottingTiersToShow = parameters.value("plottingTiersToShow").toString();
    if (parameters.contains("plottingMultiStrip"))          d->plottingMultiStrip = parameters.value("plottingMultiStrip").toBool();
    if (parameters.contains("plottingFileFormat"))          d->plottingFileFormat = parameters.value("plottingFileFormat").toInt();
    if (parameters.contains("plottingOutputDirectory"))     d->plottingOutputDirectory = parameters.value("plottingOutputDirectory").toString();
}

void Praaline::Plugins::Prosogram::PluginProsogram::scriptSentMessage(const QString &message)
{
    printMessage(message);
}

void Praaline::Plugins::Prosogram::PluginProsogram::scriptFinished(int exitcode)
{
    if (exitcode == 0)
        printMessage("Finished succesfully.");
    else
        printMessage(QString("Finished with errors. The error code was %1").arg(exitcode));
}

void Praaline::Plugins::Prosogram::PluginProsogram::createProsogramDataStructures(CorpusRepository *repository)
{
    if (!repository) return;
    d->attributePrefix = d->attributePrefix.trimmed();
    // If need be, create syllables level
    AnnotationStructureLevel *level_syll = repository->annotationStructure()->level(d->levelSyllable);
    if (!level_syll) {
        level_syll = new AnnotationStructureLevel(d->levelSyllable, AnnotationStructureLevel::IndependentIntervalsLevel, "Syllables", "");
        if (!repository->annotations()->createAnnotationLevel(level_syll)) return;
        repository->annotationStructure()->addLevel(level_syll);
    }
    // Create syllable attributes where necessary
    // ...identification
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "nucl_t1", "Nucleus t1", "Syllabic nucleus start", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "nucl_t2", "Nucleus t2", "Syllabic nucleus end", DataType::Double);
    // ...before stylisation
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "f0_min", "f0 minimum", "f0 min (Hz) within nucleus before stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "f0_max", "f0 maximum", "f0 max (Hz) within nucleus before stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "f0_mean", "f0 mean", "f0 mean (ST) within nucleus before stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "f0_median", "f0 median", "f0 median (Hz) within nucleus before stylization", DataType::Double);
    // ...after stylisation
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "f0_start", "f0 start", "f0 value (Hz) at start of nucleus after stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "f0_end", "f0 end", "f0 value (Hz) at end of nucleus after stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "lopitch", "Pitch low", "f0 min (Hz) within nucleus after stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "hipitch", "Pitch high", "f0 max (Hz) within nucleus after stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "intersyllab", "Intersyllabic mvt", "Intersyllabic interval (ST) between end of previous nucleus and start of current one", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "intrasyllabdown", "Intrasyllabic mvt down", "Sum of downward pitch interval (ST) of tonal segments in nucleus", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "intrasyllabup", "Intrasyllabic mvt up", "Sum of upward pitch interval (ST) of tonal segments in nucleus", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "trajectory", "Trajectory", "Sum of absolute pitch interval (ST) of tonal segments in nucleus (rises and falls add up)", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "dynamic", "Dynamic mvt", "Dynamic movement: 0 = static, 1 = rising, -1 = falling", DataType::Integer);
    // ...intensity
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "int_peak", "Intensity peak", "Peak intensity in nucleus (dB)", DataType::Double);
    // ...durations
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "syll_dur", "Syllable duration", "Syllable duration (msec)", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "nucl_dur", "Nucleus duration", "Syllabic nucleus duration (msec)", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "vowel_dur", "Vowel duration", "Vowel duration (msec)", DataType::Double);
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "rime_dur", "Rime duration", "Rime duration (msec)", DataType::Double);
    // Tonal annotation
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "tonal_annotation", "Tonal annotation", "Tonal annotation: pitch label + movement (Polytonia model)", DataType(DataType::VarChar, 16));
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "tonal_label", "Tonal label", "Tonal annotation label (Polytonia model)", DataType(DataType::VarChar, 16));
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "tonal_movement", "Tonal movement", "Tonal annotation movement label (Polytonia model)", DataType(DataType::VarChar, 16));
    repository->createAnnotationAttribute(d->levelSyllable, d->attributePrefix + "tonal_label_method", "Tonal annotation method", "Tonal annotation method (Polytonia model)", DataType(DataType::VarChar, 16));

    // Create level for tonal segments
    AnnotationStructureLevel *level_tonal_segments = repository->annotationStructure()->level(d->levelTonalSegments);
    if (!level_tonal_segments) {
        level_tonal_segments = new AnnotationStructureLevel(d->levelTonalSegments, AnnotationStructureLevel::IndependentIntervalsLevel, "Prosogram Tonal Segments", "");
        if (!repository->annotations()->createAnnotationLevel(level_tonal_segments)) return;
        repository->annotationStructure()->addLevel(level_tonal_segments);
    }
    repository->createAnnotationAttribute(d->levelTonalSegments, "f0_start", "f0 start", "f0 (Hz) at start of tonal segment after stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelTonalSegments, "f0_end", "f0 end", "f0 (Hz) at end of tonal segment after stylization", DataType::Double);
    repository->createAnnotationAttribute(d->levelTonalSegments, "tonal_movement", "Tonal movement", "Tonal annotation movement label", DataType(DataType::VarChar, 16));

    // Create level for voiced-unvoiced regions (intervals)
    AnnotationStructureLevel *level_vuv = repository->annotationStructure()->level(d->levelVUV);
    if (!level_vuv) {
        level_vuv = new AnnotationStructureLevel(d->levelVUV, AnnotationStructureLevel::IndependentIntervalsLevel, "Prosogram Voiced-Unvoiced Regions", "");
        if (!repository->annotations()->createAnnotationLevel(level_vuv)) return;
        repository->annotationStructure()->addLevel(level_vuv);
    }

    // Create speaker metadata attributes for pitch range
    repository->createMetadataAttribute(CorpusObject::Type_Speaker, "", "ProsogramPitchRangeCountSyll", "Prosogram Pitch Range Syllable Count", "Number of syllables on the basis of which the speaker's pitch range was estimated", DataType::Integer);
    repository->createMetadataAttribute(CorpusObject::Type_Speaker, "", "ProsogramPitchRangeTopHz", "Prosogram Pitch Range Top (Hz)", "Top of the speaker's pitch range (in Hz)", DataType::Double);
    repository->createMetadataAttribute(CorpusObject::Type_Speaker, "", "ProsogramPitchRangeMedianHz", "Prosogram Pitch Range Median (Hz)", "Median (key) of the speaker's pitch range (in Hz)", DataType::Double);
    repository->createMetadataAttribute(CorpusObject::Type_Speaker, "", "ProsogramPitchRangeBottomHz", "Prosogram Pitch Range Bottom (Hz)", "Bottom of the speaker's pitch range (in Hz)", DataType::Double);
}


void Praaline::Plugins::Prosogram::PluginProsogram::createSegmentsFromAutoSyllables(const QList<CorpusCommunication *> &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Creating utterance segmentation from automatically detected syllables");
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;

        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_autosyll = tiers->getIntervalTierByName(d->levelSyllable);
                if (!tier_autosyll) {
                    printMessage(QString("Communication %1 Annotation %2 Speaker %3 - Auto-syllable tier (%4) not found.")
                                 .arg(com->ID()).arg(annot->ID()).arg(speakerID).arg(d->levelSyllable));
                    continue;
                }
                IntervalTier *tier_segment = tiers->getIntervalTierByName(d->levelSegmentation);
                if (tier_segment) {
                    if (!d->overwrite) {
                        printMessage(QString("Communication %1 Annotation %2 Speaker %3 - Already has segmentation tier (%4).")
                                     .arg(com->ID()).arg(annot->ID()).arg(speakerID).arg(d->levelSegmentation));
                        continue;
                    } else {
                        delete tier_segment;
                    }
                }
                QList<Interval *> listLongPauses;
                while (listLongPauses.isEmpty() && d->autosyllablePauseThreshold > 0.050) {
                    foreach (Interval *syll, tier_autosyll->intervals()) {
                        if ((syll->text() == "_") && (syll->duration().toDouble() > d->autosyllablePauseThreshold)) {
                            listLongPauses << new Interval(syll->tMin(), syll->tMax(), "<sil>");
                        }
                    }
                    if (listLongPauses.isEmpty())
                        d->autosyllablePauseThreshold = d->autosyllablePauseThreshold - 0.050;
                }
                if (listLongPauses.isEmpty()) {
                    tier_segment = new IntervalTier(d->levelSegmentation, tier_autosyll->tMin(), tier_autosyll->tMax());
                } else {
                    tier_segment = new IntervalTier(d->levelSegmentation, listLongPauses, tier_autosyll->tMin(), tier_autosyll->tMax());
                    tier_segment->replace("", "<sil>", "_");
                    tier_segment->mergeIdenticalAnnotations("_");
                }
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_segment);
            }
            printMessage(QString("OK %1 %2").arg(com->ID()).arg(annot->ID()));
        }
        qDeleteAll(tiersAll);
        countDone++;
        madeProgress(countDone * 100 / communications.count());
        QApplication::processEvents();
    }
    madeProgress(100);
    printMessage("Finished");
}

void Praaline::Plugins::Prosogram::PluginProsogram::runProsogram(const QList<CorpusCommunication *> &communications)
{
    ProsoGram *prosogram = new ProsoGram(this);
    connect(prosogram, SIGNAL(logOutput(QString)), this, SLOT(scriptSentMessage(QString)));
    connect(prosogram, SIGNAL(finished(int)), this, SLOT(scriptFinished(int)));

    prosogram->timeRangeFrom = d->timeRangeFrom;
    prosogram->timeRangeTo = d->timeRangeTo;
    prosogram->f0DetectionMin = d->f0DetectionMin;
    prosogram->f0DetectionMax = d->f0DetectionMax;
    prosogram->framePeriod = d->framePeriod;
    prosogram->segmentationMethod = d->segmentationMethod;
    prosogram->levelPhone = d->levelPhone;
    prosogram->levelSyllable = d->levelSyllable;
    prosogram->levelSegmentation = d->levelSegmentation;
    prosogram->levelTonalSegments = d->levelTonalSegments;
    prosogram->levelVUV = d->levelVUV;
    prosogram->glissandoThreshold = d->glissandoThreshold;
    prosogram->keepIntermediateFiles = d->keepIntermediateFiles;
    // Plotting
    prosogram->createImageFiles = d->createImageFiles;
    prosogram->plottingStyle = d->plottingStyle;
    prosogram->plottingIntervalPerStrip = d->plottingIntervalPerStrip;
    prosogram->plottingTiersToShow = d->plottingTiersToShow;
    prosogram->plottingMultiStrip = d->plottingMultiStrip;
    prosogram->plottingFileFormat = d->plottingFileFormat;
    prosogram->plottingOutputDirectory = d->plottingOutputDirectory;

    int countDone = 0;
    madeProgress(0);
    printMessage("ProsoGram v.2.9m running");

    foreach(CorpusCommunication *com, communications) {
        if (!com) continue;
        if (!com->repository()) continue;
        // Create segmentation level if it does not exist
        if (d->createLevels) {
            // Create segmentation level if it does not exist
            if ((!d->levelSegmentation.isEmpty()) && (!com->repository()->annotationStructure()->hasLevel(d->levelSegmentation))) {
                AnnotationStructureLevel *level_segment = new AnnotationStructureLevel(
                            d->levelSegmentation, AnnotationStructureLevel::IndependentIntervalsLevel, "Segmentation", "");
                if (!com->repository()->annotations()->createAnnotationLevel(level_segment)) {
                    printMessage(QString("Error creating segmentation level: %1").arg(d->levelSegmentation));
                    return;
                }
                com->repository()->annotationStructure()->addLevel(level_segment);
            }
        }

        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (CorpusRecording *rec, com->recordings()) {
            if (!rec) continue;

            // The Automatic Segmentation from Syllables may need to create an Annotation if there is none.
            if ((d->segmentationMethod == 0) && (com->annotations().isEmpty())) {
                com->addAnnotation(new CorpusAnnotation(com->ID(), com->repository()));
            }

            foreach (CorpusAnnotation *annot, com->annotations()) {
                if (!annot) continue;
                // TODO : ANNOTATION CORRESPONDANCE !!!
                // if (annot->ID() != rec->ID()) continue;

                if (d->segmentationMethod == 0) {
                    // Segmentation Method: Automatic syllable detection
                    QString speakerID = annot->ID(); // default speaker ID - impossible to tell using automatic syllabification
                    // Add speaker to corpus, if it does not already exist. Add participation, if it does not already exist.
                    if (com->corpus()) {
                        if (!com->corpus()->hasSpeaker(speakerID))
                            com->corpus()->addSpeaker(new CorpusSpeaker(speakerID));
                        if (!com->corpus()->hasParticipation(com->ID(), speakerID))
                            com->corpus()->addParticipation(com->ID(), speakerID);
                    }
                    // Check if the syllable level exists already
                    AnnotationTier *autosyll = com->repository()->annotations()->getTier(annot->ID(), speakerID, d->levelSyllable);
                    if (autosyll) {
                        QFileInfo info(rec->filePath());
                        QString prosoPath = info.absoluteDir().absolutePath() + "/prosogram/";
                        QString filenameGlobalsheet = QString("%1_%2_globalsheet.txt").arg(rec->ID()).arg(speakerID);
                        ProsoGram::updateGlobal(com, prosoPath + filenameGlobalsheet);
                        printMessage("Autosyll already OK, updated globalsheet");
                        continue;
                    }
                    AnnotationTierGroup *tiers = new AnnotationTierGroup();
                    if (!tiers) continue;
                    // execute prosogram script
                    prosogram->runProsoGram(com->corpus(), rec, tiers, annot->ID(), speakerID);
                    delete tiers;
                } else {
                    // Other segmentation methods
                    SpeakerAnnotationTierGroupMap tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annot->ID());
                    foreach (QString speakerID, tiersAll.keys()) {
                        printMessage(QString("   speaker %1").arg(speakerID));
                        AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                        if (!tiers) continue;
                        // check speaker-specific recording
                        if (!rec->property("speakerID").toString().isEmpty()) {
                            if (rec->property("speakerID").toString() != speakerID) continue;
                        }

                        // execute prosogram script
                        prosogram->runProsoGram(com->corpus(), rec, tiers, annot->ID(), speakerID);
                    }
                    qDeleteAll(tiersAll);
                }
                QApplication::processEvents();
            }
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    disconnect(prosogram, SIGNAL(logOutput(QString)), this, SLOT(scriptSentMessage(QString)));
    disconnect(prosogram, SIGNAL(finished(int)), this, SLOT(scriptFinished(int)));
    delete prosogram;
    madeProgress(100);
    printMessage("ProsoGram finished.");
}

void Praaline::Plugins::Prosogram::PluginProsogram::runIntonationAnnotation(const QList<CorpusCommunication *> &communications)
{
    if (communications.isEmpty()) {
        printMessage("No Communications selected.");
        return;
    }
    int countDone = 0;
    madeProgress(0);
    printMessage("Automatic Annotation of Intonation running");
    IntonationAnnotator annotator;
    QPointer<Corpus> corpus = communications.first()->corpus();
    printMessage("Pitch range estimation...");
    annotator.estimatePitchRange(corpus);
    printMessage("Annotation...");
    foreach (CorpusCommunication *com, communications) {
        printMessage(QString("Annotating %1").arg(com->ID()));
        annotator.annotate(com);
        QApplication::processEvents();
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
    madeProgress(100);
    printMessage("Automatic Annotation of Intonation finished.");
}

void Praaline::Plugins::Prosogram::PluginProsogram::process(const QList<CorpusCommunication *> &communications)
{
    d->levelPhone = d->levelPhone.trimmed();
    d->levelSegmentation = d->levelSegmentation.trimmed();
    d->levelSyllable = d->levelSyllable.trimmed();

    // Create data structures if needed
    QList<QPointer<CorpusRepository> > repositoriesWithDataStructures;
    foreach(CorpusCommunication *com, communications) {
        if (!com) continue;
        if (!com->repository()) continue;
        if (d->createLevels && !repositoriesWithDataStructures.contains(com->repository())) {
            // Create and/or add attributes to syllable level
            createProsogramDataStructures(com->repository());
            // This repository has been processed
            repositoriesWithDataStructures << com->repository();
        }
    }

    if (d->command == 0) {
        runProsogram(communications);
    }
    else if (d->command == 1) {
        createSegmentsFromAutoSyllables(communications);
    }
    else if (d->command == 2) {
        runIntonationAnnotation(communications);
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Prosogram;
    Q_EXPORT_PLUGIN2(PluginProsogram, PluginProsogram)
#endif
