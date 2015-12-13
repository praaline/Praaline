#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "pluginprosogram.h"
#include "pncore/corpus/corpus.h"
#include "prosogram.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Prosogram::PluginProsogramPrivateData {
    PluginProsogramPrivateData() :
        overwrite(true), autosyllablePauseThreshold(0.250),
        timeRangeFrom(0.0), timeRangeTo(0.0),
        f0DetectionMin(60), f0DetectionMax(450), framePeriod(0.015),
        keepIntermediateFiles(true), createImageFiles(false)
    {
        // Sensible defaults
        segmentationMethod = 3;
        levelPhone = "phone";
        levelSyllable = "syll";
        glissandoThreshold = 0;
    }

    int command;
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
    int glissandoThreshold; // enum
    bool keepIntermediateFiles;
    bool createImageFiles;
    QString tiersOnImageFiles;
    QString fileFormat;
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
                                  "Create utterance segmentation from auto syllables");
    parameters << PluginParameter("overwrite", "Overwrite?", QVariant::Bool, d->overwrite);
    parameters << PluginParameter("autosyllablePauseThreshold", "Autosyllable pause threshold (sec)", QVariant::Double, d->autosyllablePauseThreshold);

    parameters << PluginParameter("timeRangeFrom", "Time range (s) From", QVariant::Double, d->timeRangeFrom);
    parameters << PluginParameter("timeRangeTo", "Time range (s) To", QVariant::Double, d->timeRangeFrom);
    parameters << PluginParameter("f0DetectionMin", "F0 detection range (Hz) Min", QVariant::Double, d->f0DetectionMin);
    parameters << PluginParameter("f0DetectionMax", "F0 detection range (Hz) Max", QVariant::Double, d->f0DetectionMax);
    parameters << PluginParameter("framePeriod", "Frame period (s)", QVariant::Double, d->framePeriod);
    parameters << PluginParameter("segmentationMethod", "Segmentation Method", QVariant::Int, d->segmentationMethod, QStringList() <<
                                  "Automatic with acoustic syllables" <<
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
    parameters << PluginParameter("keepIntermediateFiles", "Keep intermediate files", QVariant::Bool, d->keepIntermediateFiles);
    parameters << PluginParameter("createImageFiles", "Create image files", QVariant::Bool, d->createImageFiles);
    parameters << PluginParameter("tiersOnImageFiles", "Tiers (level/attribute) to include in image files", QVariant::String, d->tiersOnImageFiles);
    parameters << PluginParameter("fileFormat", "File format for images", QVariant::String, d->fileFormat);
    return parameters;
}

void Praaline::Plugins::Prosogram::PluginProsogram::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("command"))                     d->command = parameters.value("command").toInt();
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
    if (parameters.contains("keepIntermediateFiles"))       d->keepIntermediateFiles = parameters.value("keepIntermediateFiles").toBool();
    if (parameters.contains("createImageFiles"))            d->createImageFiles = parameters.value("createImageFiles").toBool();
    if (parameters.contains("tiersOnImageFiles"))           d->tiersOnImageFiles = parameters.value("tiersOnImageFiles").toString();
    if (parameters.contains("fileFormat"))                  d->fileFormat = parameters.value("fileFormat").toString();
}

void Praaline::Plugins::Prosogram::PluginProsogram::scriptSentMessage(QString message)
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

void Praaline::Plugins::Prosogram::PluginProsogram::createSegmentsFromAutoSyllables(Corpus *corpus, QList<QPointer<CorpusCommunication> > &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage("Creating utterance segmentation from automatically detected syllables");
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;

        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                IntervalTier *tier_autosyll = tiers->getIntervalTierByName("auto_syll");
                if (!tier_autosyll) {
                    printMessage(QString("Communication %1 Annotation %2 Speaker %3 - Tier auto_syll not found.")
                                 .arg(com->ID()).arg(annot->ID()).arg(speakerID));
                    continue;
                }
                IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
                if (tier_segment) {
                    if (!d->overwrite) {
                        printMessage(QString("Communication %1 Annotation %2 Speaker %3 - Already has segment tier.")
                                     .arg(com->ID()).arg(annot->ID()).arg(speakerID));
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
                    tier_segment = new IntervalTier("segment", tier_autosyll->tMin(), tier_autosyll->tMax());
                } else {
                    tier_segment = new IntervalTier("segment", tier_autosyll->tMin(), tier_autosyll->tMax(), listLongPauses);
                    tier_segment->replaceText("<sil>", "_");
                    tier_segment->mergeIdenticalAnnotations("_");
                }
                corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_segment);
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

void Praaline::Plugins::Prosogram::PluginProsogram::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    if (!corpus) {
        emit printMessage("ProsoGram: no corpus selected!");
        return;
    }

    if (d->command == 1) {
        createSegmentsFromAutoSyllables(corpus, communications);
        return;
    }
    // otherwise, good old ProsoGram

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
    prosogram->glissandoThreshold = d->glissandoThreshold;
    prosogram->keepIntermediateFiles = d->keepIntermediateFiles;
    prosogram->createImageFiles = d->createImageFiles;
    prosogram->tiersOnImageFiles = d->tiersOnImageFiles;
    prosogram->fileFormat = d->fileFormat;

    int countDone = 0;
    madeProgress(0);
    printMessage("ProsoGram v.2.9m running");
    foreach(QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            if (!rec) continue;
            foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
                if (!annot) continue;
                if (d->segmentationMethod == 0) {
                    foreach (QPointer<CorpusParticipation> part, corpus->participationsForCommunication(com->ID())) {
                        if (!part) continue;
                        QString speakerID = part->speakerID();
                        printMessage(QString("   speaker %1").arg(speakerID));
                        // check already
                        AnnotationTier *autosyll = corpus->datastoreAnnotations()->getTier(annot->ID(), speakerID, d->levelSyllable);
                        if (autosyll) {
                            QFileInfo info(corpus->baseMediaPath() + "/" + rec->filename());
                            QString prosoPath = info.absoluteDir().absolutePath() + "/prosogram/";
                            QString filenameGlobalsheet = QString("%1_%2_globalsheet.txt").arg(rec->ID()).arg(speakerID);
                            ProsoGram::updateGlobal(com, prosoPath + filenameGlobalsheet);
                            printMessage("Autosyll already OK, updated globalsheet");
                            continue;
                        }
                        QPointer<AnnotationTierGroup> tiers = new AnnotationTierGroup();
                        if (!tiers) continue;
                        // execute prosogram script
                        prosogram->runProsoGram(corpus, rec, tiers, annot->ID(), speakerID);
                        delete tiers;
                    }
                } else {
                    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
                    foreach (QString speakerID, tiersAll.keys()) {
                        printMessage(QString("   speaker %1").arg(speakerID));
                        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                        if (!tiers) continue;
                        // execute prosogram script
                        prosogram->runProsoGram(corpus, rec, tiers, annot->ID(), speakerID);
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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Prosogram;
    Q_EXPORT_PLUGIN2(PluginProsogram, PluginProsogram)
#endif
