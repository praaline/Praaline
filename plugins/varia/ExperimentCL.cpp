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

struct ExperimentCLData {
    int i;
};

ExperimentCL::ExperimentCL() :
    d(new ExperimentCLData())
{
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

QString createExpePhases(Praaline::Core::CorpusCommunication *com)
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

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_timing);
            // ret.append(annotationID).append("\t").append(speakerID).append("\n");
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}
