#include <math.h>
#include <QPair>
#include <QString>
#include <QList>
#include <QHash>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QDebug>

#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/query/Measures.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "ProsodicBoundariesAnnotator.h"

ProsodicBoundariesAnnotator::ProsodicBoundariesAnnotator(QObject *parent) :
    QObject(parent)
{
}

// static
void ProsodicBoundariesAnnotator::prepareFeatures(QHash<QString, RealValueList> &features, IntervalTier *tier_syll)
{
    // Prepare data
    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        Interval *syll = tier_syll->interval(isyll);
        // Duration
        double syll_dur = syll->attribute("duration").toDouble();
        syll->setAttribute("duration_log", log(syll_dur));
        // Smoothing for non-stylised syllables
        if (!syll->isPauseSilent() && syll->attribute("f0_min").toInt() == 0) {
            syll->setAttribute("f0_min", Measures::mean(tier_syll, "f0_min", isyll, 4, 4, true, "f0_min"));
            syll->setAttribute("f0_max", Measures::mean(tier_syll, "f0_max", isyll, 4, 4, true, "f0_min"));
            syll->setAttribute("f0_mean", Measures::mean(tier_syll, "f0_mean", isyll, 4, 4, true, "f0_min"));
        }
        // Logarithmic transformation
        if (tier_syll->interval(isyll)->attribute("f0_min").toInt() > 0) {
            double f0_max = tier_syll->interval(isyll)->attribute("f0_max").toDouble();
            double f0_max_st = 12.0 * log2(f0_max);
            tier_syll->interval(isyll)->setAttribute("f0_max_st", f0_max_st);
        }
    }
    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        // Following pause
        if (isyll < tier_syll->count() - 1 && tier_syll->interval(isyll+1)->isPauseSilent()) {
            double pausedur = tier_syll->interval(isyll+1)->duration().toDouble();
            features["following_pause_dur"] << pausedur;
            features["following_pause_dur_log"] << log(pausedur);
        }
        else {
            features["following_pause_dur"] << 0.0;
            features["following_pause_dur_log"] << 0.0;
        }
        // Duration and log(duration)
        features["syll_dur_rel20"] << Measures::relative(tier_syll, "duration", isyll, 2, 0, true, "", false);
        features["syll_dur_rel30"] << Measures::relative(tier_syll, "duration", isyll, 3, 0, true, "", false);
        features["syll_dur_rel40"] << Measures::relative(tier_syll, "duration", isyll, 4, 0, true, "", false);
        features["syll_dur_log_rel20"] << Measures::relative(tier_syll, "duration_log", isyll, 2, 0, true, "", true);
        features["syll_dur_log_rel30"] << Measures::relative(tier_syll, "duration_log", isyll, 3, 0, true, "", true);
        features["syll_dur_log_rel40"] << Measures::relative(tier_syll, "duration_log", isyll, 4, 0, true, "", true);
        // Pitch mean ST
        features["f0_mean_st"] << tier_syll->interval(isyll)->attribute("f0_mean").toDouble();
        features["f0_mean_st_rel20"] << Measures::relative(tier_syll, "f0_mean", isyll, 2, 0, true, "f0_min", true);
        features["f0_mean_st_rel30"] << Measures::relative(tier_syll, "f0_mean", isyll, 3, 0, true, "f0_min", true);
        features["f0_mean_st_rel40"] << Measures::relative(tier_syll, "f0_mean", isyll, 4, 0, true, "f0_min", true);
        features["f0_mean_st_rel50"] << Measures::relative(tier_syll, "f0_mean", isyll, 5, 0, true, "f0_min", true);
        double up = tier_syll->interval(isyll)->attribute("up").toDouble();
        double down = tier_syll->interval(isyll)->attribute("down").toDouble();
        features["f0_up"] << up;
        features["f0_down"] << down;
        features["f0_traj"] << tier_syll->interval(isyll)->attribute("trajectory").toDouble();
    }
}

int ProsodicBoundariesAnnotator::quantize(double x, int factor, int max)
{
    int r = (int) (x * factor);
    if (r < -max) r = -max;
    if (r > max) r = max;
    return r;
}

int ProsodicBoundariesAnnotator::outputCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                                        QHash<QString, RealValueList> &features, bool withPOS, QTextStream &out,
                                        bool createSequences)
{
    int noSequences = 0;
    QStringList featureSelection;
    featureSelection << "following_pause_dur" << "following_pause_dur_log" <<
                        "syll_dur_rel20" << "syll_dur_rel30" << "syll_dur_rel40" <<
                        "syll_dur_log_rel20" << "syll_dur_log_rel30" << "syll_dur_log_rel40" <<
                        "f0_mean_st_rel20" << "f0_mean_st_rel30" << "f0_mean_st_rel40" << "f0_mean_st_rel50" <<
                        "f0_up" << "f0_down"<< "f0_traj";

    bool endSequence = true;
    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        // Get attributes: syllable text
        Interval *syll = tier_syll->interval(isyll);
        QString sylltext = syll->text().replace(" ", "_").trimmed();
        if (sylltext.length() == 0) sylltext = "_";
        // Target attribute
        QString boundary = syll->attribute("boundary").toString();
        if      (boundary.contains("///"))  boundary = "B3";
        else if (boundary.contains("//"))   boundary = "B2";
        else                                boundary = "0";
        // Tokens
        QList<Interval *> tokens = tier_token->getIntervalsOverlappingWith(syll, RealTime(0, 5000));

        if (createSequences) {
            if (syll->isPauseSilent() && syll->duration().toDouble() > 0.300) {
                if (!endSequence) {
                    out << "\n";
                    endSequence = true;
                    noSequences++;
                }
                continue;
            }
            endSequence = false;
        }

        // ID
        // out << m_currentAnnotationID << "\t";
        // out << syll->xMin().toDouble() << "\t";
        out << sylltext << "\t";
        // FEATURES
        foreach (QString featureName, featureSelection) {
            if (featureName.endsWith("_z")) {
                featureName.chop(2);
                int x = quantize(features[featureName].zscore(isyll), 10, 200);
                if (x == 200 || x == -200) out << "NA\t"; else out << x << "\t";
            }
            else {
                int x = quantize(features[featureName].at(isyll), 10, 200);
                if (x == 200 || x == -200) out << "NA\t"; else out << x << "\t";
            }
        }
        // Following pause (silent or filled) presence
        if (syll->isPauseSilent())
            out << "SIL\t";
        else if (tokens.count() > 0 && tokens.last()->text() == "euh")
            out << "FIL\t";
        else if (features["following_pause_dur"].at(isyll) > 0)
            out << "BRK\t";
        else
            out << "CNT\t";
        // Token data
        bool initial = false, final = false;
        if (tokens.count() > 0) {
            // Initial - final
            if (tokens.first()->tMin() == syll->tMin()) initial = true;
            if (tokens.last()->tMax() == syll->tMax()) final = true;
            if (initial && final)
                out << "U\t";
            else if (final)
                out << "F\t";
            else if (initial)
                out << "I\t";
            else
                out << "0\t";
            // POS
            if (withPOS) {
                QString tokentext = tokens.last()->text().replace(" ", "_");
                if (tokentext.length() == 0) tokentext = "_";
                QString pos = tokens.last()->attribute("pos_min").toString();
                if (pos.length() == 0) pos = "_";
                out << tokentext << "\t";
                out << pos.left(3) << "\t";
                out << pos << "\t";
            }
        }
        else {
            out << "_\t";
            if (withPOS) { out << "_\t_\t_\t"; }
        }

        // CLASS
        if (boundary == "B3" || boundary == "B2")
            out << boundary;
        else
            out << "0";
        out << "\n";
    }
    return noSequences;
}

IntervalTier *ProsodicBoundariesAnnotator::annotateWithCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                                                           QHash<QString, RealValueList> &features, bool withPOS,
                                                           const QString &filenameModel, const QString &tier_name)
{
    IntervalTier *promise = new IntervalTier(tier_syll, tier_name);
    foreach (Interval *prom, promise->intervals()) {
        if (!prom->isPauseSilent()) prom->setText("x");
    }
    // Get a temporary file and write out the features
    QString filenameIn, filenameOut;
    QTemporaryFile fileIn, fileOut;
    if (!fileIn.open()) return promise; //empty
    filenameIn = fileIn.fileName();
    QTextStream streamIn(&fileIn);
    streamIn.setCodec("UTF-8");
    streamIn.setGenerateByteOrderMark(true);
    outputCRF(tier_syll, tier_token, features, withPOS, streamIn);
    fileIn.close();
    // Touch the output file, and get its filename
    if (!fileOut.open()) return promise;
    filenameOut = fileOut.fileName();
    fileOut.close();
    // Pass to CRF decoder
    QProcess decoder;
    // DIRECTORY:
    QString decoderCommand;
#ifdef Q_OS_WIN
    QString appPath = QCoreApplication::applicationDirPath();
    decoderCommand = appPath + "/tools/crf/crf_test";
#endif
#ifdef Q_OS_MAC
    decoderCommand = "/usr/local/bin/crf_test";
#else
    decoderCommand = "/usr/local/bin/crf_test";
#endif
    decoder.start(decoderCommand, QStringList() <<
                  "-m" << filenameModel << "-v" << "2" << "-o" << filenameOut << filenameIn);
    if (!decoder.waitForStarted(-1)) return promise;
    if (!decoder.waitForFinished(-1)) return promise;
    // Read responses into token list
    if (!fileOut.open()) return promise;
    QTextStream streamOut(&fileOut);
    streamOut.setCodec("UTF-8");
    int isyll = 0;
    do {
        QString line = streamOut.readLine();
        if (line.isEmpty() || line.startsWith("#")) continue;
        if (isyll >= promise->count()) continue;
        // move cursor to next syllable at end of sequence pauses
        while (promise->interval(isyll)->text() != "x" &&
               promise->interval(isyll)->duration().toDouble() > 0.300 &&
               isyll < promise->count())
            isyll++;
        //
        QStringList fields = line.split("\t");
        QString response = fields.at(fields.count() - 3).section("/", 0, 0);
        double score = fields.at(fields.count() - 3).section("/", 1, 1).toDouble();
        // qDebug() << line << response;
        if (promise->interval(isyll)->text() == "x") {
            if (response == "B3" || response == "B2")
                promise->interval(isyll)->setText(response);
            else
                promise->interval(isyll)->setText("");
            promise->interval(isyll)->setAttribute(tier_name + "_score", score);
        }
        isyll++;
    } while (!streamOut.atEnd());
    fileOut.close();
    return promise;
}

IntervalTier *ProsodicBoundariesAnnotator::annotate(QString annotationID, const QString &filenameModel, bool withPOS, const QString &tierName,
                                                 IntervalTier *tier_syll, IntervalTier *tier_token, QString speakerID,
                                                 QTextStream &streamFeatures, QTextStream &streamFeaturesCRF)
{
    m_currentAnnotationID = annotationID;

    QHash<QString, RealValueList> features;
    prepareFeatures(features, tier_syll);

    IntervalTier *promise = annotateWithCRF(tier_syll, tier_token, features, withPOS, filenameModel, tierName);
    return promise;

    // Write feature files
    QStringList featureSelection;
    featureSelection << "following_pause_dur" << "following_pause_dur_log" <<
                        "syll_dur_rel20" << "syll_dur_rel30" << "syll_dur_rel40" <<
                        "syll_dur_log_rel20" << "syll_dur_log_rel30" << "syll_dur_log_rel40" <<
                        "f0_mean_st_rel20" << "f0_mean_st_rel30" << "f0_mean_st_rel40" << "f0_mean_st_rel50" <<
                        "f0_up" << "f0_down"<< "f0_traj";
    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        streamFeatures << annotationID << "\t" << speakerID  << "\t" << isyll << "\t";
        Interval *syll = tier_syll->interval(isyll);
        streamFeatures << syll->text() << "\t";
        streamFeatures << syll->tMin().toDouble() << "\t";
        streamFeatures << syll->tMax().toDouble() << "\t";
        foreach (QString featureName, featureSelection) {
            if (featureName.endsWith("_z")) {
                featureName.chop(2);
                streamFeatures << features[featureName].zscore(isyll) << "\t";
            }
            else {
                streamFeatures << features[featureName].at(isyll) << "\t";
            }
        }
        streamFeatures << syll->attribute("boundary_auto").toString() << "\n";
    }
    outputCRF(tier_syll, tier_token, features, withPOS, streamFeaturesCRF, false);
    return promise;
}
