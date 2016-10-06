#include <QDebug>
#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/base/RealTime.h"
#include "pncore/base/RealValueList.h"
#include "pncore/annotation/interval.h"
#include "pncore/annotation/intervaltier.h"
#include "pncore/corpus/corpus.h"

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


void DisfluenciesExperiments::analysisCalculateDeltaRT(Corpus *corpus)
{
    int numberOfBeepFiles = 14; int numberOfBeepsInFile = 6;
    if (!corpus) return;
    QList<QList<RealTime> > beeps;
    for (int k = 1; k <= numberOfBeepFiles; ++k) {
        QString beepsID("beeps"); if (k < 10) beepsID.append("0"); beepsID.append(QString("%1").arg(k));
        IntervalTier *tier_beeps = qobject_cast<IntervalTier*>(corpus->datastoreAnnotations()->getTier(beepsID, beepsID, "beeps"));
        if (!tier_beeps) continue;
        beeps << tier_beeps->times();
    }

    foreach (CorpusSpeaker *spk, corpus->speakers()) {
        if (!spk->property("isExperimentSubject").toBool()) continue;
        QList<RealValueList > dRTs;
        RealValueList dRTsAll;
        dRTs << RealValueList() << RealValueList();
        for (int k = 1; k <= numberOfBeepFiles; ++k) {
            QString beepsID("beeps"); if (k < 10) beepsID.append("0"); beepsID.append(QString("%1").arg(k));

            AnnotationTierGroup *tiers = corpus->datastoreAnnotations()->getTiers(beepsID, spk->ID());
            if (!tiers) { qDebug() << "no beeps annotation" << beepsID << spk->ID(); continue; }
            IntervalTier *tier_tap = tiers->getIntervalTierByName("tapping");
            if (!tier_tap) { qDebug() << "no tier tapping" << beepsID << spk->ID(); continue; }
            QList<RealTime> tap_down;
            foreach (Interval *intv, tier_tap->intervals()) {
                if (intv->text() == "x") tap_down << intv->tMin();
            }
            for (int i = 0; i < numberOfBeepsInFile; ++i) {
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
            spk->setProperty(QString("drt_avg_%1").arg(k), dRTs[k-1].mean());
            spk->setProperty(QString("drt_sd_%1").arg(k), dRTs[k-1].stddev());
            dRTs[k-1].clear();
            delete tiers;
        }
        spk->setProperty("drt_avg", dRTsAll.mean());
        spk->setProperty("drt_sd", dRTsAll.stddev());
        dRTsAll.clear();
    }
    corpus->save();
}


