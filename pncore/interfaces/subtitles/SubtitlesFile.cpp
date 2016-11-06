#include <QString>
#include <QFile>
#include "SubtitlesFile.h"

namespace Praaline {
namespace Core {

RealTime timeCodeToRealTime(const QString &timecode)
{
    QStringList parts = timecode.split(":");
    if (parts.count() != 3) return RealTime();
    int hours = parts[0].toInt();
    int minutes = parts[1].toInt();
    QStringList parts2 = parts[2].split(",");
    if (parts2.count() != 2) return RealTime();
    int seconds = parts2[0].toInt();
    int msec = parts2[1].toInt();
    return RealTime::fromSeconds(hours * 3600 + minutes * 60 + seconds) + RealTime::fromMilliseconds(msec);
}

bool SubtitlesFile::loadSRT(const QString &filename, IntervalTier *tier)
{
    if (!tier) return false;
    // Open file, create stream, detect encoding
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QTextStream stream(&file);
    detectEncoding(file, stream);

    QList<Interval *> intervals;
    RealTime tMin, tMax;
    QString text;
    int state = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (state == 0) {
            state = 1;
            tMin = tMax = RealTime();
            text.clear();
        }
        else if (state == 1) {
            // timing
            QString tStart = line.section("-->", 0, 0).trimmed();
            QString tEnd = line.section("-->", 1, 1).trimmed();
            tMin = timeCodeToRealTime(tStart);
            tMax = timeCodeToRealTime(tEnd);
            state = 2;
        }
        else if ((state == 2) && (!line.isEmpty())) {
            if (!text.isEmpty()) text.append(" ");
            text.append(line);
        }
        else {
            intervals << new Interval(tMin, tMax, text);
            state = 0;
        }
    }
    tier->replaceAllIntervals(intervals);
    return true;
}

bool SubtitlesFile::saveSRT(const QString &filename, IntervalTier *tier)
{
    return false;
}

} // namespace Core
} // namespace Praaline
