#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "PluginUDPipe.h"
#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusBookmark.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/FileDatastore.h"
#include "pncore/serialisers/xml/XMLSerialiserCorpusBookmark.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::UDPipe::PluginUDPipePrivateData {
    PluginUDPipePrivateData() :
        commandPatternsREP(false)
    {}

    bool commandPatternsREP;
};

Praaline::Plugins::UDPipe::PluginUDPipe::PluginUDPipe(QObject* parent) : QObject(parent)
{
    d = new PluginUDPipePrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::UDPipe::PluginUDPipe::~PluginUDPipe()
{
    delete d;
}

bool Praaline::Plugins::UDPipe::PluginUDPipe::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::UDPipe::PluginUDPipe::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::UDPipe::PluginUDPipe::finalize() {

}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginName() const {
    return "UDPipe";
}

QtilitiesCategory Praaline::Plugins::UDPipe::PluginUDPipe::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::UDPipe::PluginUDPipe::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginDescription() const {
    return tr("UDPipe: A bridge to the UDPipe universal dependencies analyser.");
}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginCopyright() const {
    return QString(tr("Copyright") + " 2018, George Christodoulides");
}

QString Praaline::Plugins::UDPipe::PluginUDPipe::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::UDPipe::PluginUDPipe::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("commandPatternsREP", "Patterns-based detection: Repetitions", QVariant::Bool, d->commandPatternsREP);
    return parameters;
}

void Praaline::Plugins::UDPipe::PluginUDPipe::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("commandPatternsREP")) d->commandPatternsREP = parameters.value("commandPatternsREP").toBool();
}


//void Praaline::Plugins::UDPipe::PluginUDPipe::patterns(
//        const QList<QPointer<CorpusCommunication> > &communications, const QStringList &codes)
//{
//    int countDone = 0;
//    madeProgress(0);
//    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: %1 patterns").arg(codes.join(", ")));
//    QList<QPointer<CorpusBookmark> > bookmarks;
//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        printMessage(QString("Annotating %1").arg(com->ID()));
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            QString annotationID = annot->ID();
//            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
//            foreach (QString speakerID, tiersAll.keys()) {
//                printMessage(QString("   speaker %1").arg(speakerID));
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
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

void Praaline::Plugins::UDPipe::PluginUDPipe::process(const QList<QPointer<CorpusCommunication> > &communications)
{
    QStringList patternsToAnnotate;
    if (d->commandPatternsREP)
    {
        patternsToAnnotate << "REP";
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::UDPipe;
    Q_EXPORT_PLUGIN2(PluginUDPipe, PluginUDPipe)
#endif
