#include <QDebug>
#include <QString>
#include <QPair>
#include <QList>
#include <QPointer>
#include <QProcess>
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryFile>

#include "PraalineCore/Base/RealTime.h"
#include "PraalineCore/Corpus/CorpusRecording.h"
#include "PraalineCore/Annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "OpenSmileVAD.h"

OpenSmileVAD::OpenSmileVAD(QObject *parent) : QObject(parent)
{

}

// static
bool OpenSmileVAD::runVAD(const QString &filenameInputWave, QList<QPair<double, double> > &resultVADActivations)
{
    // Check if wave file exists
    if (!QFile::exists(filenameInputWave)) return false;
    // Call OpenSMILE
    QString openSmilePath = QDir::homePath() + "/Praaline/tools/opensmile/";
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
    resultVADActivations.clear();
    // Read file
    while (!vad.atEnd()) {
        QString line = vad.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (!line.contains(",")) continue;
        QString stringTime = line.section(",", 0, 0);
        QString stringScore = line.section(",", 1, 1);
        double time = stringTime.toDouble();
        double score = stringScore.toDouble();
        resultVADActivations << QPair<double, double>(time, score);
    }
    return true;
}

// static
IntervalTier *
OpenSmileVAD::splitToUtterances(QList<QPair<double, double> > &VADActivations,
                                RealTime minimumDurationSilent, RealTime minimumDurationVoice,
                                QString textSilent, QString textVoice)
{
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
    tier->replace("", "sounding", textVoice);
    tier->replace("", "silent", textSilent);    
    return tier;
}

// static
QList<Interval *>
OpenSmileVAD::splitToUtterancesWithoutPauses(QList<QPair<double, double> > &VADActivations,
                                             RealTime minimumDurationSilent, RealTime minimumDurationVoice,
                                             QString textVoice)
{
    IntervalTier *tier_utterances = splitToUtterances(VADActivations, minimumDurationSilent, minimumDurationVoice, "silent", "sounding");
    if (!tier_utterances) return QList<Interval *>();
    QList<RealTime> boundaries;
    boundaries << tier_utterances->tMin();
    for (int i = 1; i < tier_utterances->count() - 1; ++i) {
        if (tier_utterances->interval(i)->text() == "silent")
            boundaries << tier_utterances->interval(i)->tCenter();
    }
    boundaries << tier_utterances->tMax();
    QList<Interval *> intervals;
    for (int i = 0; i < boundaries.count() - 1; ++i)
        intervals << new Interval(boundaries.at(i), boundaries.at(i + 1), textVoice);
    delete tier_utterances;
    return intervals;
}

