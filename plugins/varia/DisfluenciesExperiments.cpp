#include <QDebug>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/base/RealTime.h"
#include "pncore/base/RealValueList.h"
#include "pncore/annotation/Interval.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/corpus/Corpus.h"

#include "DisfluenciesExperiments.h"

bool DisfluenciesExperiments::resultsReadTapping(const QString &sessionID, const QString &filename, Corpus *corpus)
{
    QString annotatorID, stimulusID;
    int keyCode;
    RealTime timeKeyPress, timeKeyRelease;
    QList<Interval *> eventIntervals;

    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly | QIODevice::Text )) return false;
    QTextStream stream(&file);

    int state = 0; // 0 = header, 1 = started, 1 = in tapping, 2 = seen key down (key up > 1, ends > 0)
    do {
        QString line = stream.readLine().trimmed();
        if (line.startsWith("SAMPLE")) {
            QList<QString> fields = line.split("\t");
            stimulusID = QString(fields.at(1)).replace("corpus/", "").replace(".wav", "");
            state = 0;
        }
        else if (line.startsWith("ANNOTATOR")) {
            annotatorID = line.split("\t").at(1);
            state = 0;
        }
        else if (line.startsWith("START")) {
            state = 1;
        }
        else if (line.startsWith("KEYPRS") && state == 1) {
            QList<QString> fields = line.split("\t");
            long time = fields.at(2).toLong();
            keyCode = fields.at(3).toInt();
            if (keyCode == 57) {
                timeKeyPress = RealTime::fromMilliseconds(time);
                state = 2;
            }
        }
        else if (line.startsWith("KEYREL") && state == 2) {
            QList<QString> fields = line.split("\t");
            long time = fields.at(2).toLong();
            keyCode = fields.at(3).toInt();
            if (keyCode == 57) {
                timeKeyRelease = RealTime::fromMilliseconds(time);
                // add interval
                eventIntervals << new Interval(timeKeyPress, timeKeyRelease, "x");
                state = 1;
            }
        }
        else if (line.startsWith("ENDS")) {
            state = 0;
            // add to database
            QPointer<IntervalTier> tier = new IntervalTier("tapping", eventIntervals);
            corpus->datastoreAnnotations()->saveTier(stimulusID, QString("%1_%2").arg(annotatorID).arg(sessionID), tier);
            eventIntervals.clear();
        }
    } while (!stream.atEnd());
    file.close();
    return true;
}

bool createRTAttribute(const QString &attributeID, const QString &attributeName,
                       Corpus *corpus, QPointer<MetadataStructureSection> section)
{
    if (!section->hasAttribute(attributeID)) {
        MetadataStructureAttribute *attr = new MetadataStructureAttribute(
                    attributeID, attributeName, "", DataType::Double);
        if (corpus->datastoreMetadata()->createMetadataAttribute(CorpusObject::Type_Speaker, attr)) {
            section->addAttribute(attr);
            return true;
        }
    }
    return false;
}

void createDeltaRTProperties(Corpus *corpus, int numberOfBeepFiles)
{
    if (!corpus) return;
    QPointer<MetadataStructureSection> section = corpus->metadataStructure()->section(CorpusObject::Type_Speaker, "speaker");
    if (!section) return;
    for (int i = 1; i <= numberOfBeepFiles; ++i) {
        createRTAttribute(QString("drt_avg_%1").arg(i), QString("Delta RT Mean %1").arg(i), corpus, section);
        createRTAttribute(QString("drt_sd_%1").arg(i), QString("Delta RT Std Dev %1").arg(i), corpus, section);
    }
    createRTAttribute("drt_avg", "Delta RT Mean", corpus, section);
    createRTAttribute("drt_sd", "Delta RT Std Dev", corpus, section);
    corpus->save();
}

void DisfluenciesExperiments::analysisCalculateDeltaRT(Corpus *corpus)
{
    int numberOfBeepFiles = 14; int numberOfBeepsInFile = 6;
    if (!corpus) return;
    createDeltaRTProperties(corpus, numberOfBeepFiles);

    QList<QList<RealTime> > beeps;
    for (int k = 1; k <= numberOfBeepFiles; ++k) {
        QString beepsID("beeps"); if (k < 10) beepsID.append("0"); beepsID.append(QString("%1").arg(k));
        IntervalTier *tier_beeps = qobject_cast<IntervalTier*>(corpus->datastoreAnnotations()->getTier(beepsID, beepsID, "beeps"));
        if (!tier_beeps) continue;
        QList<RealTime> beepTimes;
        foreach (Interval *intv, tier_beeps->intervals()) {
            if (intv->text() == "x") beepTimes << intv->tMin();
        }
        beeps << beepTimes;
    }

    foreach (CorpusSpeaker *spk, corpus->speakers()) {
        if (!spk->property("isExperimentSubject").toBool()) continue;
        QList<RealValueList > dRTs;
        RealValueList dRTsAll;
        for (int k = 1; k <= numberOfBeepFiles; ++k) {
            dRTs << RealValueList();
            QString beepsID("beeps"); if (k < 10) beepsID.append("0"); beepsID.append(QString("%1").arg(k));

            AnnotationTierGroup *tiers = corpus->datastoreAnnotations()->getTiers(beepsID, spk->ID());
            if (!tiers) { qDebug() << "no beeps annotation" << beepsID << spk->ID(); continue; }
            IntervalTier *tier_tap = tiers->getIntervalTierByName("tapping");
            if (!tier_tap) { qDebug() << "no tier tapping" << beepsID << spk->ID(); continue; }
            QList<RealTime> tap_down;
            foreach (Interval *intv, tier_tap->intervals()) {
                if (intv->text() == "x") tap_down << intv->tMin();
            }

            QString result = QString("%1\t%2").arg(k).arg(spk->ID());
            QList<RealTime> beepTimes = beeps.at(k - 1);
            result.append(QString::number(tap_down.count())).append("\t").append(QString::number(beepTimes.count())).append("\t");

            for (int i = 0; i < numberOfBeepsInFile; ++i) {
                RealTime beep = beepTimes.at(i);
                RealTime nextBeep = (i + 1 < beepTimes.count()) ? beepTimes.at(i + 1) :
                                                                  tier_tap->tMax();
                bool found = false;
                foreach (RealTime t, tap_down) {
                    if ((!found) && (t > beep) && (t < nextBeep)) {
                        RealTime dRT = t - beep;
                        result.append(QString::number(beep.toDouble())).append("\t").append(QString::number(t.toDouble())).append("\t");
                        dRTs[k-1].append(dRT.toDouble());
                        dRTsAll.append(dRT.toDouble());
                        found = true;
                    }
                }
            }
            spk->setProperty(QString("drt_avg_%1").arg(k), dRTs[k-1].mean());
            spk->setProperty(QString("drt_sd_%1").arg(k), dRTs[k-1].stddev());
            dRTs[k-1].clear();
            delete tiers;

            qDebug() << result;
        }
        spk->setProperty("drt_avg", dRTsAll.mean());
        spk->setProperty("drt_sd", dRTsAll.stddev());
        dRTsAll.clear();
    }
    corpus->save();
}

void DisfluenciesExperiments::analysisCreateAdjustedTappingTier(Corpus *corpus)
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
            // Get for speaker = RealTime::fromSeconds(spk->property("drt_avg").toDouble());
            // Calculate as a linear function: RT = a t + RT0 where a = -3.3839 and RT0=304.92
            double RTFactor = com->property("RTFactor").toDouble();
            if (spk->ID().endsWith("2")) RTFactor = RTFactor * 2.0;
            double RT = -3.3839 * RTFactor + 304.92;
            RealTime delta = RealTime::fromMilliseconds(RT);
            tier_tapAdj->timeShift(-delta);
            corpus->datastoreAnnotations()->saveTier(com->ID(), spk->ID(), tier_tapAdj);
        }
    }
    // Need to run the following SQL statements against the database after this procedure:
    // update tappingAdj set tMin = 0 where tMin < 0
    // update tappingAdj set tMax = 0 where tMax < 0
}

