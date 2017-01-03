#include <QObject>
#include <QDebug>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QCoreApplication>
#include <QTemporaryDir>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/structure/AnnotationStructureLevel.h"
#include "pncore/annotation/AnnotationDataTable.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

#include "annotationpluginpraatscript.h"
#include "prosogram.h"

ProsoGram::ProsoGram(QObject *parent) :
    AnnotationPluginPraatScript(parent)
{
}

QString translate(QString attribute) {
    if (attribute == "up") return "intrasyllabup";
    else if (attribute == "down") return "intrasyllabdown";
    return attribute;
}

bool updateTierFromAnnotationTable(QString filenameSpreadsheet, QString tMinAttribute, QString tMaxAttribute,
                                   QPointer<IntervalTier> tier)
{
    if (!tier) return false;
    // Read annotation table
    AnnotationDataTable table;
    if (!(table.readFromFile(filenameSpreadsheet))) {
        qDebug() << "Annotation data table file not read.";
        return false;
    }
    // Update attributes from annotation table
    for (int i = 0; i < table.getRowCount(); i++) {
        RealTime table_xMin = table.getSeconds(i, tMinAttribute);
        RealTime table_xMax = table.getSeconds(i, tMaxAttribute);
        RealTime table_xCenter = (table_xMin + table_xMax) / 2.0;
        Interval *intv = tier->intervalAtTime(table_xCenter);
        if (!intv) continue;
        foreach (QString attributeName, table.getFieldNames()) {
            QVariant value = table.getData(i, attributeName);
            intv->setAttribute(translate(attributeName), value);
            // qDebug() << attributeName << " " << value;
        }
    }
    return true;
}

bool createTierFromAnnotationTable(QString filenameSpreadsheet, QString tMinAttribute, QString tMaxAttribute,
                                   QPointer<IntervalTier> tier)
{
    if (!tier) return false;
    // Read annotation table
    AnnotationDataTable table;
    if (!(table.readFromFile(filenameSpreadsheet))) {
        qDebug() << "Annotation data table file not read.";
        return false;
    }
    // Create intervals and attributes from annotation table
    QList<Interval *> intervals;
    for (int i = 0; i < table.getRowCount(); i++) {
        RealTime table_xMin = table.getSeconds(i, tMinAttribute);
        RealTime table_xMax = table.getSeconds(i, tMaxAttribute);
        Interval *intv = new Interval(table_xMin, table_xMax, "x");
        if (!intv) continue;
        foreach (QString attributeName, table.getFieldNames()) {
            QVariant value = table.getData(i, attributeName);
            intv->setAttribute(translate(attributeName), value);
            // qDebug() << attributeName << " " << value;
        }
        intervals << intv;
    }
    tier->replaceAllIntervals(intervals);
    return true;
}


//parameters << PluginParameter("segmentationMethod", "Segmentation Method", QVariant::String, d->segmentationMethod, QStringList() <<
//                            0  "Automatic with acoustic syllables" <<
//                            1  "Nuclei in vowels in phones annotation" <<
//                            2  "Nuclei in rhyme from phones and syllables annotation" <<
//                            3  "Nuclei in syllables from phones and syllables annotation" <<
//                            4  "Nuclei in syllables from syllables annotation and local peak" <<
//                            5  "Based on segmentation annotation level");
//parameters << PluginParameter("glissandoThreshold", "Glissando threshold", QVariant::String, d->glissandoThreshold, QStringList() <<
//                            0  "G=0.16/T^2, DG=20, dmin=0.035" <<
//                            1  "G=0.24/T^2, DG=20, dmin=0.035" <<
//                            2  "G=0.32/T^2, DG=20, dmin=0.035" <<
//                            3  "G=0.32/T^2, DG=30, dmin=0.050" <<
//                            4  "G=0.24-0.32/T^2 (adaptive), DG=30, dmin=0.050" <<
//                            5  "G=0.16-0.32/T^2 (adaptive), DG=30, dmin=0.050");
//parameters << PluginParameter("keepIntermediateFiles", "Keep intermediate files", QVariant::Bool, d->keepIntermediateFiles);
//parameters << PluginParameter("createImageFiles", "Create image files", QVariant::Bool, d->createImageFiles);
//parameters << PluginParameter("tiersOnImageFiles", "Tiers (level/attribute) to include in image files", QVariant::String, d->tiersOnImageFiles);
//parameters << PluginParameter("fileFormat", "File format for images", QVariant::String, d->fileFormat);

bool ProsoGram::updateGlobal(CorpusCommunication *com, const QString &filenameGlobalsheet)
{
    if (!com) return false;
    // Read annotation table
    AnnotationDataTable table;
    if (!(table.readFromFile(filenameGlobalsheet))) {
        qDebug() << "Global profile file not read.";
        return false;
    }
    // update attributes
    for (int i = 0; i < table.getRowCount(); i++) {
        foreach (QString attributeName, table.getFieldNames()) {
            QVariant value = table.getData(i, attributeName);
            com->setProperty(attributeName.trimmed(), value);
            // qDebug() << attributeName << " " << value;
        }
    }
    return true;
}

bool fileMoveReplacingDestination(const QString &source, const QString &dest)
{
    if (QFile::exists(dest)) QFile::remove(dest);
    return QFile::rename(source, dest);
}

void ProsoGram::runProsoGram(Corpus *corpus, CorpusRecording *rec, QPointer<AnnotationTierGroup> tiers,
                             QString annotationID, QString speakerID)
{
    if (!corpus) return;
    if (!rec) return;
    if (!tiers) return;

    // Create a temporary directory
    QTemporaryDir dirTemp;
    if (!dirTemp.isValid()) return;
    QString tempDirectory = dirTemp.path(); // returns the unique directory path
    if (!tempDirectory.endsWith("/")) tempDirectory.append("/");
    // Copy the recording file into temp+.wav
    QString filenameTempRec = QString("%1_%2.wav").arg(rec->ID()).arg(speakerID);
    QFile::copy(corpus->baseMediaPath() + "/" + rec->filename(), tempDirectory + filenameTempRec);

    // Check that the needed tiers are present
    if (segmentationMethod == 1 || segmentationMethod == 2 || segmentationMethod == 3) {
        IntervalTier *tier_phones = tiers->getIntervalTierByName(levelPhone);
        if (!tier_phones) {
            emit logOutput(QString("ERROR: annotation level for phones not found"));
            return;
        }
        if (tier_phones->name() != "phone") {
            tiers->addTier(new IntervalTier(tier_phones, "phone"));
        }
    }
    if (segmentationMethod == 2 || segmentationMethod == 3 || segmentationMethod == 4) {
        IntervalTier *tier_syll = tiers->getIntervalTierByName(levelSyllable);
        if (!tier_syll) {
            emit logOutput(QString("ERROR: annotation level for syllables not found"));
            return;
        }
        if (tier_syll->name() != "syll") {
            tiers->addTier(new IntervalTier(tier_syll, "syll"));
        }
    }
    if (segmentationMethod == 5) {
        if (tiers->getIntervalTierByName(levelSegmentation) == 0) {
            emit logOutput(QString("ERROR: annotation level for segmentation not found"));
            return;
        }
    }
    if (segmentationMethod != 0) {
        PraatTextGrid::save(tempDirectory + QString("%1_%2.TextGrid").arg(rec->ID()).arg(speakerID), tiers);
    }

    QString filenameNuclei = QString("%1_%2_nucl.TextGrid").arg(rec->ID()).arg(speakerID);
    QString filenameProfile = QString("%1_%2_profile.txt").arg(rec->ID()).arg(speakerID);
    QString filenameData = QString("%1_%2_spreadsheet.txt").arg(rec->ID()).arg(speakerID);
    QString filenameStylisedPitchTier = QString("%1_%2_styl.PitchTier").arg(rec->ID()).arg(speakerID);
    QString filenameGlobalsheet = QString("%1_%2_globalsheet.txt").arg(rec->ID()).arg(speakerID);

    // Execute prosogram5
    QString appPath = QCoreApplication::applicationDirPath();
    QString script = appPath + "/plugins/prosogram/praaline_prosogram.praat";
    QStringList scriptArguments;

    // Create prosogram directory if it does not exist in the folder
    QFileInfo info(corpus->baseMediaPath() + "/" + rec->filename());
    QString prosoPath = info.absoluteDir().absolutePath() + "/prosogram/";
    if (keepIntermediateFiles) {
        info.absoluteDir().mkpath("prosogram");
    }

    // Add parameters
    QString prosogramMode = "Prosodic profile only (no drawing)"; // "Prosogram and prosodic profile"
    QString prosogramSegmentationMethod;
    if      (segmentationMethod == 0)   prosogramSegmentationMethod = "Automatic acoustic syllables";
    else if (segmentationMethod == 1)   prosogramSegmentationMethod = "Nuclei in vowels in tier phon or tier 1";
    else if (segmentationMethod == 2)   prosogramSegmentationMethod = "Nuclei in rhyme from syll and vowels in phon";
    else if (segmentationMethod == 3)   prosogramSegmentationMethod = "Nuclei in syllables in syll and vowels in phon";
    else if (segmentationMethod == 4)   prosogramSegmentationMethod = "Nuclei in syllables in syll and local peak";
    else if (segmentationMethod == 5)   prosogramSegmentationMethod = "Using external segmentation in tier segm";
    scriptArguments << prosogramMode << tempDirectory + filenameTempRec <<
                       QString::number(timeRangeFrom) << QString::number(timeRangeTo) <<
                       QString::number(f0DetectionMin) << QString::number(f0DetectionMax) <<
                       "Full" << QString::number(framePeriod) <<
                       prosogramSegmentationMethod <<
                       "G=0.16/T^2, DG=20, dmin=0.035" <<
                       ((keepIntermediateFiles) ? "1" : "0") <<
                       "4: Wide rich" << "2.0" << "1, 2" << "0" << "100" <<
                       "One strip per file" <<
                       "EMF (Windows Enhanced Metafile)" << "<input_directory>/prosogram/<basename>_";
    //EPS (Encapsulated Postscript)
    executePraatScript(script, scriptArguments);

    // Update global profile on Communication
    CorpusCommunication *com = qobject_cast<CorpusCommunication *>(rec->parent());
    if (com) {
        updateGlobal(com, tempDirectory + "globalsheet.txt");
    }

    // Update syllable tier
    IntervalTier *tier_syll = tiers->getIntervalTierByName(levelSyllable);
    AnnotationTierGroup *txgNuclei = 0;
    if ((!tier_syll) || (segmentationMethod == 0)) {
        // Automatic syllables
        txgNuclei = new AnnotationTierGroup();
        PraatTextGrid::load(tempDirectory + filenameNuclei, txgNuclei);
        if (!txgNuclei) return;
        tier_syll = txgNuclei->getIntervalTierByName("syll");
        if (!tier_syll) return;
        tier_syll->setName(levelSyllable);
        updateTierFromAnnotationTable(tempDirectory + filenameData, "nucl_t1", "nucl_t2", tier_syll);
    } else {
        updateTierFromAnnotationTable(tempDirectory + filenameData, "nucl_t1", "nucl_t2", tier_syll);
    }
    if (!corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_syll)) {
        qDebug() << "Error in saving " << annotationID << " speaker " << speakerID << " syll count " << tier_syll->count();
    }
    if (txgNuclei)
        delete txgNuclei;
    else
        delete tier_syll;

    // Move Prosogram files to storage, if needed
    if (keepIntermediateFiles) {
        fileMoveReplacingDestination(tempDirectory + filenameNuclei, prosoPath + filenameNuclei);
        fileMoveReplacingDestination(tempDirectory + filenameProfile, prosoPath + filenameProfile);
        fileMoveReplacingDestination(tempDirectory + filenameData, prosoPath + filenameData);
        fileMoveReplacingDestination(tempDirectory + filenameStylisedPitchTier, prosoPath + filenameStylisedPitchTier);
        fileMoveReplacingDestination(tempDirectory + "globalsheet.txt", prosoPath + filenameGlobalsheet);
    }
}

