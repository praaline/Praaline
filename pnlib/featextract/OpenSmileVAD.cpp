#include <QDebug>
#include <QString>
#include <QPair>
#include <QList>
#include <QPointer>
#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryFile>

#include "pncore/base/RealTime.h"
#include "pncore/corpus/CorpusRecording.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "OpenSmileVAD.h"

OpenSmileVAD::OpenSmileVAD(QObject *parent) : QObject(parent)
{

}

bool OpenSmileVAD::runVAD(const QString &filenameInputWave, QList<QPair<double, double> > &resultVADActivation)
{
    // Check if wave file exists
    if (!QFile::exists(filenameInputWave)) return false;
    // Call OpenSMILE
    QString appPath = QCoreApplication::applicationDirPath();
    QString openSmilePath = appPath + "/tools/opensmile/";
    QProcess openSmile;
    openSmile.setWorkingDirectory(openSmilePath);
    QStringList openSmileParams;
    // Clean-up previous analysis
    QString filenameCSVOutput;
    QTemporaryFile fileCSVOutput;
    if (fileCSVOutput.open()) {
        filenameCSVOutput = fileCSVOutput.fileName();
        qDebug() << "Writing VAD data to " << filenameCSVOutput;
    } else return false;
    // OpenSMILE parameters
    openSmileParams << "-C" << openSmilePath + "vad_opensource.conf" <<
                       "-I" << filenameInputWave <<
                       "-csvoutput" << filenameCSVOutput;
    openSmile.start(openSmilePath + "SMILExtract", openSmileParams);
    if (!openSmile.waitForStarted(-1)) return false;
    if (!openSmile.waitForFinished(-1)) return false;
    QString out = QString(openSmile.readAllStandardOutput());
    qDebug() << out;
    QString err = QString(openSmile.readAllStandardError());
    qDebug() << err;
    // Read VAD results
    QTextStream vad(&fileCSVOutput);
    vad.setCodec("UTF-8");
    // Clean-up results list
    resultVADActivation.clear();
    // Read file
    while (!vad.atEnd()) {
        QString line = vad.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (!line.contains(",")) continue;
        QString stringTime = line.section(",", 0, 0);
        QString stringScore = line.section(",", 1, 1);
        double time = stringTime.toDouble();
        double score = stringScore.toDouble();
        resultVADActivation << QPair<double, double>(time, score);
    }
    return true;
}

IntervalTier *OpenSmileVAD::splitToUtterances(QPointer<CorpusRecording> rec,
                                              RealTime minimumDurationSilent, RealTime minimumDurationVoice,
                                              QString textSilent, QString textVoice)
{
    if (!rec) return 0;
    QString filenameWave = rec->baseMediaPath() + "/" + QString(rec->filename()).replace(".wav", ".16k.wav");
    QList<QPair<double, double> > VADActivations;
    bool result = runVAD(filenameWave, VADActivations);
    if (!result) return 0;
    if (VADActivations.isEmpty()) return 0;
    QList<Interval *> intervalsSounding, intervalsSilent;
    QPair<double, double> point = VADActivations.first();
    RealTime start = RealTime::fromSeconds(point.first);
    bool sounding = (point.second > 0.0) ? true : false;
    foreach (point, VADActivations) {
        if (sounding && (point.second < 0.0)) {
            RealTime end = RealTime::fromSeconds(point.first);
            intervalsSounding << new Interval(start, end, "sounding");
            sounding = false;
            start = end;
        }
        else if (!sounding && (point.second > 0.0)) {
            RealTime end = RealTime::fromSeconds(point.first);
            intervalsSilent << new Interval(start, end, "silent");
            sounding = true;
            start = end;
        }
    }
    QList<Interval *> intervals;
    intervals << intervalsSounding << intervalsSilent;
    IntervalTier *tier = new IntervalTier("VAD", intervals);
    // Apply thresholds
    foreach (Interval *intv, tier->intervals()) {
        if (intv->duration() < minimumDurationSilent)
            intv->setText("sounding");
    }
    tier->mergeIdenticalAnnotations();
    foreach (Interval *intv, tier->intervals()) {
        if (intv->duration() < minimumDurationVoice)
            intv->setText("silent");
    }
    tier->mergeIdenticalAnnotations();
    tier->replaceText("sounding", textVoice);
    tier->replaceText("silent", textSilent);
    return tier;
}
