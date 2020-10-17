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
#include "PraalineCore/Annotation/AnnotationDataTable.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "PraalineMedia/AudioSegmenter.h"
using namespace Praaline::Media;

#include "ExperimentCL_ImportFiles.h"

struct ExperimentCL_ImportFilesData {
    ExperimentCL_ImportFilesData() : repository(nullptr) {}
    CorpusRepository *repository;
};

ExperimentCL_ImportFiles::ExperimentCL_ImportFiles() :
    d(new ExperimentCL_ImportFilesData())
{
}

ExperimentCL_ImportFiles::~ExperimentCL_ImportFiles()
{
    delete d;
}

void ExperimentCL_ImportFiles::setCorpusRepository(CorpusRepository *rep)
{
    d->repository = rep;
}

QString translateColourName(const QString &colour, const QString &language)
{
    if (language == "FR") {
        if      (colour == "black")      return "noir";
        else if (colour == "green")      return "vert";
        else if (colour == "red")        return "rouge";
        else if (colour == "darkgray")   return "gris";
        else if (colour == "darkorange") return "orange";
        else if (colour == "blue")       return "bleu";
    }
    else if (language == "EN") {
        if      (colour == "darkgray")   return "grey";
        else if (colour == "darkorange") return "orange";
    }
    return colour;
}

QHash<QString, RealTime> ExperimentCL_ImportFiles::getDTMFTimes(const QString &subjectID, const QString &blockGroupID)
{
    QHash<QString, RealTime> dtmf_times;
    QList<Interval *> intervals_timing = d->repository->annotations()->getIntervals(
                AnnotationDatastore::Selection(QString("%1_%2").arg(subjectID).arg(blockGroupID), subjectID, "timing"));
    foreach (Interval *intv_timing, intervals_timing) {
        if (intv_timing->attribute("dtmf").toString().isEmpty()) continue;
        dtmf_times.insert(QString("DTMF") + intv_timing->attribute("dtmf").toString(), intv_timing->tMin());
    }
    return dtmf_times;
}

void ExperimentCL_ImportFiles::exportStroopTranscriptionTextgrid(const QString &subjectID, const QString &blockGroupID)
{
    QString annotationID = QString("%1_%2").arg(subjectID).arg(blockGroupID);
    SpeakerAnnotationTierGroupMap tiersAll = d->repository->annotations()->getTiersAllSpeakers(annotationID);
    foreach (QString speakerID, tiersAll.keys()) {
        AnnotationTierGroup *tiers = tiersAll.value(speakerID);
        if (!tiers) continue;
        IntervalTier *tier_utterance = tiers->getIntervalTierByName("utterance");
        IntervalTier *tier_stroop_stimulus = tiers->getIntervalTierByName("stroop_stimulus");
        IntervalTier *tier_timing = tiers->getIntervalTierByName("timing");
        IntervalTier *tier_exp_block = tiers->getIntervalTierByName("exp_block");
        // Fix stimulus tier tMax
        RealTime last_stim_tMax = tier_stroop_stimulus->tMax();
        if (tier_utterance && (tier_utterance->tMax() > last_stim_tMax)) {
            tier_stroop_stimulus->moveTierEnd(tier_utterance->tMax());
            tier_stroop_stimulus->split(last_stim_tMax);
            d->repository->annotations()->saveTier(annotationID, subjectID, tier_stroop_stimulus);
        }
        // Update utterance
        if (tier_utterance && tier_stroop_stimulus) {
            for (int i = tier_stroop_stimulus->intervals().count() - 1; i >= 0; --i) {
                Interval *stim = tier_stroop_stimulus->interval(i);
                if (stim->isPauseSilent()) continue;
                QList<Interval *> intervals_utt = tier_utterance->getIntervalsOverlappingWith(stim);
                foreach (Interval *utt, intervals_utt) {
                    if (utt->text() == "_") continue;
                    utt->setText(stim->attribute("target_colour").toString());
                    break;
                }
            }
        }
        // Export texgrid
        AnnotationTierGroup *txg = new AnnotationTierGroup();
        txg->addTier(tier_utterance);
        txg->addTier(tier_stroop_stimulus->getIntervalTierWithAttributeAsText("target_colour"));
        txg->addTier(tier_stroop_stimulus);
        IntervalTier *tier_dtmf = (tier_timing) ? tier_timing->getIntervalTierWithAttributeAsText("dtmf") : nullptr;
        txg->addTier(tier_dtmf);
        txg->addTier(tier_timing);
        txg->addTier(tier_exp_block);
        PraatTextGrid::save("/mnt/hgfs/CORPORA/CLETU/CORPUS/" + subjectID + "/" + annotationID + ".TextGrid", txg);
        delete txg;
    }
}

QString ExperimentCL_ImportFiles::importCSV_ETU_Stroop(const QString &subjectID, const QString &language, bool isDualTask, const QString &filename)
{
    QString ret;
    // Stroop Test
    QStringList blockGroupNames;
    if (language == "FR") {
        if (!isDualTask) blockGroupNames << "FR_STROOP_A1_CONG_SLOW" << "FR_STROOP_A2_INCG_SLOW" << "FR_STROOP_A3_INCG_FAST";
        else             blockGroupNames << "FR_STROOP_B1_CONG_SLOW" << "FR_STROOP_B2_INCG_SLOW" << "FR_STROOP_B3_INCG_FAST";
    }
    else if (language == "EN") {
        if (!isDualTask) blockGroupNames << "EN_STROOP_A1_CONG_SLOW" << "EN_STROOP_A2_INCG_SLOW" << "EN_STROOP_A3_INCG_FAST";
        else             blockGroupNames << "EN_STROOP_B1_CONG_SLOW" << "EN_STROOP_B2_INCG_SLOW" << "EN_STROOP_B3_INCG_FAST";
    }
    else return "Unknown language";
    // Read CSV file
    AnnotationDataTable table;
    table.setDelimiter(","); table.setTextQualifier("\"");
    if (!(table.readFromFile(filename))) {
        return QString("CSV file %1 not read.").arg(filename);
    }
    // Each file should be 324 lines long. Interesting fields : word, color, time_word_slow, time_word_fast
    int offset(0);
    // 3 blocks (congruent slow, incongruent slow, incongruent fast)
    for (int i_blockGroup = 0; i_blockGroup < 3; ++i_blockGroup) {
        QString blockGroupName = blockGroupNames.at(i_blockGroup);
        QHash<QString, RealTime> dtmf_times = getDTMFTimes(subjectID, blockGroupName);
        QList<Point *> list_stroop_stimulus_points;
        // 5 test blocks
        for (int num_block = 1; num_block <= 5; num_block++) {
            offset = (6 * 18 * i_blockGroup) + (num_block * 18);  // skipping training block
            RealTime time_dtmf_test_start = RealTime::fromSeconds(table.getData(offset, "time_dtmf_test_start").toDouble() / 1000.0);
            QString DTMF_start = table.getData(offset, "DTMF_start").toString();
            if (!dtmf_times.contains(DTMF_start)) {
                ret.append(QString("Warning: subject ID %1 block %2 not found, skipping\n").arg(subjectID).arg(DTMF_start));
                continue;
            }
            RealTime time_shift = dtmf_times[DTMF_start] - time_dtmf_test_start;
            // 18 items in each test
            int countHighTones(0); // for dual task
            for (int line = offset; line < offset + 18; ++line) {
                QString time_field = (i_blockGroup == 2) ? "time_word_fast" : "time_word_slow";
                // Hack to correct silly error in FR slow files (logger before word)
                int line_corr = ((blockGroupName == "FR_STROOP_A1_CONG_SLOW") || (blockGroupName == "FR_STROOP_A2_INCG_SLOW")) ? line + 1 : line;
                RealTime t_min = RealTime::fromSeconds(table.getData(line_corr, time_field).toDouble() / 1000.0) + time_shift;
                QString word = table.getData(line, "word").toString();
                if (word.contains("["))
                    word = table.getData(line, QString(word).remove("[").remove("]")).toString(); // indirection for incongruent words
                Point *stim = new Point(t_min, word);
                stim->setAttribute("target_colour", translateColourName(table.getData(line, "color").toString(), language));
                stim->setAttribute("order_block", num_block);
                stim->setAttribute("order_stim", line - offset + 1);
                if (isDualTask) {
                    if (table.getData(line, "shouldplay").toInt() == 1) {
                        if (table.getData(line, "tonetype").toInt() == 0)
                            stim->setAttribute("distractor", "L");
                        else if (table.getData(line, "tonetype").toInt() == 1) {
                            stim->setAttribute("distractor", "H");
                            countHighTones++;
                        }
                    }
                }
                list_stroop_stimulus_points << stim;
            }
            if (!isDualTask) {
                QString DTMF_end = table.getData(offset, "DTMF_end").toString();
                if (dtmf_times.contains(DTMF_end)) list_stroop_stimulus_points << new Point(dtmf_times[DTMF_end], "");
            } else {
                // Dual task
                RealTime t_end = RealTime::fromSeconds(table.getData(offset + 17, "time_logger").toDouble() / 1000.0) + time_shift;
                list_stroop_stimulus_points << new Point(t_end, "");
            }
        }
        PointTier *tier_stroop_stimulus_points = new PointTier("stroop_stimulus", list_stroop_stimulus_points);
        foreach (Point *p, tier_stroop_stimulus_points->points()) {
            ret.append(blockGroupNames.at(i_blockGroup)).append("\t")
               .append(QString::number(p->time().toDouble())).append("\t").append(p->text()).append("\n");
        }
        IntervalTier *tier_stroop_stimulus = tier_stroop_stimulus_points->getIntervalsMin("stroop_stimulus");
        d->repository->annotations()->saveTier(QString("%1_%2").arg(subjectID).arg(blockGroupName), subjectID, tier_stroop_stimulus);
        delete tier_stroop_stimulus; delete tier_stroop_stimulus_points;
        exportStroopTranscriptionTextgrid(subjectID, blockGroupName);
    }
    return ret; // QString("OK\t%1\t%2").arg(filename).arg(table.getRowCount());
}


QString ExperimentCL_ImportFiles::importCSV_ETU_ReadingSpan(const QString &subjectID, const QString &filename)
{
    Q_UNUSED(subjectID)
    // Reading Span French
    // Create: FR_RSPAN
    QList<Interval *> list_rspan_sentence;
    QList<Interval *> list_rspan_memory_item;
    QList<Interval *> list_instructions;
    // Read CSV file
    AnnotationDataTable table;
    table.setDelimiter(","); table.setTextQualifier("\"");
    if (!(table.readFromFile(filename))) {
        return QString("CSV file %1 not read.").arg(filename);
    }

    return QString("OK\t%1\t%2").arg(filename).arg(table.getRowCount());
}


