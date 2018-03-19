#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QPair>
#include <QDir>
#include <QTemporaryFile>
#include <QProcess>
#include <QCoreApplication>
#include <QTextStream>
#include <QCryptographicHash>
#include "base/RealTime.h"
#include "SoundInfo.h"

namespace Praaline {
namespace Media {

SoundInfo::SoundInfo() :
    channels(0), sampleRate(0), precisionBits(0), bitRate(0), filesize(0)
{
}

SoundInfo::~SoundInfo()
{
}

QPair<long long, QString> SoundInfo::getSizeAndMD5(const QString &filename)
{
    // checksum
    QCryptographicHash crypto(QCryptographicHash::Md5);
    QFile recFile(filename);
    recFile.open(QFile::ReadOnly);
    while(!recFile.atEnd()) {
        crypto.addData(recFile.read(8192));
    }
    QByteArray hash = crypto.result();
    QString md5 = QString(hash.toHex());
    long long size = recFile.size();
    recFile.close();
    return QPair<long long, QString>(size, md5);
}

bool SoundInfo::getSoundInfo(const QString &filename, SoundInfo &info)
{
    QProcess sox;
    // DIRECTORY:
    QString soxPath = QDir::homePath() + "/Praaline/tools/sox/";
    sox.setWorkingDirectory(soxPath);
    qDebug() << soxPath + "sox --i " << filename;
    sox.start(soxPath + "sox" , QStringList() <<
              "--i" << filename);
    if (!sox.waitForStarted(-1))    // sets current thread to sleep and waits for sox end
        return false;
    if (!sox.waitForFinished(-1))   // sets current thread to sleep and waits for sox end
        return false;
    QString output(sox.readAllStandardOutput());
    QString errors(sox.readAllStandardError());
    qDebug() << output;
    qDebug() << errors;
    QStringList output_l = output.split("\n");
    foreach (QString line, output_l) {
        if (line.isEmpty()) continue;
        if (!line.contains(":")) continue;
        if (line.startsWith("Input File")) {
            // new file
            info.filename = line.section(":", 1, -1).replace("'", "").trimmed();
            QPair<long long, QString> size_and_checksum;
            size_and_checksum = getSizeAndMD5(filename);
            info.filesize = size_and_checksum.first;
            info.checksumMD5 = size_and_checksum.second;
        }
        else if (line.startsWith("Channels")) {
            info.channels = line.section(":", 1, -1).trimmed().toInt();
        }
        else if (line.startsWith("Sample Rate")) {
            info.sampleRate = line.section(":", 1, -1).trimmed().toInt();
        }
        else if (line.startsWith("Precision")) {
            info.precisionBits = line.section(":", 1, -1).replace("-bit", "").trimmed().toInt();
        }
        else if (line.startsWith("Duration")) {
            QString durationText = line.section(":", 1, -1).section("=", 1, 1).section("samples", 0, 0)
                                       .replace("samples", "").trimmed();
            sv_frame_t samples = durationText.toLongLong();
            info.duration = RealTime::frame2RealTime(samples, info.sampleRate);
            // Old method
            // QString durationText = line.section(":", 1, -1).section("=", 0, 0).trimmed();
            // long h = durationText.section(":", 0, 0).toInt();
            // long m = durationText.section(":", 1, 1).toInt();
            // long s = durationText.section(":", 2, 2).section(".", 0, 0).toInt();
            // long ms = durationText.section(".", 1, 1).toInt();
            // long msec = h * 3600000 + m * 60000 + s * 1000 + ms;
            // qDebug() << h << m << s << ms << "   " << msec;
            // info.duration = RealTime::fromMilliseconds(msec);
        }
        else if (line.startsWith("Bit Rate")) {
            QString b = line.section(":", 1, -1).replace("k", "").trimmed();
            info.bitRate = (int) (b.toDouble() * 1000.0);
        }
        else if (line.startsWith("Sample Encoding")) {
            info.encoding = line.section(":", 1, -1).trimmed();
        }
    }
    // qDebug() << info.duration.toDouble();
    return true;
}

} // namespace Media
} // namespace Praaline
