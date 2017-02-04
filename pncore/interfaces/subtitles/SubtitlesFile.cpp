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

QString realTimeToTimeCode(const RealTime &realtime)
{
    if (realtime < RealTime::zeroTime) return "-" + realTimeToTimeCode(-realtime);
    QString timecode;
    if (realtime.sec >= 3600) {
        int hours = (realtime.sec / 3600);
        if (hours < 10) timecode = timecode.append("0");
        timecode = timecode.append(QString("%1:").arg(hours));
    }
    if (realtime.sec >= 60) {
        int minutes = (realtime.sec % 3600) / 60;
        if (minutes < 10) timecode = timecode.append("0");
        timecode =timecode.append(QString("%1:").arg(minutes));
    }
    if (realtime.sec >= 10) {
        timecode = timecode.append(QString("%1").arg((realtime.sec % 60) / 10));
    } else {
        timecode = timecode.append("0");
    }
    timecode = timecode.append(QString("%1").arg(realtime.sec % 10));
    timecode = timecode.append(QString(",%1").arg(realtime.msec()));
    return timecode;
}


bool SubtitlesFile::loadSRT(const QString &filename, IntervalTier *tier, const QString &attributeID)
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
            Interval *intv = new Interval(tMin, tMax, "");
            if (attributeID.isEmpty()) intv->setText(text); else intv->setAttribute(attributeID, text);
            intervals << intv;
            state = 0;
        }
    }
    tier->replaceAllIntervals(intervals);
    return true;
}

bool SubtitlesFile::saveSRT(const QString &filename, IntervalTier *tier, const QString &attributeID)
{
    if (!tier) return false;
    // Open file, create stream
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) return false;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    int subTitleNo = 1;
    foreach (Interval *intv, tier->intervals()) {
        if (intv->isPauseSilent()) continue;
        out << QString::number(subTitleNo) << "\n";
        out << realTimeToTimeCode(intv->tMin()) << " --> " << realTimeToTimeCode(intv->tMax()) << "\n";
        QString subtitle = (attributeID.isEmpty()) ? intv->text() : intv->attribute(attributeID).toString();
        out << subtitle << "\n";
        out << "\n";
    }

    file.close();
    return false;
}

} // namespace Core
} // namespace Praaline
