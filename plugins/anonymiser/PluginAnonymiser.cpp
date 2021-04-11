#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "PluginAnonymiser.h"
#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusBookmark.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Serialisers/XML/XMLSerialiserCorpusBookmark.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Anonymiser::PluginAnonymiserPrivateData {
    PluginAnonymiserPrivateData() :
        commandPatternsREP(false)
    {}

    bool commandPatternsREP;
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
    return tr("Anonymiser: An intonation-preserving anonymiser for sound files based on a script by Daniel Hirst.");
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
    parameters << PluginParameter("commandPatternsREP", "Patterns-based detection: Repetitions", QVariant::Bool, d->commandPatternsREP);
    return parameters;
}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("commandPatternsREP")) d->commandPatternsREP = parameters.value("commandPatternsREP").toBool();
}


//void Praaline::Plugins::Anonymiser::PluginAnonymiser::patterns(
//        const QList<CorpusCommunication *> &communications, const QStringList &codes)
//{
//    int countDone = 0;
//    madeProgress(0);
//    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: %1 patterns").arg(codes.join(", ")));
//    QList<CorpusBookmark *> bookmarks;
//    SpeakerAnnotationTierGroupMap tiersAll;
//    foreach (CorpusCommunication *com, communications) {
//        if (!com) continue;
//        printMessage(QString("Annotating %1").arg(com->ID()));
//        foreach (CorpusAnnotation *annot, com->annotations()) {
//            if (!annot) continue;
//            QString annotationID = annot->ID();
//            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
//            foreach (QString speakerID, tiersAll.keys()) {
//                printMessage(QString("   speaker %1").arg(speakerID));
//                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;

//                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
//                if (!tier_tok_min) continue;

//                DisfluencyPatternDetector *PD = new DisfluencyPatternDetector();
//                PD->setTiers(tiers);
//                if (codes.contains("REP")) {
//                    PD->revertToDisfluenciesLevel1();
//                    QList<DisfluencyPatternDetector::RepetitionInfo> repetitions = PD->detectRepetitionPatterns();
//                }
//                if (codes.contains("INS")) {
//                    QList<DisfluencyPatternDetector::InsertionInfo> insertions = PD->detectInsertionPatterns();
//                    PD->codeInsertions(insertions);
//                    bookmarks << PD->createBookmarks(com->corpusID(), com->ID(), annotationID, insertions);
//                }
//                if (codes.contains("SUB")) {
//                    QList<DisfluencyPatternDetector::SubstitutionInfo> substitutions = PD->detectSubstitutionPatterns();
//                    PD->codeSubstitutions(substitutions);
//                    bookmarks << PD->createBookmarks(com->corpusID(), com->ID(), annotationID, substitutions);
//                }
//                delete PD;
//            }
//            com->repository()->annotations()->saveTiersAllSpeakers(annotationID, tiersAll);
//            qDeleteAll(tiersAll);
//            XMLSerialiserCorpusBookmark::saveCorpusBookmarks(bookmarks, com->repository()->files()->basePath() + "/autodisfluencies_bookmarks.xml");
//        }
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
//    }
//}

void Praaline::Plugins::Anonymiser::PluginAnonymiser::process(const QList<CorpusCommunication *> &communications)
{
    Q_UNUSED(communications)
    QStringList patternsToAnnotate;
    if (d->commandPatternsREP)
    {
        patternsToAnnotate << "REP";
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Anonymiser;
    Q_EXPORT_PLUGIN2(PluginAnonymiser, PluginAnonymiser)
#endif
