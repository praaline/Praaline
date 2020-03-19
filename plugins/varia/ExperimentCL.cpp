#include <QString>
#include <QHash>
#include <QPair>
#include <QFile>
#include <QTextStream>
#include <QSharedPointer>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "PraalineMedia/AudioSegmenter.h"
using namespace Praaline::Media;

#include "ExperimentCL.h"

struct ExpBlock {
    ExpBlock() {}
    ExpBlock(QString name, QString startCode, QString endCode) :
        name(name), startCode(startCode), endCode(endCode) {}
    QString name;
    QString startCode;
    QString endCode;
};

struct ExperimentCLData {
    ExperimentCLData() : repositoryResults(nullptr)
    {}
    QHash<QString, ExpBlock> blocks;
    CorpusRepository *repositoryResults;
};

ExperimentCL::ExperimentCL() :
    d(new ExperimentCLData())
{
    d->blocks.insert("920", ExpBlock("STROOP_SING_CONG_SLOW_TRAIN", "920", "520"));
    d->blocks.insert("920", ExpBlock("STROOP_SING_CONG_SLOW_TRAIN", "920", "520"));
    d->blocks.insert("921", ExpBlock("STROOP_SING_CONG_SLOW_TEST1", "921", "521"));
    d->blocks.insert("922", ExpBlock("STROOP_SING_CONG_SLOW_TEST2", "922", "522"));
    d->blocks.insert("923", ExpBlock("STROOP_SING_CONG_SLOW_TEST3", "923", "523"));
    d->blocks.insert("924", ExpBlock("STROOP_SING_CONG_SLOW_TEST4", "924", "524"));
    d->blocks.insert("925", ExpBlock("STROOP_SING_CONG_SLOW_TEST5", "925", "525"));
    d->blocks.insert("930", ExpBlock("STROOP_SING_INCG_SLOW_TRAIN", "930", "530"));
    d->blocks.insert("931", ExpBlock("STROOP_SING_INCG_SLOW_TEST1", "931", "531"));
    d->blocks.insert("932", ExpBlock("STROOP_SING_INCG_SLOW_TEST2", "932", "532"));
    d->blocks.insert("933", ExpBlock("STROOP_SING_INCG_SLOW_TEST3", "933", "533"));
    d->blocks.insert("934", ExpBlock("STROOP_SING_INCG_SLOW_TEST4", "934", "534"));
    d->blocks.insert("935", ExpBlock("STROOP_SING_INCG_SLOW_TEST5", "935", "535"));
    d->blocks.insert("940", ExpBlock("STROOP_SING_INCG_FAST_TRAIN", "940", "540"));
    d->blocks.insert("941", ExpBlock("STROOP_SING_INCG_FAST_TEST1", "941", "541"));
    d->blocks.insert("942", ExpBlock("STROOP_SING_INCG_FAST_TEST2", "942", "542"));
    d->blocks.insert("943", ExpBlock("STROOP_SING_INCG_FAST_TEST3", "943", "543"));
    d->blocks.insert("944", ExpBlock("STROOP_SING_INCG_FAST_TEST4", "944", "544"));
    d->blocks.insert("945", ExpBlock("STROOP_SING_INCG_FAST_TEST5", "945", "545"));
    d->blocks.insert("950", ExpBlock("RSPAN_TRAIN",  "950", ""));
    d->blocks.insert("953", ExpBlock("RSPAN_LEVEL3", "953", ""));
    d->blocks.insert("954", ExpBlock("RSPAN_LEVEL4", "954", ""));
    d->blocks.insert("955", ExpBlock("RSPAN_LEVEL5", "955", ""));
    d->blocks.insert("956", ExpBlock("RSPAN_LEVEL6", "956", ""));
    d->blocks.insert("960", ExpBlock("STROOP_DUAL_CONG_SLOW_TRAIN", "960", "560"));
    d->blocks.insert("961", ExpBlock("STROOP_DUAL_CONG_SLOW_TEST1", "961", "561"));
    d->blocks.insert("962", ExpBlock("STROOP_DUAL_CONG_SLOW_TEST2", "962", "562"));
    d->blocks.insert("963", ExpBlock("STROOP_DUAL_CONG_SLOW_TEST3", "963", "563"));
    d->blocks.insert("964", ExpBlock("STROOP_DUAL_CONG_SLOW_TEST4", "964", "564"));
    d->blocks.insert("965", ExpBlock("STROOP_DUAL_CONG_SLOW_TEST5", "965", "565"));
    d->blocks.insert("970", ExpBlock("STROOP_DUAL_INCG_SLOW_TRAIN", "970", "570"));
    d->blocks.insert("971", ExpBlock("STROOP_DUAL_INCG_SLOW_TEST1", "971", "571"));
    d->blocks.insert("972", ExpBlock("STROOP_DUAL_INCG_SLOW_TEST2", "972", "572"));
    d->blocks.insert("973", ExpBlock("STROOP_DUAL_INCG_SLOW_TEST3", "973", "573"));
    d->blocks.insert("974", ExpBlock("STROOP_DUAL_INCG_SLOW_TEST4", "974", "574"));
    d->blocks.insert("975", ExpBlock("STROOP_DUAL_INCG_SLOW_TEST5", "975", "575"));
    d->blocks.insert("980", ExpBlock("STROOP_DUAL_INCG_FAST_TRAIN", "980", "580"));
    d->blocks.insert("981", ExpBlock("STROOP_DUAL_INCG_FAST_TEST1", "981", "581"));
    d->blocks.insert("982", ExpBlock("STROOP_DUAL_INCG_FAST_TEST2", "982", "582"));
    d->blocks.insert("983", ExpBlock("STROOP_DUAL_INCG_FAST_TEST3", "983", "583"));
    d->blocks.insert("984", ExpBlock("STROOP_DUAL_INCG_FAST_TEST4", "984", "584"));
    d->blocks.insert("985", ExpBlock("STROOP_DUAL_INCG_FAST_TEST5", "985", "585"));
}

ExperimentCL::~ExperimentCL()
{
    delete d;
}

QString ExperimentCL::extractTimingIntervals(Praaline::Core::CorpusCommunication *com)
{
    // cut-off points 200ms 400ms
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    CorpusRecording *recTiming(nullptr);
    foreach (CorpusRecording *rec, com->recordings()) {
        if (rec->ID().endsWith("_TIMING")) recTiming = rec;
    }
    if (!recTiming) return "Error: No timing recording";
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_timing = tiers->getIntervalTierByName("timing");
            if (!tier_timing) { ret.append("No timing tier\n"); continue; }
            for (int i = 0; i < tier_timing->count(); ++i) {
                Interval *intv = tier_timing->interval(i);
                if (intv->text() != "x") continue;
                if (intv->duration() < RealTime::fromMilliseconds(200)) continue;
                if (intv->duration() > RealTime::fromMilliseconds(400)) continue;
                QString filename = "/mnt/hgfs/CORPORA/CLETU/RECORDINGS/dtmf/" +
                        QString("%1_%2_t.wav").arg(annotationID).arg(i);
                AudioSegmenter::segment(recTiming->filePath(), filename,
                                        intv->tMin() - RealTime::fromMilliseconds(100),
                                        intv->tMax() + RealTime::fromMilliseconds(200));
                // correct sampling rate
//                AudioSegmenter::runSoxCommand(QString("sox %1 -t raw - | sox -t raw -e signed -b 16 -c 1 -r 44100 - %2")
//                                              .arg(filename).arg(QString(filename).replace("_t.wav", "_c.wav")));
//                QFile::remove(filename);
//                filename = QString(filename).replace("_t.wav", "_c.wav");
                AudioSegmenter::runSoxCommand(QString("-r 44100 %1 -r 8000 %2")
                                              .arg(filename).arg(QString(filename).replace("_t.wav", ".wav")));
                QFile::remove(filename);
                filename = QString(filename).replace("_t.wav", ".wav");
                //
                ret.append(filename).append("\n");
            }

            // com->repository()->annotations()->saveTier(annotationID, speakerID, tier_timing);
            // ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret.trimmed();
}

QString ExperimentCL::readDTMF(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    CorpusRecording *recTiming(nullptr);
    foreach (CorpusRecording *rec, com->recordings()) {
        if (rec->ID().endsWith("_TIMING")) recTiming = rec;
    }
    if (!recTiming) return "Error: No timing recording";
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_timing = tiers->getIntervalTierByName("timing");
            if (!tier_timing) { ret.append("No timing tier\n"); continue; }
            QString previousCode;
            for (int i = 0; i < tier_timing->count(); ++i) {
                Interval *intv = tier_timing->interval(i);
                intv->setAttribute("dtmf", "");
                if (intv->text() != "x") continue;
                if (intv->duration() < RealTime::fromMilliseconds(200)) continue;
                if (intv->duration() > RealTime::fromMilliseconds(400)) continue;

                QString path = "/mnt/hgfs/CORPORA/CLETU/RECORDINGS/dtmf/";

                QString path = "/Volumes/Samsung USB/CLETU/RECORDINGS/dtmf/";
                QString filename = path + QString("%1_%2.wav.txt").arg(annotationID).arg(i);
                QFile file(filename);
                if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
                    ret.append("Error opening file ").append(filename).append("\n");
                    continue;
                }
                QTextStream stream(&file);
                do {
                    QString code = stream.readLine();
                    code = code.trimmed();
                    if (code.isEmpty()) continue;
                    // correct double digits
                    if      (code == "5")   code = "555";
                    else if (code == "50")  code = "550";
                    else if (code == "52")	code = "522";
                    else if (code == "53") {
                        if      (previousCode == "933") code = "533";
                        else if ((previousCode == "953") || (previousCode == "553")) code = "553";
                    }
                    else if (code == "54") {
                        if      (previousCode == "944") code = "544";
                        else if ((previousCode == "954") || (previousCode == "554")) code = "554";
                    }
                    else if (code == "56")	code = "556";
                    else if (code == "57")	code = "557";
                    else if (code == "92")	code = "922";
                    else if (code == "93")	code = "933";
                    else if (code == "94")	code = "944";
                    else if (code == "95")	code = "955";
                    // update code
                    intv->setAttribute("dtmf", code);
                    previousCode = code;
                } while (!stream.atEnd());
                file.close();
                ret.append(filename).append("\t").append(intv->attribute("dtmf").toString()).append("\n");
            }

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_timing);
            // ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString ExperimentCL::createExpeBlocks(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_timing = tiers->getIntervalTierByName("timing");
            if (!tier_timing) { ret.append("No timing tier\n"); continue; }
            IntervalTier *tier_language = tiers->getIntervalTierByName("language");
            if (!tier_language) { ret.append("No language tier\n"); continue; }

            QList<Interval *> timing_marks;
            foreach (Interval *intv, tier_timing->intervals()) {
                if (!intv->attribute("dtmf").toString().isEmpty())
                    timing_marks << intv;
            }

            QList<Interval *> blocks;
            bool seenStartCode(false);
            QString insideExpBlockName;
            QString expectedEndCode;
            RealTime tMinExpBlock;
            for (int i = 0; i < timing_marks.count(); ++i) {
                Interval *mark = timing_marks.at(i);
                QString dtmf = mark->attribute("dtmf").toString();
                // Close current block, if appropriate
                if (seenStartCode) {
                    if (dtmf == expectedEndCode) {
                        // normal stop
                        ret.append(QString("%1\tBlock %2 tMin %3 tMax %4\n")
                                   .arg(annotationID).arg(insideExpBlockName).arg(tMinExpBlock.toDouble()).arg(mark->tMax().toDouble()));
                        // create block interval
                        Interval *block = new Interval(tMinExpBlock, mark->tMax(), insideExpBlockName);
                        block->setText(QString("%1_%2_%3")
                                       .arg(annotationID.left(4))
                                       .arg(tier_language->intervalAtTime(block->tCenter())->text())
                                       .arg(block->text()));
                        blocks << block;
                        // reset
                        seenStartCode = false;
                        insideExpBlockName = "";
                        expectedEndCode = "";
                        tMinExpBlock = RealTime();
                    }
                    else if (expectedEndCode.isEmpty()) {
                        // stop when new starts
                        ret.append(QString("%1\tBlock %2 tMin %3 tMax %4\n")
                                   .arg(annotationID).arg(insideExpBlockName).arg(tMinExpBlock.toDouble()).arg(mark->tMin().toDouble()));
                        // create block interval
                        Interval *block = new Interval(tMinExpBlock, mark->tMin(), insideExpBlockName);
                        block->setText(QString("%1_%2_%3")
                                       .arg(annotationID.left(4))
                                       .arg(tier_language->intervalAtTime(block->tCenter())->text())
                                       .arg(block->text()));
                        blocks << block;
                        // reset
                        seenStartCode = false;
                        insideExpBlockName = "";
                        expectedEndCode = "";
                        tMinExpBlock = RealTime();
                    }
                    else {
                        // error
                        ret.append(QString("%1\tWarning in block %2 Start code without corresponding end code %3 at time: %4 "
                                           "Seen code %5 instead. Creating block.\n")
                                   .arg(annotationID).arg(insideExpBlockName).arg(expectedEndCode).arg(timing_marks.at(i)->tMin().toDouble())
                                   .arg(dtmf));
                        // create block interval
                        Interval *block = new Interval(tMinExpBlock, mark->tMin(), insideExpBlockName);
                        block->setText(QString("%1_%2_%3")
                                       .arg(annotationID.left(4))
                                       .arg(tier_language->intervalAtTime(block->tCenter())->text())
                                       .arg(block->text()));
                        blocks << block;
                        // reset
                        seenStartCode = false;
                        insideExpBlockName = "";
                        expectedEndCode = "";
                        tMinExpBlock = RealTime();
                    }
                }
                // Start new block, if appropriate
                if (d->blocks.contains(dtmf)) {
                    // start new block
                    seenStartCode = true;
                    insideExpBlockName = d->blocks.value(dtmf).name;
                    expectedEndCode = d->blocks.value(dtmf).endCode;
                    tMinExpBlock = mark->tMin();
                }
            }
            // If the recording finished and we're still inside a block, finish the block
            if (seenStartCode) {
                ret.append(QString("%1\tWarning in block %2 Start code without corresponding end code %3 at time: %4 "
                                   "File finished instead. Creating block.\n")
                           .arg(annotationID).arg(insideExpBlockName).arg(expectedEndCode).arg(tier_timing->tMax().toDouble()));
                // create block interval
                Interval *block = new Interval(tMinExpBlock, tier_timing->tMax(), insideExpBlockName);
                block->setText(QString("%1_%2_%3")
                               .arg(annotationID.left(4))
                               .arg(tier_language->intervalAtTime(block->tCenter())->text())
                               .arg(block->text()));
                blocks << block;
            }
            // Create blocks tier
            IntervalTier *tier_exp_block = new IntervalTier("exp_block", blocks, tier_timing->tMin(), tier_timing->tMax());
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_exp_block);
            delete tier_exp_block;
            // ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString ExperimentCL::setResultsRepository(Praaline::Core::CorpusCommunication *com)
{
    d->repositoryResults = com->repository();
    return "OK";
}

QString ExperimentCL::importCSV_ETU_FR01(const QString &filename)
{
    // Stroop Time Pressure French
    // Create: FR_STROOP_SING_CONG_SLOW, FR_STROOP_SING_INCG_SLOW, FR_STROOP_SING_INCG_FAST
    QList<Interval *> list_stroop_stimulus;
    QList<Interval *> list_instructions;


    return QString("OK\t%1").arg(filename);
}

QString ExperimentCL::importCSV_ETU_FR02(const QString &filename)
{
    // Reading Span French
    // Create: FR_RSPAN
    QList<Interval *> list_rspan_sentence;
    QList<Interval *> list_rspan_memory_item;
    QList<Interval *> list_instructions;

    return QString("OK\t%1").arg(filename);
}

QString ExperimentCL::importCSV_ETU_FR03(const QString &filename)
{
    // Stroop Dual Task French
    // Create: FR_STROOP_DUAL_CONG_SLOW, FR_STROOP_DUAL_INCG_SLOW, FR_STROOP_DUAL_INCG_FAST
    QList<Interval *> list_stroop_stimulus;
    QList<Interval *> list_stroop_distractor;
    QList<Interval *> list_instructions;

    return QString("OK\t%1").arg(filename);
}

QString ExperimentCL::importCSV_ETU_EN01(const QString &filename)
{
    // Stroop Time Pressure English
    // Create: EN_STROOP_SING_CONG_SLOW, EN_STROOP_SING_INCG_SLOW, EN_STROOP_SING_INCG_FAST
    QList<Interval *> list_stroop_stimulus;
    QList<Interval *> list_instructions;

    return QString("OK\t%1").arg(filename);
}

QString ExperimentCL::importCSV_ETU_EN02(const QString &filename)
{
    // Reading Span English
    // Create: EN_RSPAN
    QList<Interval *> list_rspan_sentence;
    QList<Interval *> list_rspan_memory_item;
    QList<Interval *> list_instructions;

    return QString("OK\t%1").arg(filename);
}

QString ExperimentCL::importCSV_ETU_EN03(const QString &filename)
{
    // Stroop Dual Task English
    // Create: EN_STROOP_DUAL_CONG_SLOW, EN_STROOP_DUAL_INCG_SLOW, EN_STROOP_DUAL_INCG_FAST
    QList<Interval *> list_stroop_stimulus;
    QList<Interval *> list_stroop_distractor;
    QList<Interval *> list_instructions;

    return QString("OK\t%1").arg(filename);
}


