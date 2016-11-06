#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include "annotation/AnnotationTier.h"
#include "annotation/PointTier.h"
#include "annotation/Point.h"
#include "PraatTextFile.h"
#include "PraatPointTierFile.h"

namespace Praaline {
namespace Core {

bool PraatPointTierFile::readPoint(bool isShortFile, QTextStream &stream, QString &line,
                                   double &timePoint, double &value)
{
    QRegularExpression regex_point("points\\s+\\[\\d*\\]:");
    QRegularExpression regex_time((isShortFile) ? "(\\d+\\.?\\d*)" : "\\s+=\\s+(\\d+\\.?\\d*)");
    QRegularExpression regex_value((isShortFile) ? "(\\d+\\.?\\d*)" : "value\\s+=\\s+(\\d+\\.?\\d*)");

    QString result;
    if (isShortFile || (regex_point.match(line).hasMatch())) {
        result = seekPattern(stream, regex_time);
        timePoint = result.toDouble();
        result = seekPattern(stream, regex_value);
        value = result.toDouble();
        return true;
    }
    else
        return false;
}

// ------------------------------

// Public static functions to load and save point tier files

bool PraatPointTierFile::load(const QString &filename, QMap<RealTime, double> &points)
{

    QFile file(filename);
    QString line;
    double timePoint_d, value;
    RealTime xMin, xMax, timePoint;

    // Open file, create stream, detect encoding
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QTextStream stream(&file);
    detectEncoding(file, stream);

    // Find file type (normal or short)
    bool isShortFile = false;
    line = stream.readLine();
    if (line.contains("ooTextFile short"))
        isShortFile = true;
    if (stream.atEnd()) return false; // extra safey for incorrect files

    do {
        line = stream.readLine();
        // Reset values
        xMin = xMax = timePoint = RealTime(0, 0);
        // File processing loop
        if (readPoint(isShortFile, stream, line, timePoint_d, value)) {
            timePoint = RealTime::fromSeconds(timePoint_d);
            points.insert(timePoint, value);
        }
    } while (!stream.atEnd());

    file.close();
    return true;
}

bool PraatPointTierFile::save(const QString &filename, const QString &praatObjectClass,
                              QMap<RealTime, double> &points)
{
    QFile file(filename);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    // write PitchTier header
    out << QString("File type = \"ooTextFile\"\nObject class = \"%1\"\n\n")
           .arg(praatObjectClass);
    out << QString("xmin = %1\nxmax = %2\n")
           .arg(QString::number(points.keys().first().toDouble(), 'f', 16))
           .arg(QString::number(points.keys().last().toDouble(), 'f', 16));

    out << QString("points: size = %1\n").arg(points.count());
    int i = 0;
    foreach (RealTime time, points.keys()) {
        out << QString("points [%1]:\n").arg(++i);
        out << QString("\tnumber = %1\n")
               .arg(QString::number(time.toDouble(), 'f', 16));
        out << QString("\tvalue = %1\n")
               .arg(QString::number(points.value(time), 'f', 16));
    }

    file.close();
    return true;
}

} // namespace Core
} // namespace Praaline
