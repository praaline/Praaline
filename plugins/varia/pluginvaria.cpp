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
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "pncore/interfaces/anvil/AnvilMetadataTranscript.h"

#include "InterraterAgreement.h"
#include "prosodicboundariesexperiment.h"
#include "MyExperiments.h"
#include "ProsodyCourse.h"
#include "DisfluenciesExperiments.h"
#include "SpeechRateExperiments.h"

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

void Praaline::Plugins::Varia::PluginVaria::setParameters(const QHash<QString, QVariant> &parameters)
{
    Q_UNUSED(parameters)
}

void chunk(QList<QPointer<CorpusCommunication> > communications) {
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            QString annotationID = annot->ID();
            tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
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
                com->repository()->annotations()->saveTier(annotationID, speakerID, tier_chunks);

                // PraatTextGrid::save(corpus->basePath() + "/" + com->ID() + ".TextGrid", tiers);
            }
            qDeleteAll(tiersAll);
        }
    }
}

//QString path = "/home/george/Dropbox/2016 Experiences perceptives disfluences/01_GAMEANNOTATOR_ANALYSIS/Annotation2";
//QDir dirinfo(path);
//QFileInfoList list;
//list << dirinfo.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs);
//dirinfo.setNameFilters(QStringList() << "*.txt");
//list << dirinfo.entryInfoList();
//foreach (QFileInfo info, list) {
//    if (!info.isDir()) {
//        DisfluenciesExperiments::resultsReadTapping("2", info.canonicalFilePath(), corpus);
//        printMessage(QString("Read data from %1").arg(info.baseName()));
//    }
//}
//corpus->save();
//return;


void Praaline::Plugins::Varia::PluginVaria::process(const QList<QPointer<CorpusCommunication> > &communications)
{    
    if (communications.isEmpty()) return;
    SpeechRateExperiments sr;
    QString path = "/home/george/Dropbox/MIS_Phradico/Experiences/02_perception-macroprosodie/Results raw files/";
    sr.readResultsFile(communications.first()->repository(), path + "results_P4.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P5.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P6.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P7.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P9.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P10.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P11.xml");
    sr.readResultsFile(communications.first()->repository(), path + "results_P12.xml");

//    sr.readResultsFile(communications.first()->repository(), path + "results_P1.txt");
//    sr.readResultsFile(communications.first()->repository(), path + "results_P2.txt");
//    sr.readResultsFile(communications.first()->repository(), path + "results_P3.txt");
    return;

//    // DisfluenciesExperiments::analysisCalculateDeltaRT(communications);
//    DisfluenciesExperiments::analysisCreateAdjustedTappingTier(communications);
//    return;


    int countDone = 0;
    madeProgress(0);
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        // MyExperiments::createTextgridsFromAutosyll(corpus, com);
        // MyExperiments::updateTranscriptionMode(corpus, com);
        countDone++;
        madeProgress(countDone * 100 / communications.count());
    }

//    ProsodyCourse::syllableTables(corpus);
//    return;

//    int countDone = 0;
//    madeProgress(0);
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        if (!com) continue;
//        TemporalAnalyser analyser;
//        analyser.calculate(corpus, com);
//        foreach (QString measureID, analyser.measureIDsForCommunication()) {
//            l
//        }
//        QString header("\t");
//        foreach (QString speakerID, analyser.speakerIDs()) {
//            header.append(speakerID).append("\t");
//        }
//        header.chop(1);
//        printMessage(header);
//        foreach(QString measureID, analyser.measureIDsForSpeaker()) {
//            QString line;
//            line.append(analyser.measureDefinitionForSpeaker(measureID).displayNameUnit()).append("\t");
//            foreach (QString speakerID, analyser.speakerIDs()) {
//                line.append(QString("%1").arg(analyser.measureSpk(speakerID, measureID))).append("\t");
//            }
//            line.chop(1);
//            printMessage(line);
//        }
//        printMessage("");
//        countDone++;
//        madeProgress(countDone * 100 / communications.count());
//    }
//    return;


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

//    PBExpe::actualStimuliFromCorpus(corpus, communications);

//    PBExpe::analysisCalculateDeltaRT(corpus);
//    PBExpe::analysisCreateAdjustedTappingTier(corpus);

//    PBExpe::analysisCalculateSmoothedTappingModel(corpus);

//   PBExpe::createProsodicUnits(corpus);
//    QStringList transcripts = PBExpe::printTranscriptionInProsodicUnits(corpus);
//    foreach (QString t, transcripts) printMessage(t);

//    PBExpe::analysisAttributeTappingToSyllablesLocalMaxima(corpus, "tok_min", "boundary");
//    PBExpe::analysisCalculateAverageDelay(corpus, "boundary2");
//    PBExpe::analysisCalculateCoverage(corpus, "boundary2");
//    PBExpe::analysisCheckBoundaryRightAfterPause(corpus);

//    PBExpe::analysisStabilisation(corpus, 10, 100);
//    PBExpe::analysisStabilisation(corpus, 20, 100);
//    PBExpe::analysisStabilisation(corpus, 30, 100);
//    PBExpe::analysisStabilisation(corpus, 15, 100);
//    PBExpe::analysisStabilisation(corpus, 25, 100);


//    PBExpe::statExtractFeaturesForModelling(corpus, true, false, "boundary");
//    PBExpe::statInterAnnotatorAgreement(corpus);
//    PBExpe::statCorrespondanceNSandMS(corpus, "boundary");


//    QString path = "C:/Users/George/Downloads/Rhap_meta";
//    foreach (QPointer<CorpusCommunication> com, communications) {
//        bool result = AnvilMetadataTranscript::load(QString("%1/%2-meta.xml").arg(path).arg(com->ID()), corpus);
//        printMessage(QString("%1 %2").arg(com->ID()).arg((result) ? "OK" : "Error"));
//    }



//    analyseBoundaries(corpus, communications);
//    return;
//    chunk(corpus, communications);
//    return;

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
