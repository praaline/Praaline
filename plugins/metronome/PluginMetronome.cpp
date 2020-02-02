#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>
#include <QStringList>

#include "PluginMetronome.h"
#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusBookmark.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Serialisers/XML/XMLSerialiserCorpusBookmark.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Metronome::PluginMetronomePrivateData {
    PluginMetronomePrivateData() :
        createMetronomeAnnotationStructure(true), commandAnnotateRhythmicPatterns(true),
        levelPhone("phone"), levelSyllable("syll"), levelRhythm("rhythm"),
        attributeProminence("promise"), valuelistProminence("p P"),
        isochronyConsecutiveCount(3), isochronyConsecutiveThreshold(20.0)
    {}

    bool createMetronomeAnnotationStructure;
    bool commandAnnotateRhythmicPatterns;
    QString levelPhone;
    QString levelSyllable;
    QString levelRhythm;
    QString attributeProminence;
    QString valuelistProminence;
    int isochronyConsecutiveCount;
    double isochronyConsecutiveThreshold;
    QString filenameBookmarks;
};

Praaline::Plugins::Metronome::PluginMetronome::PluginMetronome(QObject* parent) : QObject(parent)
{
    d = new PluginMetronomePrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Metronome::PluginMetronome::~PluginMetronome()
{
    delete d;
}

bool Praaline::Plugins::Metronome::PluginMetronome::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)
    return true;
}

bool Praaline::Plugins::Metronome::PluginMetronome::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    d->filenameBookmarks = QDir::homePath() + "/praaline_metronome_bookmarks.xml";
    return true;
}

void Praaline::Plugins::Metronome::PluginMetronome::finalize() {

}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginName() const {
    return "Metronome: Rhythmic Patterns Analyser";
}

QtilitiesCategory Praaline::Plugins::Metronome::PluginMetronome::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Metronome::PluginMetronome::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginDescription() const {
    return tr("Metronome: A plugin to detect and annotate rhythmic patterns based on prosodic prominence.");
}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginCopyright() const {
    return QString(tr("Copyright") + " 2017, George Christodoulides");
}

QString Praaline::Plugins::Metronome::PluginMetronome::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Metronome::PluginMetronome::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("commandAnnotateRhythmicPatterns", "Detect and Annotate Rhythmic Patterns", QVariant::Bool, d->commandAnnotateRhythmicPatterns);
    parameters << PluginParameter("createMetronomeAnnotationStructure", "Create annotation levels/attributes if necessary", QVariant::Bool, d->createMetronomeAnnotationStructure);
    parameters << PluginParameter("levelPhone", "Level for phones", QVariant::String, d->levelPhone);
    parameters << PluginParameter("levelSyllable", "Level for syllables", QVariant::String, d->levelSyllable);
    parameters << PluginParameter("levelRhythm", "Level for rhythmic pattern annotation", QVariant::String, d->levelRhythm);
    parameters << PluginParameter("attributeProminence", "Syllable attribute indicating prominence", QVariant::String, d->attributeProminence);
    parameters << PluginParameter("valuelistProminence", "Values indicating prominence (space-separated)", QVariant::String, d->valuelistProminence);
    parameters << PluginParameter("isochronyConsecutiveCount", "Isochrony: minimum consecutive intervals", QVariant::Int, d->isochronyConsecutiveCount);
    parameters << PluginParameter("isochronyConsecutiveThreshold", "Isochrony: just noticeable difference (%)", QVariant::Double, d->isochronyConsecutiveThreshold);
    parameters << PluginParameter("filenameBookmarks", "Save bookmarks in file", QVariant::String, d->filenameBookmarks);
    return parameters;
}

void Praaline::Plugins::Metronome::PluginMetronome::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("commandAnnotateRhythmicPatterns")) d->commandAnnotateRhythmicPatterns = parameters.value("commandAnnotateRhythmicPatterns").toBool();
    if (parameters.contains("createMetronomeAnnotationStructure")) d->createMetronomeAnnotationStructure = parameters.value("createMetronomeAnnotationStructure").toBool();
    if (parameters.contains("levelPhone")) d->levelPhone = parameters.value("levelPhone").toString();
    if (parameters.contains("levelSyllable")) d->levelSyllable = parameters.value("levelSyllable").toString();
    if (parameters.contains("levelRhythm")) d->levelRhythm = parameters.value("levelRhythm").toString();
    if (parameters.contains("attributeProminence")) d->attributeProminence = parameters.value("attributeProminence").toString();
    if (parameters.contains("valuelistProminence")) d->valuelistProminence = parameters.value("valuelistProminence").toString();
    if (parameters.contains("isochronyConsecutiveCount")) d->isochronyConsecutiveCount = parameters.value("isochronyConsecutiveCount").toInt();
    if (parameters.contains("isochronyConsecutiveThreshold")) d->isochronyConsecutiveThreshold = parameters.value("isochronyConsecutiveThreshold").toDouble();
    if (parameters.contains("filenameBookmarks")) d->filenameBookmarks = parameters.value("filenameBookmarks").toString();
}

void createAttribute(CorpusRepository *repository, AnnotationStructureLevel *level, const QString &prefix,
                     const QString &ID, const QString &name = QString(), const QString &description = QString(),
                     const DataType &datatype = DataType(DataType::VarChar, 255), int order = 0,
                     bool indexed = false, const QString &nameValueList = QString())
{
    if (level->hasAttribute(ID)) return;
    AnnotationStructureAttribute *attr = new AnnotationStructureAttribute(prefix + ID, name, description, datatype,
                                                                          order, indexed, nameValueList);
    if (repository->annotations()->createAnnotationAttribute(level->ID(), attr))
        level->addAttribute(attr);
}

void Praaline::Plugins::Metronome::PluginMetronome::createMetronomeAnnotationStructure(CorpusRepository *repository)
{
    if (!repository) return;
    // If needed, create rhythm
    AnnotationStructureLevel *level_rhythm = repository->annotationStructure()->level(d->levelRhythm);
    if (!level_rhythm) {
        level_rhythm = new AnnotationStructureLevel(d->levelRhythm, AnnotationStructureLevel::IndependentIntervalsLevel, "Rhythmic Patterns", "");
        if (!repository->annotations()->createAnnotationLevel(level_rhythm)) return;
        repository->annotationStructure()->addLevel(level_rhythm);
    }
    // Create rhythm-level attributes where necessary
    createAttribute(repository, level_rhythm, "", "delta_duration", "Delta duration (ms)", "", DataType(DataType::Double));
    createAttribute(repository, level_rhythm, "", "delta_duration_percent", "Delta duration %", "", DataType(DataType::Double));
}

void Praaline::Plugins::Metronome::PluginMetronome::annotateRhythmicPatterns(const QList<CorpusCommunication *> &communications)
{
    QList<QPointer<CorpusRepository> > repositoriesWithMetronomeAnnotationStructure;
    int countDone = 0;
    madeProgress(0);
    printMessage(QString("Metronome version 1.0: Detecting and Annotating Rhythmic Patterns"));

    // Prepare list of values that indicate that a syllable is prominent
    QStringList valuesProminent = d->valuelistProminence.split(" ");
    for (int i = 0; i < valuesProminent.count(); ++i)
        valuesProminent[i] = valuesProminent.at(i).trimmed();
    // Prepare list of vowels
    QStringList vowels;
    vowels << "i" << "e" << "E" << "a" << "A" << "o" << "O" << "y" << "2" << "9";
    vowels << "e~" << "a~" << "o~" << "9~" << "@";

    QList<CorpusBookmark *> listBookmarks;

    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        if (d->createMetronomeAnnotationStructure && !repositoriesWithMetronomeAnnotationStructure.contains(com->repository())) {
            createMetronomeAnnotationStructure(com->repository());
            repositoriesWithMetronomeAnnotationStructure << com->repository();
            emit printMessage("Created annotation levels and attributes to store rhythmic pattern annotations.");
        }
        // QString path = com->repository()->files()->basePath();
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                if (!tiers) continue;
                // Find necessary tiers: syllables and phones
                IntervalTier *tier_phone = tiers->getIntervalTierByName(d->levelPhone);
                if (!tier_phone) {
                    printMessage(QString("    Tier not found for phones (tier name: %1)").arg(d->levelPhone));
                    continue;
                }
                IntervalTier *tier_syll = tiers->getIntervalTierByName(d->levelSyllable);
                if (!tier_syll) {
                    printMessage(QString("    Tier not found for syllables (tier name: %1)").arg(d->levelSyllable));
                    continue;
                }
                // Find beats = the attack of a prominent syllable
                QList<RealTime> beats;
                foreach (Interval *syll, tier_syll->intervals()) {
                    if (valuesProminent.contains(syll->attribute(d->attributeProminence).toString().trimmed())) {
                        // This syllable is prominent. Find the beginning of its nucleus (vowel).
                        bool found = false;
                        foreach (Interval *phone, tier_phone->getIntervalsContainedIn(syll)) {
                            if (vowels.contains(phone->text().trimmed())) {
                                beats << phone->tMin();
                                found = true;
                            }
                        }
                        if (!found) {
                            // no vowel found but this syllable is annotated as being prominent...
                            beats << syll->tCenter();
                        }
                    }
                }
                // Create rhythm intervals = time intervals between beats
                if (beats.isEmpty()) {
                    printMessage("   No prominent syllables found - no beats: aborting.");
                    continue;
                }
                if (beats.first() != RealTime::zeroTime) beats.insert(0, RealTime(0, 0));
                if (beats.last() != tier_syll->tMax()) beats.append(tier_syll->tMax());
                QList<Interval *> intervals_rhythm;
                double previousDuration(0);
                for (int i = 0; i < beats.count() - 1; i++) {
                    double duration = (beats.at(i + 1) - beats.at(i)).toDouble();
                    double delta(0), percentage(0);
                    QString text = QString("%1").arg(duration * 1000.0, 0, 'f', 0);
                    if (previousDuration > 0) {
                        delta = duration - previousDuration;
                        percentage = delta / previousDuration * 100.0;
                        text = text.append(QString(" (%1%2%)").arg(percentage < 0 ? '-' : '+').arg(qFabs(percentage), 0, 'f', 1));
                    }
                    Interval *intv = new Interval(beats.at(i), beats.at(i + 1), text);
                    intv->setAttribute("delta_duration", delta);
                    intv->setAttribute("delta_duration_percent", percentage);
                    intervals_rhythm << intv;
                    previousDuration = duration;
                }
                QScopedPointer<IntervalTier> tier_rhythm(new IntervalTier(d->levelRhythm, intervals_rhythm));
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_rhythm.data());
                // Search for isochrony: n consecutive rhythm intervals having durations varying less than x%.
                int index(0), indexStart(0), countConsecutive(0), countIsochronies(0);
                while (index < tier_rhythm->count()) {
                    if (qAbs(tier_rhythm->at(index)->attribute("delta_duration_percent").toDouble()) <= d->isochronyConsecutiveThreshold) {
                        countConsecutive++;
                    }
                    else {
                        if (countConsecutive >= d->isochronyConsecutiveCount) {
                            Interval *start = tier_rhythm->at(indexStart);
                            if (start) {
                                listBookmarks << new CorpusBookmark(com->corpusID(), com->ID(), annotationID, start->tMin(),
                                                                    QString("Isochrony: %1 intervals").arg(countConsecutive));
                                countIsochronies++;
                            }
                        }
                        indexStart = index + 1;
                        countConsecutive = 0;
                    }
                    ++index;
                }
                printMessage(QString("   Found %1 regions of >=%2 intervals perceived as isochronous.")
                             .arg(countIsochronies).arg(d->isochronyConsecutiveCount));

            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }

    if (!d->filenameBookmarks.isEmpty()) {
        printMessage(QString("Saved rhythmic patterns in bookmarks file: %1").arg(d->filenameBookmarks));
        XMLSerialiserCorpusBookmark::saveCorpusBookmarks(listBookmarks, d->filenameBookmarks);
    }
    qDeleteAll(listBookmarks);
}

void Praaline::Plugins::Metronome::PluginMetronome::process(const QList<CorpusCommunication *> &communications)
{
    if (d->commandAnnotateRhythmicPatterns) {
        annotateRhythmicPatterns(communications);
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Metronome;
    Q_EXPORT_PLUGIN2(PluginMetronome, PluginMetronome)
#endif
