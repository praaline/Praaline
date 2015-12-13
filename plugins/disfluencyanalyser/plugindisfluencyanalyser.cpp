#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "plugindisfluencyanalyser.h"
#include "pncore/corpus/corpus.h"
#include "pncore/corpus/corpusbookmark.h"
#include "pncore/serialisers/xml/xmlserialisercorpusbookmark.h"

#include "disfluency.h"
#include "disfluencyanalysertool.h"
#include "disfluencypatterndetector.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyserPrivateData {
    PluginDisfluencyAnalyserPrivateData() {}
    QString command;
};

Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::PluginDisfluencyAnalyser(QObject* parent) : QObject(parent)
{
    d = new PluginDisfluencyAnalyserPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::~PluginDisfluencyAnalyser()
{
    delete d;
}

bool Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::finalize() {

}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginName() const {
    return "Disfluency Analyser for DisMo";
}

QtilitiesCategory Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginDescription() const {
    return tr("Disfluency Analyser: analyses disfluency tag sequences based on the DisMo annotation protocol.");
}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    parameters << PluginParameter("command", "Command", QVariant::String, d->command);
    return parameters;
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::setParameters(QHash<QString, QVariant> parameters)
{
    if (parameters.contains("command")) d->command = parameters.value("command").toString();
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::concordances(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    int countDone = 0;
    madeProgress(0);
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

                QString result;
                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;

                DisfluencyAnalyserTool *DA = new DisfluencyAnalyserTool(tier_tok_min, this);
                DA->readFromTier(tier_tok_min, "disfluency");
                QPointer<CorpusSpeaker> spk = corpus->speaker(speakerID);
                foreach (Disfluency *disf, DA->disfluencies()) {
                    result.append(annotationID).append("\t");
                    result.append(speakerID).append("\t");
                    if (spk) result.append(QString::number(spk->property("age").toInt())).append("\t");
                    else result.append("0\t");
                    result.append(QString::number(disf->indexStart())).append("\t");
                    result.append(QString::number(disf->indexInterruptionPoint())).append("\t");
                    result.append(QString::number(disf->indexEnd())).append("\t");
                    result.append(disf->globalTag()).append("\t");
                    result.append(disf->contextText(-5)).append("\t");
                    result.append(disf->formatted()).append("\t");
                    result.append(disf->contextText(5)).append("\n");
                }
                delete DA;
                if (result.length() > 1) result.chop(1);
                printMessage(result);
                result.clear();

            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::patterns(
        Corpus *corpus, QList<QPointer<CorpusCommunication> > communications, const QStringList &codes)
{
    int countDone = 0;
    madeProgress(0);
    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: %1 patterns").arg(codes.join(", ")));
    QList<QPointer<CorpusBookmark> > bookmarks;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;

                DisfluencyPatternDetector *PD = new DisfluencyPatternDetector();
                PD->setTiers(tiers);
                if (codes.contains("REP")) {
                    QList<DisfluencyPatternDetector::RepetitionInfo> repetitions = PD->detectRepetitionPatterns();
                    PD->codeRepetitions(repetitions);
                }
                if (codes.contains("INS")) {
                    QList<DisfluencyPatternDetector::InsertionInfo> insertions = PD->detectInsertionPatterns();
                    PD->codeInsertions(insertions);
                    bookmarks << PD->createBookmarks(corpus->ID(), com->ID(), annotationID, insertions);
                }
                if (codes.contains("SUB")) {
                    QList<DisfluencyPatternDetector::SubstitutionInfo> substitutions = PD->detectSubstitutionPatterns();
                    PD->codeSubstitutions(substitutions);
                    bookmarks << PD->createBookmarks(corpus->ID(), com->ID(), annotationID, substitutions);
                }
                delete PD;

            }
            corpus->datastoreAnnotations()->saveTiersAllSpeakers(annotationID, tiersAll);
            qDeleteAll(tiersAll);
            XMLSerialiserCorpusBookmark::saveCorpusBookmarks(bookmarks, corpus->basePath() + "/autodisfluencies_bookmarks.xml");
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    if (d->command.contains("patterns")) patterns(corpus, communications, QStringList( { "INS", "SUB"} ));
    if (d->command.contains("concordance")) concordances(corpus, communications);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::DisfluencyAnalyser;
    Q_EXPORT_PLUGIN2(PluginDisfluencyAnalyser, PluginDisfluencyAnalyser)
#endif
