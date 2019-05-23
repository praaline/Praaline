#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>

#include "pncore/base/RealTime.h"
#include "pncore/annotation/interval.h"
#include "pncore/corpus/corpus.h"

#include "DisfluencyExperiments.h"

bool DisfluencyExperiments::resultsReadTapping(const QString &subjectID, const QString &filename, Corpus *corpus)
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

