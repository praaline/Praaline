#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include "annotation/AnnotationTier.h"
#include "annotation/IntervalTier.h"
#include "annotation/PointTier.h"
#include "annotation/Interval.h"
#include "annotation/Point.h"
#include "PraatTextGrid.h"

namespace Praaline {
namespace Core {

// Initialise regular expressions
QRegularExpression PraatTextGrid::regex_interval_tier = QRegularExpression("class\\s+=\\s+\\\"IntervalTier\\\"");
QRegularExpression PraatTextGrid::regex_point_tier = QRegularExpression("class\\s+=\\s+\\\"TextTier\\\"");
QRegularExpression PraatTextGrid::regex_name = QRegularExpression("name\\s+=\\s+\\\"(.*)\\\"");
QRegularExpression PraatTextGrid::regex_size = QRegularExpression("size\\s+=\\s+(\\d*)");
QRegularExpression PraatTextGrid::regex_xmin = QRegularExpression("xmin\\s+=\\s+[+-]?(\\d+\\.?\\d*)");
QRegularExpression PraatTextGrid::regex_xmax = QRegularExpression("xmax\\s+=\\s+[+-]?(\\d+\\.?\\d*)");
QRegularExpression PraatTextGrid::regex_interval = QRegularExpression("intervals\\s+\\[\\d*\\]:");
QRegularExpression PraatTextGrid::regex_point = QRegularExpression("points\\s+\\[\\d*\\]:");
QRegularExpression PraatTextGrid::regex_text = QRegularExpression("text\\s+=\\s+\\\"(.*)\\\"");
QRegularExpression PraatTextGrid::regex_time = QRegularExpression("\\s+=\\s+[+-]?(\\d+\\.?\\d*)");
QRegularExpression PraatTextGrid::regex_mark = QRegularExpression("mark\\s+=\\s+\\\"(.*)\\\"");

QRegularExpression PraatTextGrid::regex_short_interval_tier = QRegularExpression("\\\"IntervalTier\\\"");
QRegularExpression PraatTextGrid::regex_short_point_tier = QRegularExpression("\\\"TextTier\\\"");
QRegularExpression PraatTextGrid::regex_short_name = QRegularExpression("\\\"(.*)\\\"");
QRegularExpression PraatTextGrid::regex_short_size = QRegularExpression("(\\d*)");
QRegularExpression PraatTextGrid::regex_short_xmin = QRegularExpression("[+-]?(\\d+\\.?\\d*)");
QRegularExpression PraatTextGrid::regex_short_xmax = QRegularExpression("[+-]?(\\d+\\.?\\d*)");
QRegularExpression PraatTextGrid::regex_short_text = QRegularExpression("\\\"(.*)\\\"");
QRegularExpression PraatTextGrid::regex_short_time = QRegularExpression("[+-]?(\\d+\\.?\\d*)");
QRegularExpression PraatTextGrid::regex_short_mark = QRegularExpression("\\\"(.*)\\\"");


// The following private, static member functions are used to process a TextGrid file

bool PraatTextGrid::readTierHeader(bool isShortFile, QTextStream &stream, QString &line,
                                   int &tierType, int &tierSize, QString &name, double &xMin, double &xMax)
{
    QRegularExpressionMatch match;
    QString result;
    tierType = -1;
    match = (isShortFile) ? regex_short_interval_tier.match(line) : regex_interval_tier.match(line);
    if (match.hasMatch())
        tierType = 1;
    else {
        match = (isShortFile) ? regex_short_point_tier.match(line) : regex_point_tier.match(line);
        if (match.hasMatch())
            tierType = 2;
    }
    if (tierType > 0) {
        name = seekPattern(stream, (isShortFile) ? regex_short_name : regex_name);
        result = seekPattern(stream, (isShortFile) ? regex_short_xmin : regex_xmin);
        xMin = result.toDouble();
        result = seekPattern(stream, (isShortFile) ? regex_short_xmax : regex_xmax);
        xMax = result.toDouble();
        result = seekPattern(stream, (isShortFile) ? regex_short_size : regex_size);
        tierSize = result.toInt();
        return true;
    }
    else
        return false;
}

bool PraatTextGrid::readInterval(bool isShortFile, QTextStream &stream, QString &line,
                                 double &xMin, double &xMax, QString &text)
{
    QString result;
    if (isShortFile || regex_interval.match(line).hasMatch()) {
        result = seekPattern(stream, (isShortFile) ? regex_short_xmin : regex_xmin);
        xMin = result.toDouble();
        result = seekPattern(stream, (isShortFile) ? regex_short_xmax : regex_xmax);
        xMax = result.toDouble();
        text = seekPattern(stream, (isShortFile) ? regex_short_text : regex_text);
        text.replace("\"\"", "\"");
        return true;
    }
    else
        return false;
}

bool PraatTextGrid::readPoint(bool isShortFile, QTextStream &stream, QString &line,
                              double &timePoint, QString &text)
{
    QString result;
    if (isShortFile || (regex_point.match(line).hasMatch())) {
        result = seekPattern(stream, (isShortFile) ? regex_short_time : regex_time);
        timePoint = result.toDouble();
        text = seekPattern(stream, (isShortFile) ? regex_short_mark : regex_mark);
        text.replace("\"\"", "\"");
        return true;
    }
    else
        return false;
}

// ------------------------------

// Public static functions to load and save textgrids into Annotation Groups

bool PraatTextGrid::load(const QString &filename, AnnotationTierGroup *group)
{
    if (group == 0) return false; // I can only load into AnnotationTierGroups

    QFile file(filename);
    QString line, text, tierName, new_tierName;
    double xMin_d, xMax_d, timePoint_d;
    RealTime xMin, xMax, timePoint;
    RealTime tierxMin, tierxMax;
    int tierSize = 0;
    int tierType = -1, new_tierType = -1;

    // Open file, create stream, detect encoding
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QTextStream stream(&file);
    detectEncoding(file, stream);

    QList<Interval *> intervals;
    QList<Point *> points;

    // Find file type (normal or short)
    bool isShortFile = false;
    line = stream.readLine();
    if (line.contains("ooTextFile short"))
        isShortFile = true;
    if (stream.atEnd()) return false; // extra safey for incorrect files

    if (isShortFile) {
        // Skip header information
        while (!stream.atEnd() && !line.contains("<exists>"))
            line = stream.readLine();
        line = stream.readLine(); // read next line after <exists>
        int numberOfTiers = line.toInt();
        // Read tiers
        for (int i = 0; i < numberOfTiers; i++) {
            line = stream.readLine(); // move to next line
            if (!readTierHeader(true, stream, line, tierType, tierSize, tierName, xMin_d, xMax_d))
                return false;
            tierxMin = RealTime::fromSeconds(xMin_d);
            tierxMax = RealTime::fromSeconds(xMax_d);
            if (tierType == 1) {
                for (int j = 0; j < tierSize; j++) {
                    if (!readInterval(true, stream, line, xMin_d, xMax_d, text))
                        return false;
                    xMin = RealTime::fromSeconds(xMin_d);
                    xMax = RealTime::fromSeconds(xMax_d);
                    intervals << new Interval(xMin, xMax, text);
                }
                group->addTier(new IntervalTier(tierName, intervals, tierxMin, tierxMax));
                intervals.clear();
            } else if (tierType == 2) {
                for (int j = 0; j < tierSize; j++) {
                    if (!readPoint(true, stream, line, timePoint_d, text))
                        return false;
                    timePoint = RealTime::fromSeconds(timePoint_d);
                    points << new Point(timePoint, text);
                }
                group->addTier(new PointTier(tierName, points, tierxMin, tierxMax));
                points.clear();
            }
        }
    } else {
        do {
            line = stream.readLine();
            // Reset values
            xMin = xMax = timePoint = RealTime(0, 0);
            text = "";
            // File processing loop
            if (readTierHeader(false, stream, line, new_tierType, tierSize, new_tierName, xMin_d, xMax_d)) {
                // Add previous tier first
                if (tierType == 1)
                    group->addTier(new IntervalTier(tierName, intervals, tierxMin, tierxMax));
                if (tierType == 2)
                    group->addTier(new PointTier(tierName, points, tierxMin, tierxMax));
                // Then register the new tier
                tierType = new_tierType;
                tierName = new_tierName;
                tierxMin = RealTime::fromSeconds(xMin_d);
                tierxMax = RealTime::fromSeconds(xMax_d);
                intervals.clear();
                points.clear();
            }
            else if (readInterval(false, stream, line, xMin_d, xMax_d, text)) {
                xMin = RealTime::fromSeconds(xMin_d);
                xMax = RealTime::fromSeconds(xMax_d);
                intervals << new Interval(xMin, xMax, text);
            }
            else if (readPoint(false, stream, line, timePoint_d, text)) {
                timePoint = RealTime::fromSeconds(timePoint_d);
                points << new Point(timePoint, text);
            }
        } while (!stream.atEnd());
        // Add the last tier (if any)
        if (tierType == 1)
            group->addTier(new IntervalTier(tierName, intervals, tierxMin, tierxMax));
        if (tierType == 2)
            group->addTier(new PointTier(tierName, points, tierxMin, tierxMax));
    }
    file.close();
    return true;
}

bool PraatTextGrid::save(const QString &filename, AnnotationTierGroup *group)
{
    if (group == 0) return false; // I can only save AnnotationTierGroups

    AnnotationTier *tier;
    IntervalTier *iTier;
    PointTier *pTier;
    Interval *interval;
    Point *point;

    QFile file(filename);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    // write TextGrid header
    out << "File type = \"ooTextFile\"\nObject class = \"TextGrid\"\n\n";
    out << QString("xmin = %1\nxmax = %2\n")
           .arg(QString::number(group->tMin().toDouble(), 'f', 16))
           .arg(QString::number(group->tMax().toDouble(), 'f', 16));

    int countTiers = 0;
    foreach (tier, group->tiers()) {
        if ((tier->tierType() == AnnotationTier::TierType_Intervals) || (tier->tierType() == AnnotationTier::TierType_Grouping) ||
            (tier->tierType() == AnnotationTier::TierType_Points))
            countTiers++;
    }
    out << QString("tiers? <exists>\nsize = %1\nitem []:\n").arg(countTiers);

    int n = 0;
    foreach (tier, group->tiers()) {
        // Save empty tiers as well! if (tier->isEmpty()) continue;

        QString tierType;
        if ((tier->tierType() == AnnotationTier::TierType_Intervals) || (tier->tierType() == AnnotationTier::TierType_Grouping))
            tierType = "IntervalTier";
        else if (tier->tierType() == AnnotationTier::TierType_Points)
            tierType = "TextTier";
        else
            continue; // unsupporterd tier type for textgrids

        // write tier header
        out << QString("\titem [%1]:\n").arg(++n);
        out << QString("\t\tclass = \"%1\"\n").arg(tierType);
        out << QString("\t\tname = \"%1\"\n").arg(tier->name());
        out << QString("\t\txmin = %1\n\t\txmax = %2\n")
               .arg(QString::number(tier->tMin().toDouble(), 'f', 16))
               .arg(QString::number(tier->tMax().toDouble(), 'f', 16));

        if ((tier->tierType() == AnnotationTier::TierType_Intervals) || (tier->tierType() == AnnotationTier::TierType_Grouping)) {
            out << QString("\t\tintervals: size = %1\n").arg(tier->count());
            iTier = qobject_cast<IntervalTier *>(tier);
            int i = 0;
            foreach (interval, iTier->intervals()) {
                out << QString("\t\tintervals [%1]:\n").arg(++i);
                out << QString("\t\t\txmin = %1\n")
                       .arg(QString::number(interval->tMin().toDouble(), 'f', 16));
                out << QString("\t\t\txmax = %1\n")
                       .arg(QString::number(interval->tMax().toDouble(), 'f', 16));
                out << QString("\t\t\ttext = \"%1\"\n").arg(interval->text().replace("\"", "\"\""));
            }
        }
        else if (tier->tierType() == AnnotationTier::TierType_Points) {
            out << QString("\t\tpoints: size = %1\n").arg(tier->count());
            pTier = qobject_cast<PointTier *>(tier);
            int i = 0;
            foreach (point, pTier->points()) {
                out << QString("\t\tpoints [%1]:\n").arg(++i);
                out << QString("\t\t\ttime = %1\n")
                       .arg(QString::number(point->time().toDouble(), 'f', 16));
                out << QString("\t\t\tmark = \"%1\"\n").arg(point->text().replace("\"", "\"\""));
            }
        }
    }
    file.close();
    return true;
}

QList<PraatTierData> PraatTextGrid::getTierData(const QString &filename)
{
    QList<PraatTierData> ret;

    QFile file(filename);
    QString line, tierName;
    double xMin_d, xMax_d;
    int tierSize = 0;
    int tierType = -1;
    // Open file, create stream
    if (!file.open(QIODevice::ReadOnly)) {
        return ret;
    }
    QTextStream stream(&file);
    detectEncoding(file, stream);

    // Find file type (normal or short)
    bool isShortFile = false;
    line = stream.readLine();
    if (line.contains("ooTextFile short"))
        isShortFile = true;
    if (stream.atEnd())
        return ret; // extra safey for incorrect files

    do {
        line = stream.readLine();
        // File processing loop
        if (line.contains("IntervalTier") || line.contains("TextTier")) {
            if (readTierHeader(isShortFile, stream, line, tierType, tierSize, tierName, xMin_d, xMax_d)) {
                PraatTierData tier;
                tier.name = tierName;
                tier.type = (tierType == 1) ? "Intervals" : "Points";
                tier.itemsCount = tierSize;
                ret << tier;
            }
        }
    } while (!stream.atEnd());

    file.close();
    return ret;
}

} // namespace Core
} // namespace Praaline
