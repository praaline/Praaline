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
    ExpBlock(QString name, QString startCode, QString endCode) :
        name(name), startCode(startCode), endCode(endCode) {}
    QString name;
    QString startCode;
    QString endCode;
};

struct ExperimentCLData {
    QHash<QString, ExpBlock> blocks;
};

ExperimentCL::ExperimentCL() :
    d(new ExperimentCLData())
{
    d->blocks.insert("920", ExpBlock("STROOP_CONG_SLOW_SING_TRAIN", "920", "520"));
    d->blocks.insert("920", ExpBlock("STROOP_CONG_SLOW_SING_TRAIN", "920", "520"));
    d->blocks.insert("921", ExpBlock("STROOP_CONG_SLOW_SING_TEST1", "921", "521"));
    d->blocks.insert("922", ExpBlock("STROOP_CONG_SLOW_SING_TEST2", "922", "522"));
    d->blocks.insert("923", ExpBlock("STROOP_CONG_SLOW_SING_TEST3", "923", "523"));
    d->blocks.insert("924", ExpBlock("STROOP_CONG_SLOW_SING_TEST4", "924", "524"));
    d->blocks.insert("925", ExpBlock("STROOP_CONG_SLOW_SING_TEST5", "925", "525"));
    d->blocks.insert("930", ExpBlock("STROOP_INCG_SLOW_SING_TRAIN", "930", "530"));
    d->blocks.insert("931", ExpBlock("STROOP_INCG_SLOW_SING_TEST1", "931", "531"));
    d->blocks.insert("932", ExpBlock("STROOP_INCG_SLOW_SING_TEST2", "932", "532"));
    d->blocks.insert("933", ExpBlock("STROOP_INCG_SLOW_SING_TEST3", "933", "533"));
    d->blocks.insert("934", ExpBlock("STROOP_INCG_SLOW_SING_TEST4", "934", "534"));
    d->blocks.insert("935", ExpBlock("STROOP_INCG_SLOW_SING_TEST5", "935", "535"));
    d->blocks.insert("940", ExpBlock("STROOP_INCG_FAST_SING_TRAIN", "940", "540"));
    d->blocks.insert("941", ExpBlock("STROOP_INCG_FAST_SING_TEST1", "941", "541"));
    d->blocks.insert("942", ExpBlock("STROOP_INCG_FAST_SING_TEST2", "942", "542"));
    d->blocks.insert("943", ExpBlock("STROOP_INCG_FAST_SING_TEST3", "943", "543"));
    d->blocks.insert("944", ExpBlock("STROOP_INCG_FAST_SING_TEST4", "944", "544"));
    d->blocks.insert("945", ExpBlock("STROOP_INCG_FAST_SING_TEST5", "945", "545"));
    d->blocks.insert("950", ExpBlock("RSPAN_TRAIN",  "950", ""));
    d->blocks.insert("953", ExpBlock("RSPAN_LEVEL3", "953", ""));
    d->blocks.insert("954", ExpBlock("RSPAN_LEVEL4", "954", ""));
    d->blocks.insert("955", ExpBlock("RSPAN_LEVEL5", "955", ""));
    d->blocks.insert("956", ExpBlock("RSPAN_LEVEL6", "956", ""));
    d->blocks.insert("960", ExpBlock("STROOP_CONG_SLOW_DUAL_TRAIN", "960", "560"));
    d->blocks.insert("961", ExpBlock("STROOP_CONG_SLOW_DUAL_TEST1", "961", "561"));
    d->blocks.insert("962", ExpBlock("STROOP_CONG_SLOW_DUAL_TEST2", "962", "562"));
    d->blocks.insert("963", ExpBlock("STROOP_CONG_SLOW_DUAL_TEST3", "963", "563"));
    d->blocks.insert("964", ExpBlock("STROOP_CONG_SLOW_DUAL_TEST4", "964", "564"));
    d->blocks.insert("965", ExpBlock("STROOP_CONG_SLOW_DUAL_TEST5", "965", "565"));
    d->blocks.insert("970", ExpBlock("STROOP_INCG_SLOW_DUAL_TRAIN", "970", "570"));
    d->blocks.insert("971", ExpBlock("STROOP_INCG_SLOW_DUAL_TEST1", "971", "571"));
    d->blocks.insert("972", ExpBlock("STROOP_INCG_SLOW_DUAL_TEST2", "972", "572"));
    d->blocks.insert("973", ExpBlock("STROOP_INCG_SLOW_DUAL_TEST3", "973", "573"));
    d->blocks.insert("974", ExpBlock("STROOP_INCG_SLOW_DUAL_TEST4", "974", "574"));
    d->blocks.insert("975", ExpBlock("STROOP_INCG_SLOW_DUAL_TEST5", "975", "575"));
    d->blocks.insert("980", ExpBlock("STROOP_INCG_FAST_DUAL_TRAIN", "980", "580"));
    d->blocks.insert("981", ExpBlock("STROOP_INCG_FAST_DUAL_TEST1", "981", "581"));
    d->blocks.insert("982", ExpBlock("STROOP_INCG_FAST_DUAL_TEST2", "982", "582"));
    d->blocks.insert("983", ExpBlock("STROOP_INCG_FAST_DUAL_TEST3", "983", "583"));
    d->blocks.insert("984", ExpBlock("STROOP_INCG_FAST_DUAL_TEST4", "984", "584"));
    d->blocks.insert("985", ExpBlock("STROOP_INCG_FAST_DUAL_TEST5", "985", "585"));
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
            bool seen933(false), seen953(false), seen954(false), seen944(false);
            for (int i = 0; i < tier_timing->count(); ++i) {
                Interval *intv = tier_timing->interval(i);
                intv->setAttribute("dtmf", "");
                if (intv->text() != "x") continue;
                if (intv->duration() < RealTime::fromMilliseconds(200)) continue;
                if (intv->duration() > RealTime::fromMilliseconds(400)) continue;
                QString filename = "/mnt/hgfs/CORPORA/CLETU/RECORDINGS/dtmf/" +
                        QString("%1_%2.wav.txt").arg(annotationID).arg(i);
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
                        if      (seen933) { code = "533"; seen933 = false; }
                        else if (seen953) { code = "553"; /* which may repeat */ }
                    }
                    else if (code == "54") {
                        if      (seen954) { code = "554"; /* which may repeat */ }
                        else if (seen944) { code = "544"; seen944 = false; }
                    }
                    else if (code == "56")	code = "556";
                    else if (code == "57")	code = "557";
                    else if (code == "92")	code = "922";
                    else if (code == "93")	code = "933";
                    else if (code == "94")	code = "944";
                    else if (code == "95")	code = "955";
                    // seen?
                    if      (code == "933") seen933 = true;
                    else if (code == "953") seen953 = true;
                    else if (code == "954") seen954 = true;
                    else if (code == "944") seen944 = true;
                    intv->setAttribute("dtmf", code);
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

QString createExpeBlocks(Praaline::Core::CorpusCommunication *com)
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
            QList<Interval *> timing_marks;
            foreach (Interval *intv, tier_timing->intervals()) {
                if (!intv->attribute("dtmf").toString().isEmpty())
                    timing_marks << intv;
            }

            QList<Interval *> blocks;


            // com->repository()->annotations()->saveTier(annotationID, speakerID, tier_expblock);
            // ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}
