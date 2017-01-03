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

#include "SyllableProminenceAnnotator.h"

SyllableProminenceAnnotator::SyllableProminenceAnnotator(QObject *parent) :
    QObject(parent)
{
}

// static
void SyllableProminenceAnnotator::prepareFeatures(QHash<QString, RealValueList> &features, IntervalTier *tier_syll, IntervalTier *tier_phones)
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
            syll->setAttribute("int_peak", Measures::mean(tier_syll, "int_peak", isyll, 4, 4, true, "f0_min"));
        }
        // Logarithmic transformation
        if (tier_syll->interval(isyll)->attribute("f0_min").toInt() > 0) {
            double f0_max = tier_syll->interval(isyll)->attribute("f0_max").toDouble();
            double f0_max_st = 12.0 * log2(f0_max);
            tier_syll->interval(isyll)->setAttribute("f0_max_st", f0_max_st);
        }
    }
    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        // Duration (log)
        features["syll_dur_rel22"] << Measures::relative(tier_syll, "duration", isyll, 2, 2, true, "", false);
        features["syll_dur_rel33"] << Measures::relative(tier_syll, "duration", isyll, 3, 3, true, "", false);
        features["syll_dur_rel44"] << Measures::relative(tier_syll, "duration", isyll, 4, 4, true, "", false);
        features["syll_dur_log_rel22"] << Measures::relative(tier_syll, "duration_log", isyll, 2, 2, true, "", true);
        features["syll_dur_log_rel33"] << Measures::relative(tier_syll, "duration_log", isyll, 3, 3, true, "", true);
        features["syll_dur_log_rel44"] << Measures::relative(tier_syll, "duration_log", isyll, 4, 4, true, "", true);
        features["syll_dur_log_rel55"] << Measures::relative(tier_syll, "duration_log", isyll, 5, 5, true, "", true);
        // Pitch ST
        features["f0_max_st_rel22"] << Measures::relative(tier_syll, "f0_max_st", isyll, 2, 2, true, "f0_min", true);
        features["f0_max_st_rel33"] << Measures::relative(tier_syll, "f0_max_st", isyll, 3, 3, true, "f0_min", true);
        features["f0_max_st_rel44"] << Measures::relative(tier_syll, "f0_max_st", isyll, 4, 4, true, "f0_min", true);
        features["f0_max_st_rel55"] << Measures::relative(tier_syll, "f0_max_st", isyll, 5, 5, true, "f0_min", true);
        features["f0_mean_st"] << tier_syll->interval(isyll)->attribute("f0_mean").toDouble();
        features["f0_mean_st_rel22"] << Measures::relative(tier_syll, "f0_mean", isyll, 2, 2, true, "f0_min", true);
        features["f0_mean_st_rel33"] << Measures::relative(tier_syll, "f0_mean", isyll, 3, 3, true, "f0_min", true);
        features["f0_mean_st_rel44"] << Measures::relative(tier_syll, "f0_mean", isyll, 4, 4, true, "f0_min", true);
        features["f0_mean_st_rel55"] << Measures::relative(tier_syll, "f0_mean", isyll, 5, 5, true, "f0_min", true);
        double up = tier_syll->interval(isyll)->attribute("up").toDouble();
        double down = tier_syll->interval(isyll)->attribute("down").toDouble();
        features["f0_up"] << up;
        features["f0_down"] << down;
        features["f0_mvt"] << (up + down);
        features["f0_traj"] << tier_syll->interval(isyll)->attribute("trajectory").toDouble();
        // Intensity dB
        features["intensity"] << tier_syll->interval(isyll)->attribute("int_peak").toDouble();
        features["intensity_rel22"]  << Measures::relative(tier_syll, "int_peak", isyll, 2, 2, true, "f0_min", true);
        features["intensity_rel33"]  << Measures::relative(tier_syll, "int_peak", isyll, 3, 3, true, "f0_min", true);
        features["intensity_rel44"]  << Measures::relative(tier_syll, "int_peak", isyll, 4, 4, true, "f0_min", true);
        features["intensity_rel55"]  << Measures::relative(tier_syll, "int_peak", isyll, 5, 5, true, "f0_min", true);
        // Following pause
        if (isyll < tier_syll->count() - 1 && tier_syll->interval(isyll+1)->isPauseSilent()) {
            double pausedur = tier_syll->interval(isyll+1)->duration().toDouble();
            features["following_pause_dur"] << pausedur;
        }
        else {
            features["following_pause_dur"] << 0.0;
        }
    }
}

QString doubleToString(double number, QString decimalSeparator = ".")
{
    return QString::number(number, 'f', 6).replace(".", decimalSeparator);
}

void SyllableProminenceAnnotator::outputRFACE(const QString &sampleID, IntervalTier *tier_syll, IntervalTier *tier_token,
                                           QHash<QString, RealValueList> &features, QTextStream &out)
{
    QStringList featureSelection;
    featureSelection << "syll_dur_log_rel22_z" << "syll_dur_log_rel33_z" << "syll_dur_log_rel44_z" << "syll_dur_log_rel55_z" <<
                        "f0_max_st_rel22" << "f0_max_st_rel33" << "f0_max_st_rel44" << "f0_max_st_rel55" <<
                        "f0_mean_st_rel22" << "f0_mean_st_rel33" << "f0_mean_st_rel44" << "f0_mean_st_rel55" <<
                        "f0_up" << "f0_down" << "f0_mvt" << "f0_traj" <<
                        "intensity_rel22" << "intensity_rel33" << "intensity_rel44" << "intensity_rel55";

    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        QString sylldata;

        // Get attributes
        Interval *syll = tier_syll->interval(isyll);
        QString delivery = syll->attribute("delivery").toString();
        QString prom = syll->attribute("prom").toString();
        QString sylltext = syll->text().replace(" ", "_").trimmed();
        QList<Interval *> tokens = tier_token->getIntervalsOverlappingWith(syll, RealTime(0, 5000));

        // Skip
        if (syll->isPauseSilent()) continue;
        if (!prom.isEmpty() && prom != "p" && prom != "P") continue;
        if (delivery == "H" || delivery == "%" || delivery == "&") continue;
        if (syll->attribute("f0_min").toInt() == 0) continue; // only stylized

        // METADATA + SYLL DATA
        sylldata.append(sampleID).append(":");
        sylldata.append(QString::number(isyll)).append(":");
        sylldata.append(syll->text()).append("\t");
        // FEATURES
        foreach (QString featureName, featureSelection) {
            if (featureName.endsWith("_z")) {
                featureName.chop(2);
                sylldata.append(doubleToString(features[featureName].zscore(isyll), ",")).append("\t");
            }
            else
                sylldata.append(doubleToString(features[featureName].at(isyll), ",")).append("\t");
        }
        // Following pause presence
        sylldata.append((features["following_pause_dur"].at(isyll) > 0) ? "Y" : "N").append("\t");
        sylldata.append((tokens.count() > 0 && tokens.last()->text() == "euh") ? "F" : "0").append("\t");
        // Token data - initial or final syllalble
        bool initial = false, final = false;
        if (tokens.count() > 0) {
            // Initial - final
            if (tokens.first()->tMin() == syll->tMin()) initial = true;
            if (tokens.last()->tMax() == syll->tMax()) final = true;
        }
        if (tokens.count() == 0)
            sylldata.append("0\t");
        else {
            if (initial && final)   sylldata.append("U\t");
            else if (initial)       sylldata.append("I\t");
            else if (final)         sylldata.append("F\t");
            else                    sylldata.append("0\t");
        }
        // Schwa
        sylldata.append((sylltext.endsWith("@")) ? "@" : "0").append("\t");

        // CLASS
        if (prom == "p" || prom == "P") sylldata.append("P");
        else                            sylldata.append("0");

        out << sylldata << "\n";
    }
}

void SyllableProminenceAnnotator::readRFACEprediction(QString filename, IntervalTier *tier_syll, QString attribute)
{
    QString line;
    QFile file(filename);
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;
    QTextStream stream(&file);
    line = stream.readLine(); // header
    QString classA = line.section("\t", 2, 2).right(1);
    QString classB = line.section("\t", 3, 3).right(1);
    for (int isyll = 0; isyll < tier_syll->count() && !stream.atEnd(); isyll++) {
        Interval *syll = tier_syll->interval(isyll);
        QString delivery = syll->attribute("delivery").toString();
        QString prom = syll->attribute("prom").toString();
        // Skip
        if (syll->isPauseSilent()) continue;
        if (!prom.isEmpty() && prom != "p" && prom != "P") continue;
        if (delivery == "H" || delivery == "%" || delivery == "&") continue;
        if (syll->attribute("f0_min").toInt() == 0) continue; // only stylized
        // read
        line = stream.readLine();
        double probClassA = line.section("\t", 2, 2).toDouble();
        double probClassB = line.section("\t", 3, 3).toDouble();
        // insert
        syll->setAttribute(attribute + "_" + classA, probClassA);
        syll->setAttribute(attribute + "_" + classB, probClassB);
        syll->setAttribute(attribute, ((probClassA > probClassB) ? classA : classB));
    }
    file.close();
}

void SyllableProminenceAnnotator::annotateRFACE(QString filenameModel, const QString &sampleID,
                                             IntervalTier *tier_syll, IntervalTier *tier_token,
                                             QHash<QString, RealValueList> &features, QString attributeOutput)
{
    // Get a temporary file
    QString filenameFeatures = m_modelsPath + "/prompraaline.afm";
    QString filenamePrediction = m_modelsPath + "/predpraaline.tsv";
    // QTemporaryFile fileFeatures, filePrediction;
    // Write out features
    QFile fileFeatures(filenameFeatures);
    if ( !fileFeatures.open( QIODevice::ReadWrite | QIODevice::Text ) ) return;
    // if (fileFeatures.open()) {
        // filenameFeatures = fileFeatures.fileName();
        QTextStream streamFeatures(&fileFeatures);
        streamFeatures.setCodec("UTF-8");
        streamFeatures.setGenerateByteOrderMark(true);
        streamFeatures << "\tN:syll_dur_log_rel22_z\tN:syll_dur_log_rel33_z\tN:syll_dur_log_rel44_z\tN:f0_max_st_rel22_z\tN:f0_max_st_rel33_z\tN:f0_max_st_rel44_z\tN:f0_mean_st_rel22_z\tN:f0_mean_st_rel33_z\tN:f0_mean_st_rel44_z\tN:f0_up\tN:f0_down\tN:f0_mvt\tN:f0_traj\tN:intensity_rel22_z\tN:intensity_rel33_z\tN:intensity_rel44_z\tB:pause_after\tB:filled_pause\tC:position\tB:schwa\tB:prom\n";
        outputRFACE(sampleID, tier_syll, tier_token, features, streamFeatures);
        fileFeatures.close();
    // }
    // Touch the output file, and get its filename
//    if (filePrediction.open()) {
//        filenamePrediction = filePrediction.fileName();
//        filePrediction.close();
//    }
    // Pass to Random Forest annotator
    QProcess decoder;
    // DIRECTORY:
    QString decoderPath = "D:/PROMRF/";
    decoder.start(decoderPath + "rf-ace.exe" , QStringList() <<
                  "-L" << filenameModel << "-T" << filenameFeatures << "-P" << filenamePrediction);
    if (!decoder.waitForStarted(-1)) return;
    if (!decoder.waitForFinished(-1)) return;
    // Read responses into token list
    readRFACEprediction(filenamePrediction, tier_syll, attributeOutput);
}

void SyllableProminenceAnnotator::outputSVM(IntervalTier *tier_syll, IntervalTier *tier_token,
                                         QHash<QString, RealValueList> &features, QTextStream &out)
{
    QStringList featureSelection;
    featureSelection << "syll_dur_log_rel22_z" << "syll_dur_log_rel33_z" << "syll_dur_log_rel44_z" << "syll_dur_log_rel55_z" <<
                        "f0_max_st_rel22" << "f0_max_st_rel33" << "f0_max_st_rel44" << "f0_max_st_rel55" <<
                        "f0_mean_st_rel22" << "f0_mean_st_rel33" << "f0_mean_st_rel44" << "f0_mean_st_rel55" <<
                        "f0_up" << "f0_down" << "f0_mvt" << "f0_traj" <<
                        "intensity_rel22" << "intensity_rel33" << "intensity_rel44" << "intensity_rel55";


    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        QString sylldata;

        // Get attributes
        Interval *syll = tier_syll->interval(isyll);
        QString delivery = syll->attribute("delivery").toString();
        QString prom = syll->attribute("prom").toString();
        QString sylltext = syll->text().replace(" ", "_").trimmed();
        QList<Interval *> tokens = tier_token->getIntervalsOverlappingWith(syll, RealTime(0, 5000));

        // Skip
        if (syll->isPauseSilent()) continue;
        if (!prom.isEmpty() && prom != "p" && prom != "P") continue;
        if (delivery == "H" || delivery == "%" || delivery == "&") continue;
        if (syll->attribute("f0_min").toInt() == 0) continue; // only stylized

        // LibSVM format
        // CLASS
        if (prom == "p" || prom == "P")
            sylldata.append("+1 ");
        else
            sylldata.append("-1 ");
        // FEATURES
        int ifeature = 1;
        foreach (QString featureName, featureSelection) {
            sylldata.append(QString::number(ifeature)).append(":");
            if (featureName.endsWith("_z")) {
                featureName.chop(2);
                sylldata.append(doubleToString(features[featureName].zscore(isyll))).append(" ");
            }
            else
                sylldata.append(doubleToString(features[featureName].at(isyll))).append(" ");
            ifeature++;
        }
        // Following pause presence
        sylldata.append(QString::number(ifeature)).append(":");
        sylldata.append((features["following_pause_dur"].at(isyll) > 0) ? "1" : "0").append(" ");
        ifeature++;
        // Is euh
        sylldata.append(QString::number(ifeature)).append(":");
        sylldata.append((tokens.count() > 0 && tokens.last()->text() == "euh") ? "1" : "0").append(" ");
        ifeature++;
        // Token data - initial or final syllalble
        bool initial = false, final = false;
        if (tokens.count() > 0) {
            // Initial - final
            if (tokens.first()->tMin() == syll->tMin()) initial = true;
            if (tokens.last()->tMax() == syll->tMax()) final = true;
        }
        sylldata.append(QString::number(ifeature)).append(":");
        sylldata.append((final) ? "1" : "0").append(" ");
        ifeature++;
        sylldata.append(QString::number(ifeature)).append(":");
        sylldata.append((initial) ? "1" : "0").append(" ");
        ifeature++;
        // Schwa
        sylldata.append(QString::number(ifeature)).append(":");
        sylldata.append((sylltext.endsWith("@")) ? "1" : "0").append(" ");

        out << sylldata << "\n";
    }
}

int quantize(double x, int factor, int max)
{
    int r = (int) (x * factor);
    if (r < -max) r = -max;
    if (r > max) r = max;
    return r;
}

int SyllableProminenceAnnotator::outputCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                                        QHash<QString, RealValueList> &features, bool withPOS, QTextStream &out,
                                        bool createSequences)
{
    int noSequences = 0;
    QStringList featureSelection;
    featureSelection << "syll_dur_log_rel22_z" << "syll_dur_log_rel33_z" << "syll_dur_log_rel44_z" << "syll_dur_log_rel55_z" <<
                        "f0_max_st_rel22" << "f0_max_st_rel33" << "f0_max_st_rel44" << "f0_max_st_rel55" <<
                        "f0_mean_st_rel22" << "f0_mean_st_rel33" << "f0_mean_st_rel44" << "f0_mean_st_rel55" <<
                        "f0_up" << "f0_down" << "f0_mvt" << "f0_traj" <<
                        "intensity_rel22" << "intensity_rel33" << "intensity_rel44" << "intensity_rel55";

    bool endSequence = true;
    for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
        // Get attributes
        Interval *syll = tier_syll->interval(isyll);
        QString delivery = syll->attribute("delivery").toString();
        QString prom = syll->attribute("prom").toString();

        QString sylltext = syll->text().replace(" ", "_").trimmed();

        QList<Interval *> tokens = tier_token->getIntervalsOverlappingWith(syll, RealTime(0, 5000));

        // if (!prom.isEmpty() && prom != "p" && prom != "P") exclude = true;
        // if (delivery == "H" || delivery == "%" || delivery == "&") exclude = true;

        if (createSequences) {
            if (syll->isPauseSilent() && syll->duration().toDouble() > 0.100) {
                if (!endSequence) {
                    out << "\n";
                    endSequence = true;
                    noSequences++;
                }
                continue;
            }
            endSequence = false;
        }

        if (sylltext.length() == 0) sylltext = "_";
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
        // Schwa
        if (sylltext.endsWith("@"))
            out << "@\t";
        else
            out << "0\t";
        // Delivery
        if (delivery.isEmpty()) delivery = "0";
        if (delivery == "p" || delivery == "P") delivery = "0";
        if (delivery.contains("*")) delivery = "_";
        out << delivery << "\t";
        // Random Forest Classification
        QString classRF = syll->attribute("prom-rf").toString();
        if (classRF.isEmpty()) classRF = "?";
        //out << classRF << "\t";
        // CLASS
        // if (prom == "p" || prom == "P")//
        if (prom == "W" || prom == "S")
            out << "P";
        else
            out << "0";
        out << "\n";
    }
    return noSequences;
}

IntervalTier *SyllableProminenceAnnotator::annotateWithCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
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
               promise->interval(isyll)->duration().toDouble() > 0.100 &&
               isyll < promise->count())
            isyll++;
        //
        QStringList fields = line.split("\t");
        QString response = fields.at(fields.count() - 3).section("/", 0, 0);
        double score = fields.at(fields.count() - 3).section("/", 1, 1).toDouble();
        // qDebug() << line << response;
        if (promise->interval(isyll)->text() == "x") {
            if (response == "P")
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

QString outputCRFTemplate() {
    QString ret;
    for (int i = 0; i <= 20; i++) {
        QString n = QString::number(i);
        QString m = n; if (m.length() < 2) m.prepend("0");
        ret.append("U").append(m).append("0:%x[-3,").append(n).append("]\n");
        ret.append("U").append(m).append("1:%x[-2,").append(n).append("]\n");
        ret.append("U").append(m).append("2:%x[-1,").append(n).append("]\n");
        ret.append("U").append(m).append("3:%x[-0,").append(n).append("]\n");
        ret.append("U").append(m).append("4:%x[1,").append(n).append("]\n");
        ret.append("U").append(m).append("5:%x[2,").append(n).append("]\n");
        ret.append("U").append(m).append("6:%x[3,").append(n).append("]\n");
        ret.append("U").append(m).append(QString("7:%x[-1,%1]/[0,%1]\n").arg(n));
        ret.append("U").append(m).append(QString("8:%x[0,%1]/[1,%1]\n").arg(n));
        ret.append("\n");
    }
    return ret;
}

IntervalTier *SyllableProminenceAnnotator::annotate(QString annotationID, const QString &filenameModel, bool withPOS, const QString &tierName,
                                                 IntervalTier *tier_syll, IntervalTier *tier_token, QString speakerID,
                                                 QTextStream &streamFeatures, QTextStream &streamFeaturesCRF)
{
    m_currentAnnotationID = annotationID;

    QHash<QString, RealValueList> features;
    prepareFeatures(features, tier_syll);

    IntervalTier *promise = annotateWithCRF(tier_syll, tier_token, features, withPOS, filenameModel, tierName);

    if (withPOS == false) return promise;
    // otherwise, write feature files

    QStringList featureSelection;
    featureSelection << "syll_dur_log_rel22_z" << "syll_dur_log_rel33_z" << "syll_dur_log_rel44_z" << "syll_dur_log_rel55_z" <<
                        "f0_max_st_rel22" << "f0_max_st_rel33" << "f0_max_st_rel44" << "f0_max_st_rel55" <<
                        "f0_mean_st_rel22" << "f0_mean_st_rel33" << "f0_mean_st_rel44" << "f0_mean_st_rel55" <<
                        "f0_up" << "f0_down" << "f0_mvt" << "f0_traj" <<
                        "intensity_rel22" << "intensity_rel33" << "intensity_rel44" << "intensity_rel55";

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
        streamFeatures << syll->attribute("promise").toString() << "\t";
        streamFeatures << syll->attribute("promise_pos").toString() << "\n";
    }

    outputCRF(tier_syll, tier_token, features, withPOS, streamFeaturesCRF, false);
    return promise;
}


QString SyllableProminenceAnnotator::process(AnnotationTierGroup *txg, QString annotationID, QTextStream &out)
{
    QString ret;

    IntervalTier *tier_phones = txg->getIntervalTierByName("phone");
    if (!tier_phones) return QString("Tier not found phones");
    IntervalTier *tier_syll = txg->getIntervalTierByName("syll");
    if (!tier_syll) return QString("Tier not found syll");
    IntervalTier *tier_token = txg->getIntervalTierByName("tok_min");
    if (!tier_token) return QString("Tier not found tok-min");
    m_currentAnnotationID = annotationID;

    // TRAINING
    // --------------------------------------------------------------------
    QHash<QString, RealValueList> features;
    prepareFeatures(features, tier_syll);

//    outputRFACE(annot->ID(), tier_syll, tier_token, features, out);
//    outputSVM(tier_syll, tier_token, features, out);
//    QString filenameModel = "D:/PROMRF/promacoustic_5.sf";
//    annotateRFACE(filenameModel, com->ID(), tier_syll, tier_token, features, "prom-rf");

    // CRF training file
//    int noSeq = outputCRF(tier_syll, tier_token, features, true, out);

    // ANNOTATE WITH CRF
    QString filenameModel;

    filenameModel = "d:/promcrf/train_cregio_nopos.model";
    IntervalTier *promiseA = annotateWithCRF(tier_syll, tier_token, features, false, filenameModel, "promise-cregio");
    filenameModel = "d:/promcrf/train_cross_nopos.model";
    IntervalTier *promiseB = annotateWithCRF(tier_syll, tier_token, features, false, filenameModel, "promise-cross");

    AnnotationTierGroup *txgOut = new AnnotationTierGroup();
    PraatTextGrid::load(QString("D:/Sandrine_promise/audiobook2/%1.textgrid").arg(annotationID), txgOut);

    txgOut->removeTierByName("promise");
    txgOut->removeTierByName("promise-pos");

    int index = txgOut->getTierIndexByName("prom");
    if (index < 0) index = txgOut->getTierIndexByName("syll");
    txgOut->insertTierReplacing(index + 1, promiseB);
    txgOut->insertTierReplacing(index + 1, promiseA);

    PraatTextGrid::save(QString("D:/Sandrine_promise/out/%1.textgrid").arg(annotationID), txgOut);
    delete txgOut;

//    filenameModel = "d:/promcrf/promcrf_pos_all.model";
//    IntervalTier *promisepos = annotateWithCRF(tier_syll, tier_token, features, true, filenameModel, "promise-pos");
//    index = txg->getTierIndexByName("promise");
//    txg->insertTierReplacing(index + 1, promisepos);

    // CREATE CHECK TIER
//    int index = txg->getTierIndexByName("prom");
//    IntervalTier *prom = txg->getIntervalTierByName("prom");
//    IntervalTier *check = new IntervalTier(prom, "prom-check");
//    for (int i = 0; i < tier_syll->countItems(); ++i) {
//        if (tier_syll->interval(i)->attribute("prom").toString().toUpper() != promise->interval(i)->text())
//            check->interval(i)->setText("*");
//    }
//    txg->insertTierReplacing(index + 1, check);

//    delete promisepos;

//    foreach (Interval *syll, tier_syll->intervals()) {
//        ret.append(syll->text()).append("\t");
//        ret.append(syll->attribute("prom").toString()).append("\t");
//        ret.append(syll->attribute("prom-rf").toString()).append("\n");
//    }

    //return outputCRFTemplate();

    qDebug() << annotationID; // << noSeq;
    ret.append(annotationID + "OK"); //.append("\t").append(QString::number(noSeq));
    return ret;
}

