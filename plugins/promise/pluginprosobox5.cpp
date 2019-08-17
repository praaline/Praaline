#include <QObject>
#include <QDebug>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QCoreApplication>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusAnnotation.h"
#include "pncore/structure/AnnotationStructureLevel.h"
#include "pncore/annotation/AnnotationDataTable.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/datastore/FileDatastore.h"

#include "attributenametranslation.h"
#include "annotationpluginpraatscript.h"
#include "pluginprosobox5.h"

PluginProsobox5::PluginProsobox5(QObject *parent) :
    AnnotationPluginPraatScript(parent)
{
}

QString translate(QString attribute) {
    if (attribute == "up") return "intrasyllabup";
    else if (attribute == "down") return "intrasyllabdown";
    return attribute;
}

bool updateTierFromAnnotationTable(QString filenameSpreadsheet, QString tMinAttribute, QString tMaxAttribute,
                                   IntervalTier *tier)
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

void PluginProsobox5::ProsoGram(Corpus *corpus, CorpusRecording *rec, CorpusAnnotation *annot)
{
    if (!corpus) return;
    if (!rec) return;
    if (!annot) return;

    QList<QString> speakerIDs = corpus->repository()->annotations()->getSpeakersInLevel(annot->ID(), "syll");
    foreach (QString speakerID , speakerIDs) {
        QString filenameTempRec = rec->basePath() + rec->filename();
        filenameTempRec = filenameTempRec.replace(rec->ID() + ".", speakerID + ".", Qt::CaseInsensitive);
        if (filenameTempRec != rec->basePath() + rec->filename())
            QFile::copy(rec->basePath() + rec->filename(), filenameTempRec);

        QFileInfo info(filenameTempRec);

        QString absPath = info.absoluteDir().absolutePath() + "/";
        QString prosoPath = absPath + "prosogram/";
        QString filename = info.fileName();

        QString filenameNuclei = QString(filename).replace(".wav", "_nucl.TextGrid");
        QString filenameProfile = QString(filename).replace(".wav", "_profile.txt");
        QString filenameSpreadsheet = QString(filename).replace(".wav", "_spreadsheet.txt");
        QString filenameStylisedPitchTier = QString(filename).replace(".wav", "_styl.PitchTier");
        QString filenameGlobalsheet = QString(filename).replace(".wav", "_globalsheet.txt");

        // Execute prosogram
        QString appPath = QCoreApplication::applicationDirPath();
        QString script = appPath + "/tools/prosogram29m/praaline_prosogram.praat";
        QStringList scriptArguments;
        // create prosogram directory if it does not exist in the folder
        info.absoluteDir().mkpath("prosogram");
        // TODO: this should be dynamic...
        QString timeRangeFrom = "0.0"; QString timeRangeTo = "0.0";
        QString f0detectionFrom = "60"; QString f0detectionTo = "450";
        QString parameterCalculation = "Full";
        QString framePeriod = "0.015";
        QString saveIntermediateData = "1";
        QString prosogramMode = "Prosodic profile only (no drawing)"; // "Prosogram and prosodic profile"
        QString tiersToShow = " "; // (filename.endsWith("_FR.wav")) ? "3, 9, 10, 14" : "3, 8, 9, 10";
        //
        scriptArguments << prosogramMode << filenameTempRec <<
                           timeRangeFrom << timeRangeTo << f0detectionFrom << f0detectionTo <<
                           parameterCalculation << framePeriod <<
                           "Nuclei in syllables in syll and vowels in phon" <<
                           "G=0.16/T^2, DG=20, dmin=0.035" << saveIntermediateData <<
                           "4: Wide rich" << "2.0" << tiersToShow << "0" << "100" <<
                           "One strip per file" <<
                           "EMF (Windows Enhanced Metafile)" << "<input_directory>/prosogram/<basename>_";
        //EPS (Encapsulated Postscript)
        executePraatScript(script, scriptArguments);

        // Move Prosogram files
        QFile::rename(absPath + "globalsheet.txt", prosoPath + filenameGlobalsheet);
        QFile::rename(absPath + filenameNuclei, prosoPath + filenameNuclei);
        QFile::rename(absPath + filenameProfile, prosoPath + filenameProfile);
        QFile::rename(absPath + filenameSpreadsheet, prosoPath + filenameSpreadsheet);
        QFile::rename(absPath + filenameStylisedPitchTier, prosoPath + filenameStylisedPitchTier);

        if (filenameTempRec != rec->basePath() + rec->filename())
            QFile::remove(filenameTempRec);

        // Update syllable tier
        IntervalTier *syll = qobject_cast<IntervalTier *>(corpus->repository()->annotations()->getTier(annot->ID(), speakerID, "syll"));
        if (!syll) return;
        updateTierFromAnnotationTable(prosoPath + filenameSpreadsheet, "nucl_t1", "nucl_t2", syll);
        corpus->repository()->annotations()->saveTier(annot->ID(), speakerID, syll);
        delete syll;
    }
}



void PluginProsobox5::CreateSyllTable(CorpusCommunication *com)
{
    if (com->annotationsCount() == 0)
        return;
    QString basePath = com->repository()->files()->basePath();
    QFileInfo info(basePath + "/" + com->annotation(0)->filename());
    QString annotationID = info.fileName().replace(".TextGrid", "", Qt::CaseInsensitive);
    QString absPath = info.absoluteDir().absolutePath() + "/";
    QString prosoPath = absPath + "prosogram/";

    qDebug() << annotationID;
    qDebug() << absPath;
    qDebug() << prosoPath;

    // Execute makesyll
    QString appPath = QCoreApplication::applicationDirPath();
    QString script = appPath + "/tools/prosobox5/praaline_makesylltable.praat";
    QStringList scriptArguments;
    QString syllTiers = "delivery,if,prom,promauto";
    QString suprasyllTiers = "tok-mwu,AP,ss,speaker";
    QString pauseTier = "ss";

    scriptArguments << annotationID << absPath << prosoPath <<
                       "phones" << "syll" << syllTiers << "1" << "1" << suprasyllTiers << pauseTier << "1";
    executePraatScript(script, scriptArguments);
}

void PluginProsobox5::ProsoProm(CorpusCommunication *com)
{
    if (com->annotationsCount() == 0)
        return;
    QString basePath = com->repository()->files()->basePath();
    QString filenameAnnotation = basePath + "/" + com->annotation(0)->filename();
    QString filenameTable = QString(filenameAnnotation).replace(".TextGrid", ".Table", Qt::CaseInsensitive);
    // Execute PromGrad
    QString appPath = QCoreApplication::applicationDirPath();
    QString script = appPath + "/tools/prosobox5/prosoprom/prosoprom.praat";
    QStringList scriptArguments;
    QString strategy = "gradual"; QString promTierName = "promauto"; QString insertPromgrad = "1";
    // QString doPromEval = "0"; QString addRelaviveParameterTiers = "0";

    // f0param, blockpause, scope, weight
    scriptArguments << filenameAnnotation << filenameTable <<
                       "mean" << "0.250" << "2" << "1" << "1" <<
                       strategy << promTierName << insertPromgrad;
    // << doPromEval << addRelaviveParameterTiers << "3 full";
    executePraatScript(script, scriptArguments);
}

void PluginProsobox5::MakeIFtier(CorpusCommunication *com)
{
    if (com->annotationsCount() == 0)
        return;
    QString basePath = com->repository()->files()->basePath();
    QString filenameAnnotation = basePath + "/" + com->annotation(0)->filename();
    // Execute PromGrad
    QString appPath = QCoreApplication::applicationDirPath();
    QString script = appPath + "/tools/various/make_if_tier_from_lex3.praat";
    QStringList scriptArguments;

    scriptArguments << filenameAnnotation << "phones" << "syll" << "if" << "tok-min" << "AP";
    executePraatScript(script, scriptArguments);
}
