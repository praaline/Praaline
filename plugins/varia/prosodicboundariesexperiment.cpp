#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>
#include <QPointer>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include "prosodicboundaries.h"
#include "pncore/base/RealValueList.h"
#include "svcore/data/model/SparseOneDimensionalModel.h"
#include "svcore/data/model/SparseTimeValueModel.h"
#include "pncore/corpus/corpus.h"
#include "pncore/corpus/corpusspeaker.h"

#include "prosodicboundaries.h"
#include "prosodicboundariesexperiment.h"

bool PBExpe::ipuIsMonological(Interval *ipu, IntervalTier *timeline, QString &speaker)
{
    QString spk;
    QList<Interval *> timelineIntervals = timeline->getIntervalsOverlappingWith(ipu);
    bool first = true;
    foreach (Interval *timelineIntv, timelineIntervals) {
        if (timelineIntv->text().isEmpty()) continue;
        if (first) {
            spk = timelineIntv->text(); first = false;
        }
        else {
            if (spk != timelineIntv->text()) {
                qDebug() << spk << timelineIntv->text();
                return false;
            }
        }
    }
    speaker = spk;
    return true;
}

void PBExpe::measuresForPotentialStimuli(QPointer<CorpusAnnotation> annot, QList<Interval *> &stimuli, QTextStream &out,
                                         IntervalTier *tier_syll, IntervalTier *tier_tokmin)
{
    out << "annotationID\tstimulusID\tduration\tnumSilentPauses\tpauseTotalDur\tarticulationRatio\tarticulationRate\tspeechRate\tsyllDurationCV\tnumFilledPauses\tnumRepetitionDIPs\tratioFILtoNumSyll\tmelodicPath\tnumProsodicBoundaries\tnumBoundariesT2\tnumBoundariesT3\tratioSILtoPBs\ttranscription\n";
    foreach (Interval *stim, stimuli) {
        double pauseTotalDur = 0.0;
        int numSyllables = 0, numSilentPauses = 0, numFilledPauses = 0, numRepetitionDIPs = 0;
        int numProsodicBoundaries = 0, numBoundariesT2 = 0, numBoundariesT3 = 0;
        double trajectory = 0.0;
        QList<Interval *> syllables = tier_syll->getIntervalsContainedIn(stim);
        QList<Interval *> tokens = tier_tokmin->getIntervalsContainedIn(stim);
        RealValueList syll_durations;
        foreach (Interval *syll, syllables) {
            if (syll->text() == "_" || syll->text().isEmpty()) {
                numSilentPauses++;
                pauseTotalDur += syll->duration().toDouble();
            } else {
                numSyllables++;
                trajectory += syll->attribute("trajectory").toDouble();
                // exclude filled pauses from CV calculation
                Interval *token = tier_tokmin->intervalAtTime(syll->tCenter());
                if (!token->attribute("disfluency").toString().contains("FIL"))
                    syll_durations.append(syll->duration().toDouble());
            }
            QString boundary = syll->attribute("boundary").toString();
            QString contour = syll->attribute("contour").toString();
            if (boundary.contains("//") || boundary.contains("///")) {
                numProsodicBoundaries++;
                if (contour == "T" && boundary.contains("///")) numBoundariesT3++;
                else if (contour == "T" && boundary.contains("//")) numBoundariesT2++;
            }
        }
        double articulationRatio = (stim->duration().toDouble() - pauseTotalDur) / stim->duration().toDouble();
        double speechRate = syllables.count() / stim->duration().toDouble();
        double articulationRate = syllables.count() / (stim->duration().toDouble() - pauseTotalDur);
        double syllDurationCV = syll_durations.stddev() / syll_durations.mean();
        double melodicPath = trajectory / (stim->duration().toDouble() - pauseTotalDur);
        foreach (Interval *token, tokens) {
            if (token->attribute("disfluency").toString().contains("FIL")) numFilledPauses++;
            if (token->attribute("disfluency").toString().contains("REP*")) numRepetitionDIPs++;
        }

        out << annot->ID() << "\t" << stim->text() << "\t";
        out << QString("%1").arg(stim->duration().toDouble()).replace(".", ",") << "\t";
        // Silent pauses, number and duration
        out << numSilentPauses << "\t";
        out << QString("%1").arg(pauseTotalDur).replace(".", ",") << "\t";
        // Speech rate, articulation rate
        out << QString("%1").arg(articulationRatio).replace(".", ",") << "\t";
        out << QString("%1").arg(articulationRate).replace(".", ",") << "\t";
        out << QString("%1").arg(speechRate).replace(".", ",") << "\t";
        // Coefficient of variation of syllable length
        out << QString("%1").arg(syllDurationCV).replace(".", ",") << "\t";
        // Disfluencies
        out << numFilledPauses << "\t" << numRepetitionDIPs << "\t";
        out << QString("%1").arg(double(numFilledPauses) / double(numSyllables)).replace(".", ",") << "\t";
        // Melodic path
        out << QString("%1").arg(melodicPath).replace(".", ",") << "\t";
        // Perceptual boundary annotation
        out << numProsodicBoundaries << "\t" << numBoundariesT2 << "\t" << numBoundariesT3 << "\t";
        out << QString("%1").arg(double(numSilentPauses) / double(numProsodicBoundaries)).replace(".", ",") << "\t";

        out << stim->attribute("transcription").toString() << "\n";
    }

}

void PBExpe::potentialStimuliFromSample(Corpus *corpus, QPointer<CorpusAnnotation> annot, QTextStream &out)
{
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annot->ID());
    foreach (QString speakerID, tiersAll.keys()) {
        QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);

        IntervalTier *timeline = corpus->datastoreAnnotations()->getSpeakerTimeline(annot->ID(), "tok_mwu");
        IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
        IntervalTier *tier_tokmin = tiers->getIntervalTierByName("tok_min");
        QList<Interval *> pauseBoundariesForIPUs;
        QList<Interval *> potentialStimuli;
        double pauseThreshold = 0.250, minimumStimulusLength = 20.0, maximumStimulusLength = 60.0;

        // Select pauses that are over the threshold => potential IPU boundaries
        foreach (Interval *syll, tier_syll->intervals()) {
            if (syll->text() == "_") {
                // first and last silence are always included
                int pause_index = timeline->intervalIndexAtTime(syll->tCenter());
                if (pause_index == 0 || pause_index == timeline->countItems() - 1) {
                    pauseBoundariesForIPUs << new Interval(syll);
                }
                // exclude if pause is smaller than threshold
                if (syll->duration().toDouble() < pauseThreshold) continue;
                // add to IPU boundaries
                pauseBoundariesForIPUs << new Interval(syll);
            }
        }
        while (potentialStimuli.isEmpty() && (minimumStimulusLength > 2.0)) {
            // Select sequences of IPUs that are monological and are between the min, max stimulus length
            for (int i = 0; i < pauseBoundariesForIPUs.count() - 1; ++i) {
                for (int j = i + 1; j < pauseBoundariesForIPUs.count(); ++j) {
                    Interval *left = pauseBoundariesForIPUs.at(i); Interval *right = pauseBoundariesForIPUs.at(j);
                    RealTime stimulusDuration = right->tMin() - left->tMax();
                    if (stimulusDuration.toDouble() < minimumStimulusLength) continue;
                    if (stimulusDuration.toDouble() > maximumStimulusLength) break;
                    qDebug() << stimulusDuration.toDouble();
                    Interval *potentialStimulus = new Interval(left->tMax(), right->tMin(), "");
                    QString spk;
                    if (!ipuIsMonological(potentialStimulus, timeline, spk)) {
                        delete potentialStimulus;
                        continue;
                    }
                    // Got a potential stimulus!
                    QList<Interval *> tokens = tier_tokmin->getIntervalsContainedIn(potentialStimulus);
                    QString transcription;
                    foreach (Interval *token, tokens) {
                        transcription.append(token->text()).append(" ");
                    }
                    potentialStimulus->setText(QString("IPU %1 (%2) %3 - %4").arg(i+1).arg(spk)
                                               .arg(left->tMax().toDouble()).arg(right->tMin().toDouble()));
                    potentialStimulus->setAttribute("transcription", transcription);
                    potentialStimulus->setAttribute("speaker", spk);
                    potentialStimuli << potentialStimulus;
                }
            }
            // Measures on potential stimuli
            measuresForPotentialStimuli(annot, potentialStimuli, out, tier_syll, tier_tokmin);
            minimumStimulusLength -= 2.0;
        }
    }
    qDeleteAll(tiersAll);
}

void PBExpe::potentialStimuliFromCorpus(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications)
{
    QFile file("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/potential_stimuli.txt");
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    foreach (QPointer<CorpusCommunication> com, communications) {
        if (!com) continue;
        foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
            if (!annot) continue;
            potentialStimuliFromSample(corpus, annot, out);
        }
    }
    file.close();
}

// ====================================================================================================================

bool PBExpe::resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus)
{
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);
    QString stimulusID;
    long t0 = 0, t1 = 0;
    RealTime timeKeyDown, timeKeyUp;
    QList<Interval *> eventIntervals;
    int state = 0; // 0 = start, 1 = in tapping, 2 = seen key down (key up > 1, ends > 0)
    do {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("<tapping ")) {
            QList<QString> fields = line.split("\"");
            stimulusID = QString(fields.at(1)).replace("stimuli/", "").replace(".wav", "");
            t0 = fields.at(3).toLong();
            state = 1;
        }
        else if (line.startsWith("<key_down ") && state == 1) {
            QList<QString> fields = line.split("\"");
            long time = fields.at(3).toLong();
            timeKeyDown = RealTime::fromMilliseconds(time);
            state = 2;
        }
        else if (line.startsWith("<key_up ") && state == 2) {
            QList<QString> fields = line.split("\"");
            long time = fields.at(3).toLong();
            timeKeyUp = RealTime::fromMilliseconds(time);
            // add interval
            eventIntervals << new Interval(timeKeyDown, timeKeyUp, "x");
            state = 1;
        }
        else if (line.startsWith("<ends")) {
            QList<QString> fields = line.split("\"");
            t1 = fields.at(3).toLong();
            if (state == 2) { // pending key down
                timeKeyUp = RealTime::fromMilliseconds(t1 - t0);
                eventIntervals << new Interval(timeKeyDown, timeKeyUp, "x");
            }
            state = 0;
            // add to database
            IntervalTier *tier = new IntervalTier("tapping", eventIntervals, RealTime(0, 0), RealTime::fromMilliseconds(t1 - t0));
            corpus->datastoreAnnotations()->saveTier(stimulusID, subjectID, tier);
            delete tier;
            eventIntervals.clear();
        }
    } while (!stream.atEnd());
    file.close();
    return true;
}

bool PBExpe::resultsReadParticipantInfo(const QString &subjectID, const QString &expeType,
                                        const QString &filename, Corpus *corpus)
{
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);
    QPointer<CorpusSpeaker> spk;
    do {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("<participant_id ")) {
            // 0                     1 2     3 4    5 6             7 8       9
            // <participant_id prog="0" sex="1" L1="0" hearingprob="1" music="1" />
            QStringList fields = line.split("\"");
            QString programme = (fields.at(1) == "0") ? "FIAL" : "PSP";
            QString sex = (fields.at(3) == "0") ? "M" : "F";
            QString L1 = (fields.at(5) == "0") ? "FR" : "other";
            bool hearingProb = (fields.at(7) == "0") ? true : false;
            bool music = (fields.at(9) == "0") ? true : false;
            spk = corpus->speaker(subjectID);
            if (spk) {
                spk->setName(spk->ID());
                spk->setProperty("sex", sex);
                spk->setProperty("isExperimentSubject", true);
                spk->setProperty("experimentType", expeType);
                spk->setProperty("studyProgramme", programme);
                spk->setProperty("languageL1", L1);
                spk->setProperty("hasKnownHearingProblems", hearingProb);
                spk->setProperty("hasMusicalTraining", music);
            }
        }
    } while (!stream.atEnd());
    file.close();
    return true;
}

// ====================================================================================================================

void PBExpe::analysisCalculateDeltaRT(Corpus *corpus)
{
    if (!corpus) return;
    QList<QList<RealTime> > beeps;
    QList<RealTime> beeps01, beeps02;
    beeps01 << RealTime::fromSeconds(0.982267573)
            << RealTime::fromSeconds(4.059274376)
            << RealTime::fromSeconds(11.925895691)
            << RealTime::fromSeconds(21.612879818)
            << RealTime::fromSeconds(22.501360544)
            << RealTime::fromSeconds(32.0);
    beeps02 << RealTime::fromSeconds(0.989750566)
            << RealTime::fromSeconds(10.675374149)
            << RealTime::fromSeconds(12.062766439)
            << RealTime::fromSeconds(15.136507936)
            << RealTime::fromSeconds(22.500136054)
            << RealTime::fromSeconds(32.0);
    beeps << beeps01 << beeps02;
    foreach (CorpusSpeaker *spk, corpus->speakers()) {
        if (!spk->property("isExperimentSubject").toBool()) continue;
        QList<RealValueList > dRTs;
        RealValueList dRTsAll;
        dRTs << RealValueList() << RealValueList();
        for (int k = 1; k <= 2; ++k) {
            QString annotationID = QString("beeps0%1").arg(k);
            AnnotationTierGroup *tiers = corpus->datastoreAnnotations()->getTiers(annotationID, spk->ID());
            if (!tiers) { qDebug() << "no beeps annotation" << annotationID << spk->ID(); continue; }
            IntervalTier *tier_tap = tiers->getIntervalTierByName("tapping");
            if (!tier_tap) { qDebug() << "no tier tapping"; continue; }
            QList<RealTime> tap_down;
            foreach (Interval *intv, tier_tap->intervals()) {
                if (intv->text() == "x") tap_down << intv->tMin();
            }
            for (int i = 0; i < 5; ++i) {
                RealTime beep = beeps.at(k - 1).at(i);
                RealTime nextBeep = beeps.at(k - 1).at(i + 1);
                bool found = false;
                foreach (RealTime t, tap_down) {
                    if ((!found) && (t > beep) && (t < nextBeep)) {
                        RealTime dRT = t - beep;
                        qDebug() << spk->ID() << " " << beep.toDouble() << " "  << t.toDouble();
                        dRTs[k-1].append(dRT.toDouble());
                        dRTsAll.append(dRT.toDouble());
                        found = true;
                    }
                }
            }
            spk->setProperty(QString("drt_avg_0%1").arg(k), dRTs[k-1].mean());
            spk->setProperty(QString("drt_sd_0%1").arg(k), dRTs[k-1].stddev());
            dRTs[k-1].clear();
            delete tiers;
        }
        spk->setProperty("drt_avg", dRTsAll.mean());
        spk->setProperty("drt_sd", dRTsAll.stddev());
        dRTsAll.clear();
    }
}

void PBExpe::analysisCreateAdjustedTappingTier(Corpus *corpus)
{
    if (!corpus) return;
    foreach (CorpusCommunication *com, corpus->communications()) {
        foreach (CorpusSpeaker *spk, corpus->speakers()) {
            if (!spk->property("isExperimentSubject").toBool()) continue;
            AnnotationTierGroup *tiers = corpus->datastoreAnnotations()->getTiers(com->ID(), spk->ID());
            if (!tiers) { qDebug() << "no beeps annotation" << com->ID() << spk->ID(); continue; }
            IntervalTier *tier_tap = tiers->getIntervalTierByName("tapping");
            if (!tier_tap) { qDebug() << "no tier tapping"; continue; }
            IntervalTier *tier_tapAdj = new IntervalTier(tier_tap, "tappingAdj");
            RealTime delta = RealTime::fromSeconds(spk->property("drt_avg").toDouble());
            tier_tapAdj->timeShift(-delta);
            corpus->datastoreAnnotations()->saveTier(com->ID(), spk->ID(), tier_tapAdj);
        }
    }
}

void PBExpe::analysisCalculateSmoothedTappingModel(Corpus *corpus)
{
    if (!corpus) return;
    foreach (CorpusCommunication *com, corpus->communications()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiers = corpus->datastoreAnnotations()->getTiersAllSpeakers(com->ID());

        // Sparse one-dimensional model of key-down events, adjusted for subject RT
        sv_samplerate_t sampleRate = 16000;
        SparseOneDimensionalModel *modelInstants = new SparseOneDimensionalModel(sampleRate, 1);
        RealTime tMax;
        int totalAnnotators = 0;
        foreach (QString subjectID, tiers.keys()) {
            QPointer<AnnotationTierGroup> tiersSubj = tiers.value(subjectID);
            IntervalTier *tier_tapping = tiersSubj->getIntervalTierByName("tappingAdj");
            if (!tier_tapping) continue;
            if (tier_tapping->tMax() > tMax) tMax = tier_tapping->tMax();
            totalAnnotators++;
            foreach(Interval *intv, tier_tapping->intervals()) {
                if (intv->text() != "x") continue;
                SparseOneDimensionalModel::Point point(RealTime::realTime2Frame(intv->tMin(), sampleRate),
                                                       subjectID);
                modelInstants->addPoint(point);
            }
        }

        // Smoothed time-value model, calculated using a sliding window / also saved as a point-tier
        QList<Point *> smoothPoints;

        RealTime step = RealTime::fromMilliseconds(25);
        RealTime slidingWindowLeft = RealTime::fromMilliseconds(250);
        RealTime slidingWindowRight = RealTime::fromMilliseconds(250);
        RealTime groupingWindowLeft = RealTime::fromMilliseconds(500);
        RealTime groupingWindowRight = RealTime::fromMilliseconds(500);
        RealTime groupingDistanceLimit = RealTime::fromMilliseconds(300);

        // convert time values to frames in current resolution
        sv_frame_t stepF = RealTime::realTime2Frame(step, sampleRate);
        sv_frame_t slidingWindowLeftF = RealTime::realTime2Frame(slidingWindowLeft, sampleRate);
        sv_frame_t slidingWindowRightF = RealTime::realTime2Frame(slidingWindowRight, sampleRate);
        sv_frame_t groupingWindowLeftF = RealTime::realTime2Frame(groupingWindowLeft, sampleRate);
        sv_frame_t groupingWindowRightF = RealTime::realTime2Frame(groupingWindowRight, sampleRate);
        sv_frame_t groupingDistanceLimitF = RealTime::realTime2Frame(groupingDistanceLimit, sampleRate);

        for (sv_frame_t frame = 0; frame <= RealTime::realTime2Frame(tMax, sampleRate); frame += stepF) {
            // outermost limits
            sv_frame_t groupingFrame0 = frame - groupingWindowLeftF;
            sv_frame_t groupingFrame1 = frame + groupingWindowRightF;
            // 1. Select group of PPBs at a specific point
            SparseOneDimensionalModel::PointList PPBs;
            sv_frame_t previousFrame;
            // IMPORTANT NOTE: getPoints is not precise (may return +/-2 points before and after the interval passed to
            // the function - we have to check the returned values! (cf. Sonic Visualiser documentation)
            // Left side
            SparseOneDimensionalModel::PointList leftPPBs = modelInstants->getPoints(groupingFrame0, frame);
            previousFrame = frame;
            for (SparseOneDimensionalModel::PointList::const_iterator i = leftPPBs.end(); i != leftPPBs.begin(); --i) {
                const SparseOneDimensionalModel::Point &p(*i);
                if (p.frame > frame) continue;
                if (p.frame < groupingFrame0) break;
                if (previousFrame -  p.frame > groupingDistanceLimitF) break;
                PPBs.insert(p);
                previousFrame = p.frame;
            }
            // Right side
            SparseOneDimensionalModel::PointList rightPPBs = modelInstants->getPoints(frame, groupingFrame1);
            previousFrame = frame;
            for (SparseOneDimensionalModel::PointList::const_iterator i = rightPPBs.begin(); i != rightPPBs.end(); ++i) {
                const SparseOneDimensionalModel::Point &p(*i);
                if (p.frame <= frame) continue;
                if (p.frame > groupingFrame1) break;
                if (p.frame - previousFrame > groupingDistanceLimitF) break;
                PPBs.insert(p);
                previousFrame = p.frame;
            }
            // Calculations based on group
            QStringList groupSubjectsHavingAnnotated;
            QStringList groupTimesAdj;
            QStringList groupTimesOrig;
            RealTime groupFirstPPB, groupLastPPB;
            bool isFirst = true;
            for (SparseOneDimensionalModel::PointList::const_iterator i = PPBs.begin(); i != PPBs.end(); ++i) {
                const SparseOneDimensionalModel::Point &p(*i);
                RealTime t = RealTime::frame2RealTime(p.frame, sampleRate);
                if (isFirst) { groupFirstPPB = t; isFirst = false; }
                if (!groupSubjectsHavingAnnotated.contains(p.label)) {
                    groupSubjectsHavingAnnotated << p.label;
                    groupTimesAdj << QString("%1").arg(t.toDouble());
                    CorpusSpeaker *spk = corpus->speaker(p.label);
                    RealTime orig = t + RealTime::fromSeconds(spk->property("drt_avg").toDouble());
                    groupTimesOrig << QString("%1").arg(orig.toDouble());
                    if (groupLastPPB < t) groupLastPPB = t;
                }
            }

            // 2. Calculate continuous strength using a sliding window
            sv_frame_t slidingFrame0 = frame - slidingWindowLeftF;
            sv_frame_t slidingFrame1 = frame + slidingWindowRightF;
            int slidingCount = 0;
            QStringList slidingSubjectsHavingAnnotated;
            SparseOneDimensionalModel::PointList windowPPBs = modelInstants->getPoints(slidingFrame0, slidingFrame1);
            for (SparseOneDimensionalModel::PointList::const_iterator i = windowPPBs.begin(); i != windowPPBs.end(); ++i) {
                const SparseOneDimensionalModel::Point &p(*i);
                if (p.frame < slidingFrame0) continue;
                if (p.frame > slidingFrame1) continue;
                if (!slidingSubjectsHavingAnnotated.contains(p.label)) {
                    slidingSubjectsHavingAnnotated << p.label;
                    slidingCount++;
                }
            }
            // 3. Update values
            com->setProperty("totalAnnotators", totalAnnotators);
            double force = ((double)slidingCount) / ((double)totalAnnotators);
            Point *sp = new Point(RealTime::frame2RealTime(frame, sampleRate), groupSubjectsHavingAnnotated.join(","));
            sp->setAttribute("force", force);
            sp->setAttribute("firstPPB", groupFirstPPB.toDouble());
            sp->setAttribute("lastPPB", groupLastPPB.toDouble());
            sp->setAttribute("subjects", groupSubjectsHavingAnnotated.join("|"));
            sp->setAttribute("timesOrig", groupTimesOrig.join("|"));
            sp->setAttribute("timesAdj", groupTimesAdj.join("|"));
            smoothPoints << sp;
        }
        PointTier *tier_smooth = new PointTier("smooth", smoothPoints, RealTime(0,0), tMax);

        // Calculate local maxima
        QList<Point *> maxima;
        foreach (Point *p, tier_smooth->findLocalMaxima(RealTime::fromMilliseconds(250), "force")) {
            double force = p->attribute("force").toDouble();
            if (force < 0.2) continue;
            maxima << p;
        }
        QList<Point *> filtered;
        int i = 0;
        QList<Point *> group;
        while (i < maxima.count()) {
            group << maxima.at(i);
            if ((i == maxima.count() - 1) || (maxima.at(i+1)->time() - maxima.at(i)->time() > RealTime::fromMilliseconds(250))) {
                int j = group.count() / 2;
                filtered << group.at(j);
                group.clear();
            } else {
                group << maxima.at(i);
            }
            ++i;
        }
        foreach (Point *p, filtered) {
            p->setAttribute("localMax", true);
        }

        corpus->datastoreAnnotations()->saveTier(com->ID(), "smooth", tier_smooth);
        qDebug() << com->ID();
    }
    corpus->save(); // make sure you save the total annotators data!
}

struct PerceivedBoundary {
    QString subject;
    RealTime timeAdj;
    RealTime timeOrig;
};

QList<PerceivedBoundary> groupFromLists(QStringList &subjects, QStringList &timesAdj, QStringList &timesOrig)
{
    QList<PerceivedBoundary> list;
    if (subjects.count() != timesAdj.count() || timesAdj.count() != timesOrig.count()) {
        qDebug() << "bad group!";
        return list;
    }
    for (int i = 0; i < subjects.count(); ++i) {
        PerceivedBoundary ppb;
        ppb.subject = subjects.at(i);
        ppb.timeAdj = RealTime::fromSeconds(timesAdj.at(i).toDouble());
        ppb.timeOrig = RealTime::fromSeconds(timesOrig.at(i).toDouble());
        list << ppb;
    }
    return list;
}

QList<PerceivedBoundary> groupFromSP(Point *sp)
{
    QStringList subjects = sp->attribute("subjects").toString().split("|");
    QStringList timesAdj = sp->attribute("timesAdj").toString().split("|");
    QStringList timesOrig = sp->attribute("timesOrig").toString().split("|");
    return groupFromLists(subjects, timesAdj, timesOrig);
}

QList<PerceivedBoundary> groupFromSyll(Interval *syll)
{
    QStringList subjects = syll->attribute("boundarySubjects").toString().split("|");
    QStringList timesAdj = syll->attribute("boundaryTimesAdj").toString().split("|");
    QStringList timesOrig = syll->attribute("boundaryTimesOrig").toString().split("|");
    return groupFromLists(subjects, timesAdj, timesOrig);
}

void PBExpe::analysisAttributeTappingToSyllablesLocalMaxima(Corpus *corpus)
{
    if (!corpus) return;
    foreach (CorpusCommunication *com, corpus->communications()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiers = corpus->datastoreAnnotations()->getTiersAllSpeakers(com->ID());
        QPointer<AnnotationTierGroup> tiersSmooth = tiers.value("smooth");
        if (!tiersSmooth) continue;
        PointTier *tier_smooth = tiersSmooth->getPointTierByName("smooth");
        if (!tier_smooth) continue;
        foreach (QString speakerID, tiers.keys()) {
            QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
            IntervalTier *tier_syll = tiersSpk->getIntervalTierByName("syll");
            if (!tier_syll) continue;

            // Clean up PPB data in syllables
            foreach (Interval *syll, tier_syll->intervals()) {
                syll->setAttribute("boundaryDelay", 0.0);
                syll->setAttribute("boundaryDispersion", 0.0);
                syll->setAttribute("boundaryForce", 0.0);
                syll->setAttribute("boundaryFirstPPB", 0.0);
                syll->setAttribute("boundaryLastPPB", 0.0);
                syll->setAttribute("boundarySubjects", "");
                syll->setAttribute("boundaryTimesAdj", "");
                syll->setAttribute("boundaryTimesOrig", "");
            }

            if (tier_syll->countItems() < 10) {
                corpus->datastoreAnnotations()->saveTier(com->ID(), speakerID, tier_syll);
                continue; // (secondary speakers, not analysed)
            }

            IntervalTier *tier_tok_min = tiersSpk->getIntervalTierByName("tok_min");
            if (!tier_tok_min) continue;

            // For each syllable, mark which is the corresponding "last syll of word"
            QList<int> lastSyllables;
            int last_syll = -1;
            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                if (tok_min->isPauseSilent()) {
                    lastSyllables << ((last_syll < 0) ? 0 : last_syll);
                    last_syll++;
                } else {
                    int prev_last_syll = last_syll;
                    last_syll = tier_syll->intervalIndexAtTime(tok_min->tMax() - RealTime(0, 10));
                    for (int i = prev_last_syll; i < last_syll; ++i) lastSyllables << last_syll;
                }
            }
            if (lastSyllables.count() != tier_syll->count()) {
                continue; // and investiage
            }

            // Go through local maxima and attribute PPB groups to syllables
            QHash<int, QList<int> > assignments;
            for (int spIndex = 0; spIndex < tier_smooth->countItems(); ++spIndex) {
                Point *sp = tier_smooth->point(spIndex);
                if (sp->attribute("localMax").toBool()) {
                    int target_syll;
                    if (sp->time() > tier_syll->tMax())
                        target_syll = tier_syll->countItems() - 1;
                    else
                        target_syll = tier_syll->intervalIndexAtTime(sp->time());
                    if (target_syll < 0) {
                        qDebug() << "syll not found" << com->ID() << " at " << sp->time().toDouble();
                        continue;
                    }
                    // move to corresponding last syllable
                    target_syll = lastSyllables.at(target_syll);

                    if (assignments.contains(target_syll)) {
                        QList<int> list = assignments.value(target_syll);
                        list.append(spIndex);
                        assignments[target_syll] = list;
                    } else {
                        assignments[target_syll] << spIndex;
                    }
                }
            }

            foreach (int syllIndex, assignments.keys()) {
                QList<int> groups = assignments.value(syllIndex);
                QMap<RealTime, PerceivedBoundary> merged;
                foreach (int spIndex, groups) {
                    QList<PerceivedBoundary> group = groupFromSP(tier_smooth->point(spIndex));
                    foreach (PerceivedBoundary PPB, group) {
                        if (!merged.contains(PPB.timeAdj)) {
                            merged.insert(PPB.timeAdj, PPB);
                        }
                    }
                }
                QStringList subjects, timesAdj, timesOrig;
                foreach (PerceivedBoundary PPB, merged) {
                    subjects << PPB.subject;
                    timesAdj << QString("%1").arg(PPB.timeAdj.toDouble());
                    timesOrig << QString("%1").arg(PPB.timeOrig.toDouble());
                }
                int totalAnnotators = com->property("totalAnnotators").toInt();
                double force = ((double)(merged.count())) / ((double)totalAnnotators);
                Interval *syll = tier_syll->interval(syllIndex);

                if (force > 1.0) {
                    qDebug() << com->ID() << " " << force;
                    continue;
                }

                syll->setAttribute("boundaryForce", force);
                syll->setAttribute("boundaryFirstPPB", merged.first().timeAdj.toDouble());
                syll->setAttribute("boundaryLastPPB", merged.last().timeAdj.toDouble());
                syll->setAttribute("boundarySubjects", subjects.join("|"));
                syll->setAttribute("boundaryTimesAdj", timesAdj.join("|"));
                syll->setAttribute("boundaryTimesOrig", timesOrig.join("|"));
            }
            corpus->datastoreAnnotations()->saveTier(com->ID(), speakerID, tier_syll);
        }
        qDeleteAll(tiers);
        qDebug() << com->ID();
    }
}

void PBExpe::analysisCalculateAverageDelay(Corpus *corpus)
{
    if (!corpus) return;
    QStringList vowels;
    vowels << "a" << "e" << "i" << "o" << "u" << "E" << "O" << "2" << "9" << "y" << "a~" << "e~" << "o~" << "2~" << "9~" << "@" ;
    foreach (CorpusCommunication *com, corpus->communications()) {
        QMap<QString, QPointer<AnnotationTierGroup> > tiers = corpus->datastoreAnnotations()->getTiersAllSpeakers(com->ID());
        foreach (QString speakerID, tiers.keys()) {
            QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
            IntervalTier *tier_syll = tiersSpk->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            IntervalTier *tier_phone = tiersSpk->getIntervalTierByName("phone");
            if (!tier_phone) continue;

            foreach (Interval *syll, tier_syll->intervals()) {
                if (syll->attribute("boundaryForce").toDouble() == 0.0) continue;
                QList<PerceivedBoundary> PPBs = groupFromSyll(syll);
                if (PPBs.isEmpty()) continue;
                // Find syll nucleus / center
                RealTime t0;
                QList<Interval *> phones = tier_phone->getIntervalsContainedIn(syll);
                foreach (Interval *phone, phones) {
                    if (vowels.contains(phone->text())) {
                        t0 = phone->tMin();
                    }
                }
                if (t0 == RealTime()) {
                    t0 = syll->tCenter();
                }
                // Now calculate average delay and dispersion
                RealValueList responseTimes;
                foreach (PerceivedBoundary PPB, PPBs) {
                    responseTimes << (PPB.timeOrig - t0).toDouble();
                }
                syll->setAttribute("boundaryDelay", responseTimes.mean());
                syll->setAttribute("boundaryDispersion", responseTimes.stddev());
            }

            corpus->datastoreAnnotations()->saveTier(com->ID(), speakerID, tier_syll);
        }
        qDeleteAll(tiers);
        qDebug() << com->ID();
    }
}

void PBExpe::analysisCalculateCoverage(Corpus *corpus)
{
    if (!corpus) return;
    QHash<QString, int> AllPPBs;
    foreach (CorpusCommunication *com, corpus->communications()) {
        QString id = com->ID();
        if (!id.startsWith("A") && !id.startsWith("B")) continue;

        QMap<QString, QPointer<AnnotationTierGroup> > tiers = corpus->datastoreAnnotations()->getTiersAllSpeakers(com->ID());
        foreach (QString speakerID, tiers.keys()) {
            QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
            IntervalTier *tier_syll = tiersSpk->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            foreach (Interval *syll, tier_syll->intervals()) {
                if (syll->attribute("boundaryForce").toDouble() == 0.0) continue;
                QList<PerceivedBoundary> PPBs = groupFromSyll(syll);
                foreach (PerceivedBoundary PPB, PPBs) {
                    QString hash = QString("%1_%2_%3").arg(com->ID()).arg(PPB.subject).arg(PPB.timeOrig.toDouble());
                    if (AllPPBs.contains(hash))
                        AllPPBs[hash] = AllPPBs[hash] + 1;
                    else
                        AllPPBs.insert(hash, 1);
                }
            }
        }
        qDeleteAll(tiers);
        qDebug() << com->ID();
    }
    qDebug() << AllPPBs.keys().count();
}

void PBExpe::analysisFeaturesForModelling(Corpus *corpus)
{
    if (!corpus) return;
    QStringList ppbAttributeIDs;
    ppbAttributeIDs << "boundaryDelay" << "boundaryDispersion" << "boundaryForce" <<
                       "boundaryFirstPPB" << "boundaryLastPPB";
    // "boundarySubjects" << "boundaryTimesAdj" << "boundaryTimesOrig"

    QFile file("D:/DROPBOX/2015-10_SP8_ProsodicBoundariesExpe/features.txt");
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "stimulusID\tstimulusType\tspeaker\tsyll_ID\tsyll_tmin\tsyll\t"
           "expertBoundaryType\texpertContour\texpertBoundary\t"
           "durNextPause\tlogdurNextPause\tlogdurNextPauseZ\t"
           "durSyllRel20\tdurSyllRel30\tdurSyllRel40\tdurSyllRel50\t"
           "logdurSyllRel20\tlogdurSyllRel30\tlogdurSyllRel40\tlogdurSyllRel50\t"
           "f0meanSyllRel20\tf0meanSyllRel30\tf0meanSyllRel40\tf0meanSyllRel50\t"
           "intrasyllab_up\tintrasyllab_down\ttrajectory\t"
           "tok_mwu\tsequence\trection\tsyntacticBoundaryType\tpos_mwu\tpos_mwu_cat\tpos_clilex\t"
           "boundaryDelay\tboundaryDispersion\tboundaryForce\tboundaryFirstPPB\tboundaryLastPPB\n";
    foreach (CorpusCommunication *com, corpus->communications()) {
        QString id = com->ID();
        if (!id.startsWith("A") && !id.startsWith("B")) continue;

        QMap<QString, QPointer<AnnotationTierGroup> > tiers = corpus->datastoreAnnotations()->getTiersAllSpeakers(com->ID());
        foreach (QString speakerID, tiers.keys()) {
            QPointer<AnnotationTierGroup> tiersSpk = tiers.value(speakerID);
            IntervalTier *tier_syll = tiersSpk->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            QList<int> ppbSyllables;
            for (int i = 0; i < tier_syll->countItems(); ++i) {
                Interval *syll = tier_syll->interval(i);
                if (syll->attribute("boundaryForce").toDouble() == 0.0) continue;
                ppbSyllables << i;
            }
            ProsodicBoundaries::analyseBoundaryList(out, corpus, com->ID(), ppbSyllables, ppbAttributeIDs);
        }
        qDeleteAll(tiers);
        qDebug() << com->ID();
    }

    file.close();
}


