#include <QDebug>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>

#include "PraalineCore/Annotation/Interval.h"
using namespace Praaline::Core;

#include "PraalineMedia/AudioSegmenter.h"

namespace Praaline {
namespace Media {

// Audio Segmenter
// Input:  a wave file to extract segments from
//         the path where the extracts should be saved
//         a list of intervals: their tMin, tMax defines the segments to extract
//         attributeID -> this attribute will be used to name the extracted file (otherwise, will use text)
// Output: extracted wave files

// static
QList<QStringList> AudioSegmenter::script(const QString &filenameIn, const QString &pathOut, QList<Interval *> segments,
                                          const QString &attribute, uint newSamplingRate, bool normalise, int channels)
{
    QList<QStringList> script;
    int i = 1;
    foreach (Interval *seg, segments) {
        QStringList arguments;
        arguments << filenameIn;
        QString filename = (attribute.isEmpty()) ? seg->text() : seg->attribute(attribute).toString();
        if (filename.isEmpty()) filename = QString::number(i);
        if (normalise) {
            arguments << QString("--norm");
        }
        arguments << QString("%1/%2.wav").arg(pathOut).arg(filename);
        if (segments.count() > 0) {
            arguments << "trim" << QString::number(seg->tMin().toDouble(), 'g', 6) <<
                                   QString::number(seg->duration().toDouble(), 'g', 6);
        }
        if (channels > 0) {
            arguments << "channels" << QString("%1").arg(channels);
        }
        if (newSamplingRate) {
            arguments << "rate" << QString::number(newSamplingRate);
        }
        script << arguments;
        i++;
    }
    return script;
}

bool AudioSegmenter::segment(const QString &filenameIn, const QString &pathOut, QList<Interval *> segments,
                             const QString &attribute, uint newSamplingRate, bool normalise, int channels)
{
    QProcess sox;
    // DIRECTORY:
    QString soxPath = QDir::homePath() + "/Praaline/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    QString path = pathOut;
    if (path.endsWith("/")) path.chop(1);
    QList<QStringList> scriptArguments = script(filenameIn, pathOut, segments, attribute, newSamplingRate, normalise, channels);
    foreach (QStringList arguments, scriptArguments) {
        sox.start(soxPath + "sox", arguments);
        if (!sox.waitForStarted(-1))    // sets current thread to sleep and waits for sox end
            return false;
        if (!sox.waitForFinished(-1))   // sets current thread to sleep and waits for sox end
            return false;
        QString output(sox.readAllStandardOutput());
        QString errors(sox.readAllStandardError());
        // qDebug() << output;
        // qDebug() << errors;
    }
    return true;
}

// static
bool AudioSegmenter::segment(const QString &filenameIn, const QString &filenameOut,
                             RealTime timeFrom, RealTime timeTo,
                             uint newSamplingRate, bool normalise, int channels)
{
    QProcess sox;
    // DIRECTORY:
    QString soxPath = QDir::homePath() + "/Praaline/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    QStringList arguments;
    arguments << filenameIn;
    if (normalise) {
        arguments << QString("--norm");
    }
    arguments << filenameOut;
    arguments << "trim" << QString::number(timeFrom.toDouble(), 'g', 6) <<
                           QString::number((timeTo - timeFrom).toDouble(), 'g', 6);
    if (channels > 0) {
        arguments << "channels" << QString("%1").arg(channels);
    }
    if (newSamplingRate) {
        arguments << "rate" << QString::number(newSamplingRate);
    }
    sox.start(soxPath + "sox", arguments);
    if (!sox.waitForStarted(-1))    // sets current thread to sleep and waits for sox end
        return false;
    if (!sox.waitForFinished(-1))   // sets current thread to sleep and waits for sox end
        return false;
    QString output(sox.readAllStandardOutput());
    QString errors(sox.readAllStandardError());
    // qDebug() << output;
    // qDebug() << errors;
    return true;
}

bool AudioSegmenter::runSoxCommand(const QString &command)
{
    QProcess sox;
    // DIRECTORY:
    QString soxPath = QDir::homePath() + "/Praaline/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    QStringList arguments = command.split(" ");
    sox.start(soxPath + "sox", arguments);
    if (!sox.waitForStarted(-1))    // sets current thread to sleep and waits for sox end
        return false;
    if (!sox.waitForFinished(-1))   // sets current thread to sleep and waits for sox end
        return false;
    QString output(sox.readAllStandardOutput());
    QString errors(sox.readAllStandardError());
    // qDebug() << output;
    // qDebug() << errors;
    return true;
}

bool AudioSegmenter::bookmarkCutting(const QString &filenameIn, const QString &filenameOut, QList<Interval *> intervalsToRemove)
{
    QProcess sox;
    // DIRECTORY:
    QString soxPath = QDir::homePath() + "/Praaline/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    int i = 1;
    QMap<RealTime, Interval *> mapRemoveSorted;
    foreach (Interval *intv, intervalsToRemove)
        mapRemoveSorted.insert(intv->tMin(), intv);
    QMapIterator<RealTime, Interval *> iter(mapRemoveSorted);
    iter.toBack();
    while (iter.hasPrevious()) {
        iter.previous();
        QStringList arguments;
        // input file
        if (i == 1)
            arguments << filenameIn;
        else
            arguments << QString("temp%1.wav").arg(i);
        // output file
        if (i == intervalsToRemove.count())
            arguments << filenameOut;
        else
            arguments << QString("temp%1.wav").arg(i + 1);
        // trim command
        arguments << "trim 0 "
                  << QString("=%1").arg(QString::number(iter.value()->tMin().toDouble(), 'g', 6))
                  << QString("=%1").arg(QString::number(iter.value()->tMax().toDouble(), 'g', 6));
        // run sox for this interval
        // qDebug() << soxPath + "sox" << arguments;
//        sox.start(soxPath + "sox", arguments);
//        if (!sox.waitForFinished(-1)) // sets current thread to sleep and waits for sox end
//            return false;
        // next
        i++;
    }
    return true;
}

// static
bool AudioSegmenter::resample(const QString &filenameIn, const QString &filenameOut,
                              uint newSamplingRate, bool normalise, int channels)
{
    QProcess sox;
    // DIRECTORY:
    QString soxPath = QDir::homePath() + "/Praaline/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    QStringList arguments;
    arguments << filenameIn;
    if (normalise) {
        arguments << QString("--norm");
    }
    arguments << filenameOut;
    if (channels > 0) {
        arguments << "channels" << QString("%1").arg(channels);
    }
    if (newSamplingRate) {
        arguments << "rate" << QString::number(newSamplingRate);
    }    
    sox.start(soxPath + "sox", arguments);
    if (!sox.waitForStarted(-1))    // sets current thread to sleep and waits for sox end
        return false;
    if (!sox.waitForFinished(-1))   // sets current thread to sleep and waits for sox end
        return false;
    QString output(sox.readAllStandardOutput());
    QString errors(sox.readAllStandardError());
    qDebug() << output;
    qDebug() << errors;
    return true;
}

// static
bool AudioSegmenter::generateSilence(const QString &filenameOut, RealTime duration)
{
    // Sox generate silence
    // sox -n -r 16000 -c 1 silence0.4.wav trim 0.0 0.4
    // Description:
    // -n    : no input (null file handle)
    // -r     : sampling rate (16000 Hz)
    // -c     : channel (1 for mono, 2 for stereo)
    QString cmd = QString("-n -r 16000 -c 1 %1 trim 0.0 %2").arg(filenameOut).arg(duration.toDouble());
    cmd = cmd.replace("  ", " ");
    return runSoxCommand(cmd);
}

} // namespace Media
} // namespace Praaline
