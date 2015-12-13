#include <QString>
#include <QStringList>
#include <QProcess>
#include <QCoreApplication>
#include "audiosegmenter.h"

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
    QString appPath = QCoreApplication::applicationDirPath();
    QString soxPath = appPath + "/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    QString path = pathOut;
    if (path.endsWith("/")) path.chop(1);
    QList<QStringList> scriptArguments = script(filenameIn, pathOut, segments, attribute, newSamplingRate, normalise, channels);
    foreach (QStringList arguments, scriptArguments) {
        sox.start(soxPath + "sox.exe", arguments);
        if (!sox.waitForFinished(-1)) // sets current thread to sleep and waits for sox end
            return false;
    }
    return true;
}
