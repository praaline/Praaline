#include <QString>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QPointer>
#include <QtPlugin>
#include <QIcon>
#include <QApplication>
#include <ExtensionSystemConstants>

#include <QFile>
#include <QTextStream>

#include "chunkannotator.h"
#include "svbridge.h"
#include "pncore/interfaces/praat/praattextgrid.h"

#include "pncore/interfaces/anvil/AnvilMetadataTranscript.h"

#include "InterraterAgreement.h"
#include "prosodicboundariesexperiment.h"

#include "pluginvaria.h"


using namespace Qtilities::ExtensionSystem;
using namespace Praaline::Plugins;

struct Praaline::Plugins::Varia::PluginVariaPrivateData {
    PluginVariaPrivateData() {}
};

Praaline::Plugins::Varia::PluginVaria::PluginVaria(QObject* parent) : QObject(parent)
{
    d = new PluginVariaPrivateData;
    setObjectName(pluginName());
}

Praaline::Plugins::Varia::PluginVaria::~PluginVaria()
{
    delete d;
}

bool Praaline::Plugins::Varia::PluginVaria::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    return true;
}

bool Praaline::Plugins::Varia::PluginVaria::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)

    return true;
}

void Praaline::Plugins::Varia::PluginVaria::finalize() {

}

QString Praaline::Plugins::Varia::PluginVaria::pluginName() const {
    return "Varia";
}

QtilitiesCategory Praaline::Plugins::Varia::PluginVaria::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Praaline::Plugins::Varia::PluginVaria::pluginVersionInformation() const {
    VersionInformation version_info(1, 0, 0);
    return version_info;
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisher() const {
    return "George Christodoulides";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisherWebsite() const {
    return "http://www.corpusannotation.org";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginPublisherContact() const {
    return "info@corpusannotation.org";
}

QString Praaline::Plugins::Varia::PluginVaria::pluginDescription() const {
    return tr("Varia is a part-of-speech tagger, multi-word unit and disfluency detector for spoken language transcriptions.");
}

QString Praaline::Plugins::Varia::PluginVaria::pluginCopyright() const {
    return QString(tr("Copyright") + " 2012-2014, George Christodoulides");
}

QString Praaline::Plugins::Varia::PluginVaria::pluginLicense() const {
    return tr("GPL v.3");
}

QList<IAnnotationPlugin::PluginParameter> Praaline::Plugins::Varia::PluginVaria::pluginParameters() const
{
    QList<IAnnotationPlugin::PluginParameter> parameters;
    return parameters;
}

void Praaline::Plugins::Varia::PluginVaria::setParameters(QHash<QString, QVariant> parameters)
{
}

void chunk(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications) {
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
                IntervalTier *tier_tokens = tiers->getIntervalTierByName("tok_mwu");
                if (!tier_tokens) continue;
                IntervalTier *tier_chunks = tiers->getIntervalTierByName("chunk");
                if (!tier_chunks) {
                    tier_chunks = new IntervalTier("chunk", tiers->tMin(), tiers->tMax());
                    tiers->addTier(tier_chunks);
                }
                ChunkAnnotator chunker(tier_tokens, tier_chunks, "pos_mwu");
                chunker.annotate("d:/chunker/chunker_fr.model");
                corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_chunks);

                PraatTextGrid::save(corpus->basePath() + "/" + com->ID() + ".TextGrid", tiers);
            }
            qDeleteAll(tiersAll);
        }
    }
}

void Praaline::Plugins::Varia::PluginVaria::exportMultiTierTextgrids(
        Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    int countDone = 0;
    madeProgress(0);
    printMessage(QString("DisMo Disfluency Analyser ver. 0.1 running: Exporting multi-tier textgrid"));
    QString path = corpus->basePath();

    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        printMessage(QString("Exporting %1").arg(com->ID()));
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
            foreach (QString speakerID, tiersAll.keys()) {
                printMessage(QString("   speaker %1").arg(speakerID));
                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
                if (!tiers) continue;

                IntervalTier *tier_phone = 0, *tier_syll = 0;
                IntervalTier *tier_tok_min = 0, *tier_tok_mwu = 0, *tier_ortho = 0;
                QStringList tierNamesToExclude; tierNamesToExclude << "words" << "tok-mwu" << "pos-mwu";
                QList<AnnotationTier *> restOfTiers;
                foreach(AnnotationTier *atier, tiers->tiers()) {
                    if      (atier->name() == "phone")       tier_phone = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "syll")        tier_syll = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "tok_min")     tier_tok_min = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "tok_mwu")     tier_tok_mwu = qobject_cast<IntervalTier *>(atier);
                    else if (atier->name() == "ortho")       tier_ortho = qobject_cast<IntervalTier *>(atier);
                    else {
                        restOfTiers << atier;
                    }
                }

                QPointer<AnnotationTierGroup> txg = new AnnotationTierGroup();
                if (tier_phone) {
                    txg->addTier(new IntervalTier(tier_phone));
                }
                if (tier_syll) {
                    txg->addTier(new IntervalTier(tier_syll));
                    IntervalTier *tier_promdelivery = new IntervalTier(tier_syll, "prom-delivery");
                    foreach (Interval *intv, tier_promdelivery ->intervals()) {
                        intv->setText(intv->attribute("delivery1").toString());
                        if (intv->text().isEmpty())
                            intv->setText(intv->attribute("promise_pos").toString());
                    }
                    txg->addTier(tier_promdelivery);
                }
                if (tier_tok_min) {\
                    txg->addTier(new IntervalTier(tier_tok_min, "tok-min"));
                    IntervalTier *tier_pos_min = new IntervalTier(tier_tok_min, "pos-min");
                    foreach (Interval *intv, tier_pos_min->intervals())
                        intv->setText(intv->attribute("pos_min").toString());
                    txg->addTier(tier_pos_min);
                    IntervalTier *tier_disfluencyL1 = new IntervalTier(tier_tok_min, "disfluency-lex");
                    foreach (Interval *intv, tier_disfluencyL1->intervals()) {
                        intv->setText(intv->attribute("dis1_lex").toString());
                    }
                    txg->addTier(tier_disfluencyL1);
                    IntervalTier *tier_disfluencyL2 = new IntervalTier(tier_tok_min, "disfluency-rep");
                    foreach (Interval *intv, tier_disfluencyL2->intervals()) {
                        intv->setText(intv->attribute("dis1_rep").toString());
                    }
                    txg->addTier(tier_disfluencyL2);
                    IntervalTier *tier_disfluencyL3 = new IntervalTier(tier_tok_min, "disfluency-struct");
                    foreach (Interval *intv, tier_disfluencyL3->intervals()) {
                        intv->setText(intv->attribute("dis1_struct").toString());
                    }
                    txg->addTier(tier_disfluencyL3);
                }
                if (tier_tok_mwu) {
                    txg->addTier(new IntervalTier(tier_tok_mwu, "tok-mwu"));
                    IntervalTier *tier_pos_mwu = new IntervalTier(tier_tok_mwu, "pos-mwu");
                    foreach (Interval *intv, tier_pos_mwu->intervals())
                        intv->setText(intv->attribute("pos_mwu").toString());
                    txg->addTier(tier_pos_mwu);
                }
                if (tier_ortho) {
                    txg->addTier(new IntervalTier(tier_ortho));
                }
                if (txg->tiersCount() > 0) {
//                    if (tier_phone) {
//                        foreach (AnnotationTier *atier, txg->tiers()) {
//                            IntervalTier *tier = qobject_cast<IntervalTier *>(atier);
//                            if (tier)
//                                tier->fixBoundariesBasedOnTier(tier_phone);
//                        }
//                    }
                    // add rest of tiers (not aligned to phones)
                    foreach (AnnotationTier *atier, restOfTiers) {
                        if (atier->tierType() == AnnotationTier::TierType_Points)
                            txg->addTier(new PointTier(qobject_cast<PointTier *>(atier)));
                        else if (atier->tierType() == AnnotationTier::TierType_Intervals)
                            txg->addTier(new IntervalTier(qobject_cast<IntervalTier *>(atier)));
                    }
                    // exclude tiers
                    foreach (QString name, tierNamesToExclude) {
                        txg->removeTierByName(name);
                    }
                    PraatTextGrid::save(path + "/" + annot->ID() + "_disfprom.TextGrid", txg);
                }
            }
            qDeleteAll(tiersAll);
        }
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }
}


void Praaline::Plugins::Varia::PluginVaria::process(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{

//    int countDone = 0;
//    madeProgress(0);
//    printMessage(QString("Inter-rater agreement"));
//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            QString annotationID = annot->ID();
//            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;

//                IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//                IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");

//                InterraterAgreement agreement;
//                agreement.addGroup("0", QStringList() << "0");
//                agreement.addGroup("P", QStringList() << "P" << "L");
//                QList<Interval *> syllables = tier_syll->intervals();
//                foreach (Interval * syll, syllables) {
//                    if (syll->attribute("promise_pos").toString().isEmpty()) syll->setAttribute("promise_pos", "0");
//                    if (syll->attribute("delivery2").toString().isEmpty()) syll->setAttribute("delivery2", "0");
//                }
//                double k = agreement.getCohenKappa("0", "P", syllables, "promise_pos", "delivery2");
//                printMessage(QString("%1\t%2").arg(com->ID()).arg(k));
//            }
//            qDeleteAll(tiersAll);
//        }
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
//    }
//    return;

//    exportMultiTierTextgrids(corpus, communications);
//    return;

//    QString path = "/home/george/Dropbox/2015-10 SP8 - Prosodic boundaries perception experiment/RESULTS/Result files raw 22-02-2016";
//    QDir dirinfo(path);
//    QFileInfoList list;
//    list << dirinfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
//    dirinfo.setNameFilters(QStringList() << "*.csv");
//    list << dirinfo.entryInfoList();
//    foreach (QFileInfo info, list) {
//        if (!info.isDir()) {
//            QString subjectID = info.baseName().replace("expe1-subject-", "").replace("expe2-subject-", "")
//                    .replace("expe3-subject-", "").replace("expe4-subject-", "").replace(".csv", "");
//            if (subjectID.length() == 1) subjectID = "S20" + subjectID;
//            else subjectID = "S2" + subjectID;
//            QString expeType;
//            if (info.baseName().contains("expe1")) expeType = "1";
//            else if (info.baseName().contains("expe2")) expeType = "2";
//            else if (info.baseName().contains("expe3")) expeType = "3";
//            else if (info.baseName().contains("expe4")) expeType = "4";
//            // PBExpe::resultsReadParticipantInfo(subjectID, expeType, info.canonicalFilePath(), corpus);
//            // PBExpe::resultsReadTapping(subjectID, info.canonicalFilePath(), corpus);
//            printMessage(QString("Read data from %1").arg(info.baseName()));
//        }
//    }
//    corpus->save();
//    return;

//    PBExpe::analysisCalculateDeltaRT(corpus);
//    PBExpe::analysisCreateAdjustedTappingTier(corpus);

    PBExpe::analysisCalculateSmoothedTappingModel(corpus);
    PBExpe::analysisAttributeTappingToSyllablesLocalMaxima(corpus);
    PBExpe::analysisCalculateAverageDelay(corpus);
//    PBExpe::analysisCalculateCoverage(corpus);

//    PBExpe::analysisStabilisation(corpus, 10, 100);
//    PBExpe::analysisStabilisation(corpus, 20, 100);
//    PBExpe::analysisStabilisation(corpus, 30, 100);
//    PBExpe::analysisStabilisation(corpus, 15, 100);
//    PBExpe::analysisStabilisation(corpus, 25, 100);


//    PBExpe::statExtractFeaturesForModelling(corpus);
    // PBExpe::statInterAnnotatorAgreement(corpus);
    // PBExpe::statCorrespondanceNSandMS(corpus);


//    QString path = "C:/Users/George/Downloads/Rhap_meta";
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        bool result = AnvilMetadataTranscript::load(QString("%1/%2-meta.xml").arg(path).arg(com->ID()), corpus);
//        printMessage(QString("%1 %2").arg(com->ID()).arg((result) ? "OK" : "Error"));
//    }



//    analyseBoundaries(corpus, communications);
//    return;
//    chunk(corpus, communications);
//    return;

//    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        if (!com->hasRecordings()) continue;
//        QPointer<CorpusRecording> rec = com->recordings().first();
//        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//            if (!annot) continue;
//            QString annotationID = annot->ID();
//            tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
//            foreach (QString speakerID, tiersAll.keys()) {
//                QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//                if (!tiers) continue;
//                PraatTextGrid::save(corpus->basePath() + "/" + com->ID() + ".TextGrid", tiers);
//            }
//            qDeleteAll(tiersAll);
//        }
//    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Praaline::Plugins::Varia;
    Q_EXPORT_PLUGIN2(PluginVaria, PluginVaria)
#endif


//IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//AnnotationTierGroup *txg = new AnnotationTierGroup();
//PraatTextGrid::load(QString("D:/corpora/locasf/%1.textgrid").arg(speakerID), txg);
//if (!txg) printMessage("cannot open textgrid");
//PointTier *tier_boundary = txg->getPointTierByName("boundary");
//foreach (Point *point, tier_boundary->points()) {
//    Interval *syll = tier_syll->intervalAtTime(point->timePoint() - RealTime(0, 100));
//    if (!syll) { printMessage(QString("no syll at %1").arg(point->timePoint().toDouble())); continue; }
//    syll->setAttribute("boundary", point->text());
//}
//corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_syll);


//IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
//if (!tier_phone) continue;
//QString filenameRec = corpus->baseMediaPath() + "/" + rec->filename();
//QFileInfo info(filenameRec);
//QString filename = info.absolutePath() + QString("/%1_phone.svl").arg(speakerID);
//SVBridge::saveSVTimeInstantsLayer(filename, rec->sampleRate(), tier_phone);


//QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
//foreach (QPointer<CorpusCommunication> com, communications) {
//    if (!com) continue;
//    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
//        if (!annot) continue;
//        QString annotationID = annot->ID();
//        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
//        foreach (QString speakerID, tiersAll.keys()) {
//            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
//            if (!tiers) continue;

//            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
//            QStringList subjectsA, subjectsB, subjects;
//            subjectsA << "AH" << "CdlC" << "GB" << "LF" << "MB" << "NH";
//            subjectsB << "AEL" << "AP" << "JLV" << "ML" << "SL" << "ZT";
//            subjects << subjectsA << subjectsB;
//            foreach (QString code, QStringList() << "P" << "Z") {
//                foreach (Interval *syll, tier_syll->intervals()) {
//                    int score = 0, scoreA = 0, scoreB = 0;
//                    foreach (QString subject, subjectsA) {
//                        if (syll->attribute(QString("prom_%1").arg(subject)).toString().toUpper() == code) scoreA++;
//                    }
//                    foreach (QString subject, subjectsB) {
//                        if (syll->attribute(QString("prom_%1").arg(subject)).toString().toUpper() == code) scoreB++;
//                    }
//                    score = scoreA + scoreB;
//                    syll->setAttribute(QString("%1_scoreA").arg(code), scoreA);
//                    syll->setAttribute(QString("%1_scoreB").arg(code), scoreB);
//                    syll->setAttribute(QString("%1_score").arg(code), score);
//                }
//            }
//            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_syll);
//        }
//        qDeleteAll(tiersAll);
//    }
//}

//QString path = "D:/DROPBOX/LCLIG2210/2014-2015 Projet/Zeinab/";
//AnnotationTierGroup *txg = new AnnotationTierGroup();
//PraatTextGrid::load(path + "Tier_ortho-phono-mot-syllabe-zeinab_traore.textgrid", txg);
//IntervalTier *phones = txg->getIntervalTierByName("phones");
//if (!phones) {
//    printMessage("Tier not found phones");
//    return;
//}
//foreach(AnnotationTier *t, txg->tiers()) {
//    if (t->tierType() != AnnotationTier::TierType_Intervals) continue;
//    IntervalTier *tier = (IntervalTier *)t;
//    tier->fixBoundariesBasedOnTier(phones, RealTime(0, 10000000));
//    printMessage(QString("aligned tier %1").arg(tier->name()));
//}
//PraatTextGrid::save(path + "zeinab_participant01b_corr.textgrid", txg);
