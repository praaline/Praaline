#include <math.h>
#include <QPair>
#include <QString>
#include <QList>
#include <QHash>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDir>
#include <QDebug>

#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/statistics/Measures.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "ProsodicBoundariesAnnotator.h"

struct ProsodicBoundariesAnnotatorData {
    ProsodicBoundariesAnnotatorData() :
        attributeBoundaryTrain("promise_boundary"), attributeBoundaryContourTrain("promise_contour"),
        fileFeaturesTable(0), streamFeaturesTable(0), fileCRFData(0), streamCRFData(0),
        headerLineInFeaturesTable(true)
    {
        featureSelection << "following_pause_dur" << "following_pause_dur_log" <<
                            "syll_dur_rel20" << "syll_dur_rel30" << "syll_dur_rel40" <<
                            "syll_dur_log_rel20" << "syll_dur_log_rel30" << "syll_dur_log_rel40" <<
                            "f0_mean_st_rel20" << "f0_mean_st_rel30" << "f0_mean_st_rel40" << "f0_mean_st_rel50" <<
                            "f0_up" << "f0_down"<< "f0_traj";
        extraFeatures << "boundaryForce" << "boundary2Force" << "boundaryExpertForce";
    }

    QString currentAnnotationID;
    // Statistical models
    QString modelsPath;
    QString modelFilenameBoundary;
    QString modelFilenameBoundaryContours;
    // Attribute names
    QString attributeBoundaryTrain;
    QString attributeBoundaryContourTrain;
    // Feature tables
    QFile *fileFeaturesTable;
    QTextStream *streamFeaturesTable;
    QFile *fileCRFData;
    QTextStream *streamCRFData;
    bool headerLineInFeaturesTable;
    QStringList featureSelection;
    QStringList extraFeatures;
};

ProsodicBoundariesAnnotator::ProsodicBoundariesAnnotator(QObject *parent) :
    QObject(parent), d(new ProsodicBoundariesAnnotatorData)
{
}

ProsodicBoundariesAnnotator::~ProsodicBoundariesAnnotator()
{
    if (d->streamFeaturesTable) { d->streamFeaturesTable->flush();  delete d->streamFeaturesTable;  }
    if (d->streamCRFData)       { d->streamCRFData->flush();        delete d->streamCRFData;        }
    if (d->fileFeaturesTable)   { d->fileFeaturesTable->close();    delete d->fileFeaturesTable;    }
    if (d->fileCRFData)         { d->fileCRFData->close();          delete d->fileCRFData;          }
    delete d;
}

// ========================================================================================================================================
// Parameters: statistical models
// ========================================================================================================================================

QString ProsodicBoundariesAnnotator::modelsPath() const
{
    return d->modelsPath;
}

void ProsodicBoundariesAnnotator::setModelsPath(const QString &modelsPath)
{
    d->modelsPath = modelsPath;
}

QString ProsodicBoundariesAnnotator::modelFilenameBoundary() const
{
    return d->modelFilenameBoundary;
}

void ProsodicBoundariesAnnotator::setModelFilenameBoundary(const QString &filename)
{
    d->modelFilenameBoundary = filename;
}

QString ProsodicBoundariesAnnotator::modelFilenameBoundaryCountours() const
{
    return d->modelFilenameBoundaryContours;
}

void ProsodicBoundariesAnnotator::setModelFilenameBoundaryCountours(const QString &filename)
{
    d->modelFilenameBoundaryContours = filename;
}

// ========================================================================================================================================
// Parameters: attribute names
// ========================================================================================================================================

QString ProsodicBoundariesAnnotator::attributeBoundaryTrain() const
{
    return d->attributeBoundaryTrain;
}

void ProsodicBoundariesAnnotator::setAttributeBoundaryTrain(const QString &attributeID)
{
    d->attributeBoundaryTrain = attributeID;
}

QString ProsodicBoundariesAnnotator::attributeBoundaryContourTrain() const
{
    return d->attributeBoundaryContourTrain;
}

void ProsodicBoundariesAnnotator::setAttributeBoundaryContourTrain(const QString &attributeID)
{
    d->attributeBoundaryContourTrain = attributeID;
}

// ========================================================================================================================================
// Methods to control the creation of feature tables and training files
// ========================================================================================================================================

bool ProsodicBoundariesAnnotator::openFeaturesTableFile(const QString &filename)
{
    d->fileFeaturesTable = new QFile(filename);
    if (! d->fileFeaturesTable->open(QFile::WriteOnly | QFile::Text)) return false;
    d->streamFeaturesTable = new QTextStream(d->fileFeaturesTable);
    d->streamFeaturesTable->setCodec("UTF-8");
    d->streamFeaturesTable->generateByteOrderMark();
    if (d->headerLineInFeaturesTable) {
        QTextStream &out = (*d->streamFeaturesTable);
        // Write features table
        out << "annotationID\tspeakerID\tindexSyll\tsyllText\ttMin\ttMax";
        foreach (QString featureID, d->featureSelection)
            out << "\t" << featureID;
        foreach (QString featureID, d->extraFeatures)
            out << "\t" << featureID;
        out << "\n";
    }
    return true;
}

bool ProsodicBoundariesAnnotator::openCRFDataFile(const QString &filename)
{
    d->fileCRFData = new QFile(filename);
    if (! d->fileCRFData->open(QFile::WriteOnly | QFile::Text)) return false;
    d->streamCRFData = new QTextStream(d->fileCRFData);
    d->streamCRFData->setCodec("UTF-8");
    d->streamCRFData->generateByteOrderMark();
    return true;
}

void ProsodicBoundariesAnnotator::closeFeaturesTableFile()
{
    if (d->streamFeaturesTable) {
        d->streamFeaturesTable->flush();
        delete d->streamFeaturesTable;
        d->streamFeaturesTable = Q_NULLPTR;
    }
    if (d->fileFeaturesTable) {
        d->fileFeaturesTable->close();
        delete d->fileFeaturesTable;
        d->fileFeaturesTable = Q_NULLPTR;
    }
}

void ProsodicBoundariesAnnotator::closeCRFDataFile()
{
    if (d->streamCRFData) {
        d->streamCRFData->flush();
        delete d->streamCRFData;
        d->streamCRFData = Q_NULLPTR;
    }
    if (d->fileCRFData) {
        d->fileCRFData->close();
        delete d->fileCRFData;
        d->fileCRFData = Q_NULLPTR;
    }
}

// ========================================================================================================================================
// PREPARE FEATURES
// ========================================================================================================================================

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
        double up = tier_syll->interval(isyll)->attribute("intrasyllabup").toDouble();
        double down = tier_syll->interval(isyll)->attribute("intrasyllabdown").toDouble();
        features["f0_up"] << up;
        features["f0_down"] << down;
        features["f0_traj"] << tier_syll->interval(isyll)->attribute("trajectory").toDouble();
    }
}

// ========================================================================================================================================
// Output for external statistical modelling tools
// ========================================================================================================================================

int ProsodicBoundariesAnnotator::outputCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                                           QHash<QString, RealValueList> &features, bool withPOS, bool annotateContours,
                                           QTextStream &out, bool createSequences)
{
    bool quantize(true);
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
        QString sylltext = syll->text().replace(" ", "_").replace("\t", "").trimmed();
        if (sylltext.length() == 0) sylltext = "_";
        // Target attribute
        QString target;
        bool translateTarget(false);
        if (!annotateContours) {
            target = syll->attribute(d->attributeBoundaryTrain).toString();
            if (translateTarget) {
                if      (target.contains("///"))  target = "B3";
                else if (target.contains("//"))   target = "B2";
                else                              target = "0";
            }
        } else {
            target = syll->attribute(d->attributeBoundaryContourTrain).toString();
            if (translateTarget) {
                if      (target.contains("C"))  target = "C";
                else if (target.contains("S"))  target = "S";
                else if (target.contains("T"))  target = "T";
                else if (target.contains("F"))  target = "F";
                else                            target = "0";
            }
        }
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
                if (quantize) {
                    int x = Measures::quantize(features[featureName].zscore(isyll), 10, 200);
                    if (x == 200 || x == -200) out << "NA\t"; else out << x << "\t";
                } else {
                    out << features[featureName].zscore(isyll) << "\t";
                }
            }
            else {
                if (quantize) {
                    int x = Measures::quantize(features[featureName].at(isyll), 10, 200);
                    if (x == 200 || x == -200) out << "NA\t"; else out << x << "\t";
                } else {
                    out << features[featureName].at(isyll) << "\t";
                }
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
        // Extra feautres
        foreach (QString featureID, d->extraFeatures) {
            out << syll->attribute(featureID).toString() << "\t";
        }

        // TARGET CLASS (see above)
        out << target;
        out << "\n";
    }
    return noSequences;
}

IntervalTier *ProsodicBoundariesAnnotator::annotateWithCRF(IntervalTier *tier_syll, IntervalTier *tier_token,
                                                           QHash<QString, RealValueList> &features, bool withPOS, bool annotateContours,
                                                           const QString &filenameModel, const QString &tier_name)
{
    IntervalTier *promise = tier_syll->clone(tier_name);
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
    outputCRF(tier_syll, tier_token, features, withPOS, annotateContours, streamIn);
    fileIn.close();
    // Touch the output file, and get its filename
    if (!fileOut.open()) return promise;
    filenameOut = fileOut.fileName();
    fileOut.close();
    // Pass to CRF decoder
    QProcess decoder;
    // DIRECTORY:
    QString decoderCommand = QDir::homePath() + "/Praaline/tools/crf/crf_test";
    decoder.start(decoderCommand, QStringList() <<
                  "-m" << filenameModel << "-v" << "2" << "-o" << filenameOut << filenameIn);
    if (!decoder.waitForStarted(-1)) return promise;
    if (!decoder.waitForFinished(-1)) return promise;

    // These responses will be carried over as automatic annotations. Other responses will be ignored.
    QStringList possibleResponseClasses;
    if (annotateContours)
        possibleResponseClasses << "C" << "S" << "T" << "F";
    else
        possibleResponseClasses << "B2" << "B3";

    // Read responses into list
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
        QString response = fields.at(fields.count() - (possibleResponseClasses.count() + 2)).section("/", 0, 0);
        double score = fields.at(fields.count() - (possibleResponseClasses.count() + 2)).section("/", 1, 1).toDouble();
        // qDebug() << line << response;
        if (promise->interval(isyll)->text() == "x") {
            if (possibleResponseClasses.contains(response))
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

IntervalTier *ProsodicBoundariesAnnotator::annotate(const QString &annotationID, const QString &speakerID, const QString &tierName,
                                                    Praaline::Core::IntervalTier *tier_syll, Praaline::Core::IntervalTier *tier_token,
                                                    bool annotateContours)
{
    d->currentAnnotationID = annotationID;
    QHash<QString, RealValueList> features;
    prepareFeatures(features, tier_syll);

    QString filenameModel;
    if (annotateContours)
        filenameModel = d->modelsPath + d->modelFilenameBoundaryContours;
    else
        filenameModel = d->modelsPath + d->modelFilenameBoundary;

    IntervalTier *promise = annotateWithCRF(tier_syll, tier_token, features, true, annotateContours, filenameModel, tierName);

    if (d->streamFeaturesTable) {
        QTextStream &out = (*d->streamFeaturesTable);
        // Write features table
        QStringList featureSelection;
        featureSelection << "following_pause_dur" << "following_pause_dur_log" <<
                            "syll_dur_rel20" << "syll_dur_rel30" << "syll_dur_rel40" <<
                            "syll_dur_log_rel20" << "syll_dur_log_rel30" << "syll_dur_log_rel40" <<
                            "f0_mean_st_rel20" << "f0_mean_st_rel30" << "f0_mean_st_rel40" << "f0_mean_st_rel50" <<
                            "f0_up" << "f0_down"<< "f0_traj";
        for (int isyll = 0; isyll < tier_syll->count(); isyll++) {
            out << annotationID << "\t" << speakerID  << "\t" << isyll << "\t";
            Interval *syll = tier_syll->interval(isyll);
            out << syll->text() << "\t";
            out << syll->tMin().toDouble() << "\t";
            out << syll->tMax().toDouble() << "\t";
            foreach (QString featureID, featureSelection) {
                if (featureID.endsWith("_z")) {
                    featureID.chop(2);
                    out << features[featureID].zscore(isyll) << "\t";
                }
                else {
                    out << features[featureID].at(isyll) << "\t";
                }
            }
            out << syll->attribute(d->attributeBoundaryTrain).toString();
            foreach (QString featureID, d->extraFeatures)
                out << "\t" << syll->attribute(featureID).toString();
            out << "\n";
        }
    }
    if (d->streamCRFData) {
        outputCRF(tier_syll, tier_token, features, true, annotateContours, (*d->streamCRFData), true);
    }
    return promise;
}
