#include <QTemporaryDir>
#include <QFileInfo>
#include "prosoboxscript.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"

ProsoboxScript::ProsoboxScript(QObject *parent) : AnnotationPluginPraatScript(parent)
{
}

ProsoboxScript::~ProsoboxScript()
{
}

void ProsoboxScript::createTextGrid(QString path, Corpus *corpus, QString annotationID, QString speakerID)
{
    AnnotationTierGroup *tiers = corpus->repository()->annotations()->getTiers(annotationID, speakerID);
    if (!tiers) return;
    AnnotationTierGroup *txg = new AnnotationTierGroup();
    IntervalTier *tier_phone = tiers->getIntervalTierByName("phone");
    IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
    IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
    IntervalTier *tier_tok_mwu = tiers->getIntervalTierByName("tok_mwu");
    IntervalTier *tier_ortho = tiers->getIntervalTierByName("segment");
    if (tier_phone) txg->addTier(tier_phone->clone());
    if (tier_syll) {
        foreach (Interval *syll, tier_syll->intervals()) {
            syll->setText(syll->text().replace("\t", ""));
        }
        txg->addTier(tier_syll->clone());
        IntervalTier *tier_promise = tier_syll->clone("promise");
        foreach (Interval *intv, tier_promise->intervals()) intv->setText(intv->attribute("promise").toString());
        txg->addTier(tier_promise);
    }
    if (tier_tok_min) {
        txg->addTier(tier_tok_min->clone());
        IntervalTier *tier_pos_min = tier_tok_min->clone("pos_min");
        foreach (Interval *intv, tier_pos_min->intervals()) intv->setText(intv->attribute("pos_min").toString());
        txg->addTier(tier_pos_min);
    }
    if (tier_tok_mwu) {
        txg->addTier(tier_tok_mwu->clone());
        IntervalTier *tier_pos_mwu = tier_tok_mwu->clone("pos_mwu");
        foreach (Interval *intv, tier_pos_mwu->intervals()) intv->setText(intv->attribute("pos_mwu").toString());
        txg->addTier(tier_pos_mwu);
    }
    if (tier_ortho) txg->addTier(tier_ortho->clone("ortho"));
    foreach (QString tierName, txg->tierNames()) {
        IntervalTier *tier = txg->getIntervalTierByName(tierName);
        if (!tier) continue;
        tier->fixBoundariesBasedOnTier(tier_phone);
    }
    IntervalTier *tier_timeline = corpus->repository()->annotations()->getSpeakerTimeline("", annotationID, "segment");
    if (tier_timeline) {
        tier_timeline->replace("", speakerID, "L1");
        tier_timeline->replace("", "L1+L2", "L1");
        tier_timeline->replace("", "L2+L1", "L1");
        tier_timeline->fillEmptyWith("", "_");
        tier_timeline->mergeIdenticalAnnotations(QString(), QStringList() << "_");
        tier_timeline->setName("speaker");
        txg->addTier(tier_timeline);
    }
    PraatTextGrid::save(QString("%1/%2_%3.TextGrid").arg(path).arg(annotationID).arg(speakerID), txg);
    delete txg;
}

void ProsoboxScript::runCreateSyllTable(QString path, Corpus *corpus, QString annotationID, QString speakerID)
{
    Q_UNUSED(path)
    Q_UNUSED(corpus)
    Q_UNUSED(annotationID)
    Q_UNUSED(speakerID)

//    QString filenameTempRec = QString("%1_%2.wav").arg(rec->ID()).arg(speakerID);
//    QFile::copy(corpus->baseMediaPath() + rec->filename(), tempDirectory + filenameTempRec);
//    QFileInfo info(corpus->baseMediaPath() + rec->annotation(0)->filename());
//    QString annotationID = info.fileName().replace(".TextGrid", "", Qt::CaseInsensitive);
//    QString absPath = info.absoluteDir().absolutePath() + "/";
//    QString prosoPath = absPath + "prosogram/";
//    qDebug() << annotationID;
//    qDebug() << absPath;
//    qDebug() << prosoPath;
//    // Execute makesyll
//    QString appPath = QCoreApplication::applicationDirPath();
//    QString script = appPath + "/plugins/prosobox5/praaline_makesylltable.praat";
//    QStringList scriptArguments;
//    QString syllTiers = "delivery,if,prom,promauto";
//    QString suprasyllTiers = "tok-mwu,AP,ss,speaker";
//    QString pauseTier = "ss";
//    scriptArguments << annotationID << absPath << prosoPath <<
//                       "phones" << "syll" << syllTiers << "1" << "1" << suprasyllTiers << pauseTier << "1";
//    executePraatScript(script, scriptArguments);
}

void ProsoboxScript::runProsoProm(Corpus *corpus, CorpusRecording *rec, AnnotationTierGroup *tiers, QString annotationID, QString speakerID)
{
    Q_UNUSED(tiers)

    // Create a temporary directory
    QTemporaryDir dirTemp;
    if (!dirTemp.isValid()) return;
    QString tempDirectory = dirTemp.path(); // returns the unique directory path
    if (!tempDirectory.endsWith("/")) tempDirectory.append("/");
    // Copy the recording file into temp+.wav
    QString filenameTempRec = QString("%1_%2.wav").arg(annotationID).arg(speakerID);
    QFile::copy(rec->filePath(), tempDirectory + filenameTempRec);
    // Copy a textgrid with the needed information into temp+.textgrid
    createTextGrid(tempDirectory, corpus, annotationID, speakerID);
    QString filenameTempAnnot = QString("%1_%2.TextGrid").arg(annotationID).arg(speakerID);


//    QString filenameAnnotation = com->basePath() + com->annotation(0)->filename();
//    QString filenameTable = QString(filenameAnnotation).replace(".TextGrid", ".Table", Qt::CaseInsensitive);
//    // Execute ProsoProm
//    QString appPath = QCoreApplication::applicationDirPath();
//    QString script = appPath + "/plugins/prosobox5/prosoprom/prosoprom.praat";
//    QStringList scriptArguments;
//    QString strategy = "gradual"; QString promTierName = "promauto"; QString insertPromgrad = "1";
//    // QString doPromEval = "0"; QString addRelaviveParameterTiers = "0";

//    // f0param, blockpause, scope, weight
//    scriptArguments << filenameAnnotation << filenameTable <<
//                       "mean" << "0.250" << "2" << "1" << "1" <<
//                       strategy << promTierName << insertPromgrad;
//    // << doPromEval << addRelaviveParameterTiers << "3 full";
//    executePraatScript(script, scriptArguments);
}



void ProsoboxScript::runMakeIFtier(Corpus *corpus, CorpusRecording *rec, AnnotationTierGroup *tiers, QString annotationID, QString speakerID)
{
    Q_UNUSED(corpus)
    Q_UNUSED(rec)
    Q_UNUSED(tiers)
    Q_UNUSED(annotationID)
    Q_UNUSED(speakerID)
//    if (com->annotationsCount() == 0)
//        return;
//    QString filenameAnnotation = com->basePath() + com->annotation(0)->filename();
//    // Execute PromGrad
//    QString appPath = QCoreApplication::applicationDirPath();
//    QString script = appPath + "/plugins/various/make_if_tier_from_lex3.praat";
//    QStringList scriptArguments;

//    scriptArguments << filenameAnnotation << "phones" << "syll" << "if" << "tok-min" << "AP";
//    executePraatScript(script, scriptArguments);

}

