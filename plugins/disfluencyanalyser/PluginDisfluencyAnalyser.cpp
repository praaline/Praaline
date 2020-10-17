#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include "PluginDisfluencyAnalyser.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusBookmark.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Datastore/FileDatastore.h"
#include "PraalineCore/Serialisers/XML/XMLSerialiserCorpusBookmark.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "Disfluency.h"
#include "DisfluencyAnalyserTool.h"
#include "DisfluencyPatternDetector.h"

using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyserPrivateData {
    PluginDisfluencyAnalyserPrivateData() :
        commandPatternsREP(false), commandPatternsINS(false), commandPatternsSUB(false),
        commandExportMultiTierTextgrids(false), commandConcordances(false)
    {}

    bool commandPatternsREP;
    bool commandPatternsINS;
    bool commandPatternsSUB;
    bool commandExportMultiTierTextgrids;
    bool commandConcordances;
    bool commandCreateSequences;
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
    parameters << PluginParameter("commandPatternsREP", "Patterns-based detection: Repetitions", QVariant::Bool, d->commandPatternsREP);
    parameters << PluginParameter("commandPatternsINS", "Patterns-based detection: Insertions", QVariant::Bool, d->commandPatternsINS);
    parameters << PluginParameter("commandPatternsSUB", "Patterns-based detection: Substitutions", QVariant::Bool, d->commandPatternsSUB);
    parameters << PluginParameter("commandExportMultiTierTextgrids", "Export multi-tier textgrids", QVariant::Bool, d->commandExportMultiTierTextgrids);
    parameters << PluginParameter("commandConcordances", "Create Concordances", QVariant::Bool, d->commandConcordances);
    parameters << PluginParameter("commandCreateSequences", "Create Sequences in database", QVariant::Bool, d->commandCreateSequences);
    return parameters;
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::setParameters(const QHash<QString, QVariant> &parameters)
{
    if (parameters.contains("commandPatternsREP")) d->commandPatternsREP = parameters.value("commandPatternsREP").toBool();
    if (parameters.contains("commandPatternsINS")) d->commandPatternsINS = parameters.value("commandPatternsINS").toBool();
    if (parameters.contains("commandPatternsSUB")) d->commandPatternsSUB = parameters.value("commandPatternsSUB").toBool();
    if (parameters.contains("commandExportMultiTierTextgrids")) d->commandExportMultiTierTextgrids = parameters.value("commandExportMultiTierTextgrids").toBool();
    if (parameters.contains("commandConcordances")) d->commandConcordances = parameters.value("commandConcordances").toBool();
    if (parameters.contains("commandCreateSequences")) d->commandCreateSequences = parameters.value("commandCreateSequences").toBool();
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::concordances(const QList<CorpusCommunication *> &communications)
{
    bool withSyllData = true;
    int countDone = 0;
    madeProgress(0);
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

                QString result;
                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;

                IntervalTier *tier_syll = 0;
                if (withSyllData) tier_syll = tiers->getIntervalTierByName("syll");

                DisfluencyAnalyserTool *DA = new DisfluencyAnalyserTool(tier_tok_min, this);
                DA->readFromTier(tier_tok_min, "disfluency");
                // QPointer<CorpusSpeaker> spk = com->corpus()->speaker(speakerID);
                foreach (Disfluency *disf, DA->disfluencies()) {

                    // Only repetitions
                    if (disf->globalTag() != "REP") continue;

                    // CALCULATIONS
                    // structure
                    QList<Interval *> reparandum = disf->reparandumIntervals();
                    QList<Interval *> interregnum = disf->interregnumIntervals();
                    QList<Interval *> reparans = disf->reparansIntervals();
                    RealTime startReparandum, endReparandum, startInterregnum, endInterregnum, startReparans, endReparans;
                    // double meanPitchReparandum(0.0), meanPitchReparans(0.0);

                    if (!reparandum.isEmpty()) {
                        startReparandum = reparandum.first()->tMin();
                        endReparandum = reparandum.last()->tMax();

                        QList<Interval *> sylls = tier_syll->getIntervalsContainedIn(startReparandum, endReparandum);
                        // int countSyll(0); // double sum(0.0);
                        foreach (Interval *syll, sylls) {
                            if (syll->attribute("f0_min").toInt() == 0) continue;
                            //sum += syll->attribute("f0_mean");
                            // countSyll++;
                        }
                        // if (countSyll == 0) meanPitchReparandum = -1.0; // not stylized

                    }
                    if (!interregnum.isEmpty()) {
                        startInterregnum = interregnum.first()->tMin();
                        endInterregnum = interregnum.last()->tMax();
                    }
                    if (!reparans.isEmpty()) {
                        startReparans = reparans.first()->tMin();
                        endReparans = reparans.last()->tMax();
                    }

                    int numberOfTokensInReparandum(0), numberOfTokensInReparans(0), numberOfRepetitions(0);
                    int numberOf_SIL_InReparandum(0),   numberOf_FIL_InReparandum(0),
                        numberOf_SIL_InInterregnum(0),  numberOf_FIL_InInterregnum(0),
                        numberOf_SIL_InReparans(0),     numberOf_FIL_InReparans(0);

                    foreach (Interval *intv, reparandum) {
                        QString dis = intv->attribute("disfluency").toString();
                        if      (dis.contains("SIL")) numberOf_SIL_InReparandum++;
                        else if (dis.contains("FIL")) numberOf_FIL_InReparandum++;
                        else    numberOfTokensInReparandum++;
                    }
                    foreach (Interval *intv, interregnum) {
                        QString dis = intv->attribute("disfluency").toString();
                        if      (dis.contains("SIL")) numberOf_SIL_InInterregnum++;
                        else if (dis.contains("FIL")) numberOf_FIL_InInterregnum++;
                    }
                    foreach (Interval *intv, reparans) {
                        QString dis = intv->attribute("disfluency").toString();
                        if      (dis.contains("SIL")) numberOf_SIL_InReparans++;
                        else if (dis.contains("FIL")) numberOf_FIL_InReparans++;
                        else    numberOfTokensInReparans++;
                    }
                    if (numberOfTokensInReparans > 0)
                        numberOfRepetitions = numberOfTokensInReparandum / numberOfTokensInReparans;

                    // OUTPUT: Export results
                    result.append(annotationID).append("\t");
                    result.append(speakerID).append("\t");
                    // The entire sequence: from the tMin of the reparandum to the tMax of the reparans
                    result.append(QString::number(disf->timeReparandumStart().toDouble())).append("\t");
                    result.append(QString::number(disf->timeReparansEnd().toDouble())).append("\t");
                    // result.append(disf->globalTag()).append("\t");
                    // interval indices
                    result.append(QString::number(disf->indexStart())).append("\t");
                    result.append(QString::number(disf->indexInterruptionPoint())).append("\t");
                    result.append(QString::number(disf->indexEnd())).append("\t");

                    result.append(QString::number(startReparandum.toDouble())).append("\t");
                    result.append(QString::number(endReparandum.toDouble())).append("\t");
                    result.append(QString::number(startInterregnum.toDouble())).append("\t");
                    result.append(QString::number(endInterregnum.toDouble())).append("\t");
                    result.append(QString::number(startReparans.toDouble())).append("\t");
                    result.append(QString::number(endReparans.toDouble())).append("\t");

                    result.append(QString::number(numberOfTokensInReparandum)).append("\t");
                    result.append(QString::number(numberOf_SIL_InReparandum)).append("\t");
                    result.append(QString::number(numberOf_FIL_InReparandum)).append("\t");

                    result.append(QString::number(numberOf_SIL_InInterregnum)).append("\t");
                    result.append(QString::number(numberOf_FIL_InInterregnum)).append("\t");

                    result.append(QString::number(numberOfTokensInReparans)).append("\t");
                    result.append(QString::number(numberOf_SIL_InReparans)).append("\t");
                    result.append(QString::number(numberOf_FIL_InReparans)).append("\t");

                    result.append(QString::number(numberOfRepetitions)).append("\t");

                    QString contextLeft = disf->contextText(-10).trimmed();
                    QString contextRight = disf->contextText(10).trimmed();


                    result.append(contextLeft).append("\t");
                    result.append(contextLeft.right(1) == "_" ? "SIL" : "").append("\t");
                    result.append(disf->reparandumText()).append("\t");
                    result.append(disf->interregnumText()).append("\t");
                    result.append(disf->interregnumAttribute("pos_min", " ")).append("\t");
                    result.append(disf->interregnumAttribute("disfluency", " ")).append("\t");
                    result.append(disf->reparansText().left(1) == "_" ? "SIL" : "").append("\t");
                    result.append(disf->reparansText()).append("\t");
                    result.append(contextRight).append("\t");

                    result.append(disf->reparandumAttribute("disfluency_man", " ")).append("\t");
                    result.append(disf->interregnumAttribute("disfluency_man", " ")).append("\t");
                    result.append(disf->reparansAttribute("disfluency_man", " ")).append("\n");
                    // result.append(disf->formatted()).append("\t");
                }
                delete DA;
                if (result.length() > 1) result.chop(1);
                if (!result.isEmpty()) printMessage(result);
                result.clear();

            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::createSequences
    (const QList<CorpusCommunication *> &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: Creating sequences in database"));

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

                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;

                DisfluencyAnalyserTool *DA = new DisfluencyAnalyserTool(tier_tok_min, this);
                DA->readFromTier(tier_tok_min, "disfluency");
                // foreach (Disfluency *disf, DA->disfluencies()) {
                    // THIS IS WHERE WE WOULD CREATE A SEQUENCE RECORD IN THE DATABASE

                // }
                delete DA;

            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::patterns(
        const QList<CorpusCommunication *> &communications, const QStringList &codes)
{
    int countDone = 0;
    madeProgress(0);
    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: %1 patterns").arg(codes.join(", ")));
    QList<CorpusBookmark *> bookmarks;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        printMessage(QString("Annotating %1").arg(com->ID()));
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
                if (!tier_tok_min) continue;

                DisfluencyPatternDetector *PD = new DisfluencyPatternDetector();
                PD->setTiers(tiers);
                if (codes.contains("REP")) {
                    PD->revertToDisfluenciesLevel1();
                    QList<DisfluencyPatternDetector::RepetitionInfo> repetitions = PD->detectRepetitionPatterns();
                }
                if (codes.contains("INS")) {
                    QList<DisfluencyPatternDetector::InsertionInfo> insertions = PD->detectInsertionPatterns();
                    PD->codeInsertions(insertions);
                    bookmarks << PD->createBookmarks(com->corpusID(), com->ID(), annotationID, insertions);
                }
                if (codes.contains("SUB")) {
                    QList<DisfluencyPatternDetector::SubstitutionInfo> substitutions = PD->detectSubstitutionPatterns();
                    PD->codeSubstitutions(substitutions);
                    bookmarks << PD->createBookmarks(com->corpusID(), com->ID(), annotationID, substitutions);
                }
                delete PD;
            }
            com->repository()->annotations()->saveTiersAllSpeakers(annotationID, tiersAll);
            qDeleteAll(tiersAll);
            XMLSerialiserCorpusBookmark::saveCorpusBookmarks(bookmarks, com->repository()->files()->basePath() + "/autodisfluencies_bookmarks.xml");
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::exportMultiTierTextgrids(
        const QList<CorpusCommunication *> &communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: Exporting multi-tier textgrid"));

    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusCommunication *com, communications) {
        if (!com) continue;
        QString path = com->repository()->files()->basePath();
        printMessage(QString("Exporting %1").arg(com->ID()));
        foreach (CorpusAnnotation *annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                AnnotationTierGroup *tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                IntervalTier *tier_phone = 0, *tier_syll = 0, *tier_delivery = 0;
                IntervalTier *tier_tok_min = 0, *tier_tok_mwu = 0, *tier_ortho = 0;
                QStringList tierNamesToExclude; tierNamesToExclude << "words" << "tok-mwu" << "pos-mwu";
                QList<AnnotationTier *> restOfTiers;
                foreach(AnnotationTier *atier, tiers->tiers()) {
                    if      (atier->name() == "phone")       tier_phone = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "syll")        tier_syll = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "delivery")    tier_delivery = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "tok_min")     tier_tok_min = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "tok_mwu")     tier_tok_mwu = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "ortho")       tier_ortho = qobject_cast<IntervalTier *>(atier);
                    else {
                        restOfTiers << atier;
                    }
                }

                AnnotationTierGroup *txg = new AnnotationTierGroup();
                if (tier_phone) {
                    txg->addTier(tier_phone->cloneWithoutAttributes());
                }
                if (tier_syll) {
                    txg->addTier(tier_syll->cloneWithoutAttributes());
                    tier_delivery = tier_syll->cloneWithoutAttributes("delivery");
                    foreach (Interval *intv, tier_delivery->intervals())
                        if (intv->isPauseSilent()) intv->setText("_"); else intv->setText("");
                    txg->addTier(tier_delivery);
                }
                if (tier_tok_min) {\
                    txg->addTier(tier_tok_min->cloneWithoutAttributes("tok-min"));
                    IntervalTier *tier_pos_min = tier_tok_min->clone("pos-min");
                    foreach (Interval *intv, tier_pos_min->intervals())
                        intv->setText(intv->attribute("pos_min").toString());
                    txg->addTier(tier_pos_min);
                    IntervalTier *tier_disfluencyL1 = tier_tok_min->clone("disfluency-lex");
                    foreach (Interval *intv, tier_disfluencyL1->intervals()) {
                        QString d = intv->attribute("disfluency").toString();
                        if (d == "LEN" || d == "FST" || d == "FIL" || d == "WDP")
                            intv->setText(d);
                        else if (d.contains("SIL"))
                            intv->setText("SIL");
                        else
                            intv->setText("");
                        if (tier_delivery && (d == "FIL")) {
                            Interval *syll = tier_delivery->intervalAtTime(intv->tCenter());
                            if (syll) syll->setText("FIL");
                        }
                    }
                    txg->addTier(tier_disfluencyL1);
                    IntervalTier *tier_disfluencyL2 = tier_tok_min->clone("disfluency-rep");
                    foreach (Interval *intv, tier_disfluencyL2->intervals()) {
                        QString d = intv->attribute("disfluency").toString();
                        if (d.contains("REP"))
                            intv->setText(d.remove("+SIL").remove("+FST").remove("+LEN").remove("+FIL").remove("+WDP"));
                        else
                            intv->setText("");
                    }
                    txg->addTier(tier_disfluencyL2);
                    IntervalTier *tier_disfluencyL3 = tier_tok_min->clone("disfluency-struct");
                    foreach (Interval *intv, tier_disfluencyL3->intervals()) {
                        QString d = intv->attribute("disfluency").toString();
                        if (d.contains("INS") || d.contains("SUB") || d.contains("DEL") || d.contains("COM"))
                            intv->setText(d);
                        else
                            intv->setText("");
                    }
                    txg->addTier(tier_disfluencyL3);
                }
                if (tier_tok_mwu) {
                    txg->addTier(tier_tok_mwu->clone("tok-mwu"));
                    IntervalTier *tier_pos_mwu = tier_tok_mwu->clone("pos-mwu");
                    foreach (Interval *intv, tier_pos_mwu->intervals())
                        intv->setText(intv->attribute("pos_mwu").toString());
                    txg->addTier(tier_pos_mwu);
                }
                if (tier_ortho) {
                    txg->addTier(tier_ortho->clone());
                }
                if (txg->tiersCount() > 0) {
                    if (tier_phone) {
                        foreach (AnnotationTier *atier, txg->tiers()) {
                            IntervalTier *tier = qobject_cast<IntervalTier *>(atier);
                            if (tier)
                                tier->fixBoundariesBasedOnTier(tier_phone);
                        }
                    }
                    // add rest of tiers (not aligned to phones)
                    foreach (AnnotationTier *atier, restOfTiers) {
                        if (atier->tierType() == AnnotationTier::TierType_Points)
                            txg->addTier(qobject_cast<PointTier *>(atier)->clone());
                        else if (atier->tierType() == AnnotationTier::TierType_Intervals)
                            txg->addTier(qobject_cast<IntervalTier *>(atier)->clone());
                    }
                    // exclude tiers
                    foreach (QString name, tierNamesToExclude) {
                        txg->removeTierByName(name);
                    }
                    PraatTextGrid::save(path + "/" + annot->ID() + "_dismo.TextGrid", txg);
                }
            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}

void Praaline::Plugins::DisfluencyAnalyser::PluginDisfluencyAnalyser::process(const QList<CorpusCommunication *> &communications)
{
    QStringList patternsToAnnotate;
    if (d->commandPatternsREP) patternsToAnnotate << "REP";
    if (d->commandPatternsINS) patternsToAnnotate << "INS";
    if (d->commandPatternsSUB) patternsToAnnotate << "SUB";
    if (!patternsToAnnotate.isEmpty())
        patterns(communications, patternsToAnnotate);
    if (d->commandConcordances) {
        QString title = "annotationID\tspeakerID\ttMinSeqence\ttMaxSequence\tindexStart\tindexInterruption\tindexEnd\t";
        title = title.append("startReparandum\tendReparandum\tstartInterregnum\tendInterregnum\tstartReparans\tendReparans\t");
        title = title.append("numberOfTokensInReparandum\tnumberOf_SIL_InReparandum\tnumberOf_FIL_InReparandum\t");
        title = title.append("numberOf_SIL_InInterregnum\tnumberOf_FIL_InInterregnum\t");
        title = title.append("numberOfTokensInReparans\tnumberOf_SIL_InReparans\tnumberOf_FIL_InReparans\t");
        title = title.append("numberOfRepetitions\t");
        title = title.append("leftContext\tleftContext_SIL\t");
        title = title.append("reparandum\tinterregnum\tinterregnum_pos\tinterregnum_disfluency\treparans\t");
        title = title.append("rightContext\trightContext_SIL\t");
        title = title.append("reparans_arc\tinterregnum_arc\treparans_arc");
        printMessage(title);

        concordances(communications);
    }
    if (d->commandCreateSequences) {
        createSequences(communications);
    }
    if (d->commandExportMultiTierTextgrids)
        exportMultiTierTextgrids(communications);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::DisfluencyAnalyser;
    Q_EXPORT_PLUGIN2(PluginDisfluencyAnalyser, PluginDisfluencyAnalyser)
#endif
