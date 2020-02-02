#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QProcess>

#include "PraalineMedia/SoxExternal.h"

namespace Praaline {
namespace Media {

SoxExternal::SoxExternal(QObject *parent) :
    QObject(parent)
{
}

QString SoxExternal::bin()
{
    QString path;
    QFile soxApp;

    if (soxApp.exists(qApp->applicationDirPath()+QDir::separator()+"sox.exe"))
        path = qApp->applicationDirPath()+QDir::separator()+"sox.exe";
    else if (soxApp.exists(qApp->applicationDirPath()+QDir::separator()+"sox"))
        path = qApp->applicationDirPath()+QDir::separator()+"sox";
    else if (soxApp.exists(QDir::toNativeSeparators("/usr/bin/sox")))
        path = QDir::toNativeSeparators("/usr/bin/sox");
    else if (soxApp.exists(QDir::toNativeSeparators("/usr/local/bin/sox")))
        path = QDir::toNativeSeparators("/usr/local/bin/sox");

    return path;
}

QString SoxExternal::about()
{
    QString desc;
    if (!bin().isEmpty()) {
        QProcess proc;
        proc.start(bin()+" --version");
        proc.waitForFinished();
        desc = proc.readAll().simplified();
        proc.close();
    }
    return desc;
}

QString SoxExternal::formats()
{
    QString result;
    QString tmp;
    if (!bin().isEmpty()) {
        QProcess proc;
        proc.start(bin()+" --help");
        proc.waitForFinished();
        tmp = proc.readAll().simplified();
        proc.close();
    }
    if (!tmp.isEmpty()) {
        QStringList lines = tmp.split("AUDIO FILE FORMATS:");
        if (!lines.isEmpty()) {
            QStringList audioList = lines.takeLast().simplified().split(" ");
            foreach(QString audioFormat, audioList) {
                if (audioFormat.startsWith("PLAYLIST"))
                    break;
                result.append("*."+audioFormat+" ");
            }
        }
    }
    return result;
}

QString SoxExternal::dat(QString filename, float fps, int duration, bool x, bool y, int xFactor, int yFactor, bool forceNull)
{
    emit status(tr("Generating data ..."));
    QString result;
    QString data;
    data=QDir::tempPath()+QDir::separator()+qApp->applicationName()+".dat";
    QString curves;
    QFile audioFile;
    QFile tmp(data);
    if (audioFile.exists(filename) && !bin().isEmpty()) {
        if (tmp.exists())
            tmp.remove();
        QProcess proc;
        QString exestring = bin()+" \""+filename+"\" -r "+QString::number(fps)+" \""+data+"\" trim 0 "+QString::number(duration/fps);
        proc.start(exestring);
        proc.waitForFinished();
        proc.close();
    }
    else {
        emit error(tr("Audio file or SoX does not exists"));
        return result;
    }

    if (tmp.exists()) {
        if (tmp.open(QIODevice::ReadOnly|QIODevice::Text))
            curves = tmp.readAll();
        tmp.close();
        tmp.remove();
    }

    double maxX = 0;
    double maxY = 0;
    if (!curves.isEmpty()) {
        QStringList lines = curves.split("\n", QString::SkipEmptyParts);
        foreach(QString line, lines) {
            if (!line.startsWith(";")) {
                QStringList cordinates = line.simplified().split(" ");
                int posCurr=0;
                foreach(QString pos, cordinates) {
                    if (posCurr==1) {
                        double posX = pos.toDouble();
                        if (posX<0)
                            posX = -posX;
                        if (posX>maxX)
                            maxX=posX;
                    }
                    if (posCurr==2) {
                        double posY = pos.toDouble();
                        if (posY<0)
                            posY = -posY;
                        if (posY>maxY)
                            maxY=posY;
                    }
                    posCurr++;
                }
            }
        }
    }
    if (!curves.isEmpty()) {
        QString dat;
        QStringList lines = curves.split("\n", QString::SkipEmptyParts);
        foreach(QString line, lines) {
            if (!line.startsWith(";")) {
                QStringList cordinates = line.simplified().split(" ");
                int posCurr=0;
                foreach(QString pos, cordinates) {
                    if (posCurr==1 && x) {
                        dat.append(QString::number(pos.toDouble()*xFactor/maxX,'f',10));
                        if (!y) {
                            if (forceNull)
                                dat.append("_0.00000");
                            dat.append("\n");
                        }
                    }
                    if (posCurr==2 && y) {
                        if (x)
                            dat.append("_");
                        else {
                            if (forceNull)
                                dat.append("0.00000_");
                        }
                        dat.append(QString::number(pos.toDouble()*yFactor/maxY,'f',10)+"\n");
                    }
                    posCurr++;
                }
            }
        }
        result = dat;
    }
    if (result.isEmpty())
        emit error(tr("No data collected"));
    else
        emit status(tr("Done"));

    return result;
}

float SoxExternal::duration(QString filename, float fps)
{
    float result = 0;
    QString tmp;
    if (!bin().isEmpty()) {
        QProcess proc;
        QString exec = bin()+" \""+filename+"\" -n stat";
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start(exec);
        proc.waitForFinished();
        tmp = proc.readAll();
        proc.close();
    }
    if (!tmp.isEmpty()) {
        QStringList lines = tmp.split("\n");
        foreach(QString line, lines) {
            if (line.startsWith("Length (seconds):"))
                result = line.simplified().remove("Length (seconds): ").toFloat();
        }
    }
    if (fps>0 && result>0)
        result = result*fps;
    return result;
}

} // namespace Media
} // namespace Praaline

