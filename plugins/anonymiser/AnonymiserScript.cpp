#include <QObject>
#include <QDebug>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "PraalineCore/Corpus/CorpusRecording.h"
#include "PraalineCore/Corpus/CorpusAnnotation.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
#include "PraalineCore/Interfaces/Praat/PraatPointTierFile.h"
using namespace Praaline::Core;

#include "AnnotationPluginPraatScript.h"
#include "AnonymiserScript.h"

AnonymiserScript::AnonymiserScript(QObject *parent) : AnnotationPluginPraatScript(parent),
    pitchAnalysisDuration(30.0), pitchAnalysisTimestep(0.01), automaticMinMaxF0(true), minF0(60.0), maxF0(700.0), scaleIntensity(1.0)
{
}

bool fileMoveReplacingDestination(const QString &source, const QString &dest)
{
    if (QFile::exists(dest)) QFile::remove(dest);
    return QFile::rename(source, dest);
}

void AnonymiserScript::run(Praaline::Core::CorpusRecording *rec, Praaline::Core::IntervalTier *tierWithAnonymiserLabels, QString anonymisedFilename)
{
    if (!rec) return;
    if (!tierWithAnonymiserLabels) return;

    // Create a temporary directory
    QTemporaryDir dirTemp;
    if (!dirTemp.isValid()) return;
    QString tempDirectory = dirTemp.path(); // returns the unique directory path
    if (!tempDirectory.endsWith("/")) tempDirectory.append("/");

    // Copy the recording file into temp+.wav
    QFileInfo info(rec->filePath());
    QString filenameBase = info.baseName();
    QString filenameTempRec = QString("%1.wav").arg(filenameBase);
    QFile::copy(rec->filePath(), tempDirectory + filenameTempRec);

    // Create matching textgrid with buzz tier
    AnnotationTierGroup *tiers = new AnnotationTierGroup(this);
    tiers->addTier(tierWithAnonymiserLabels->clone());
    PraatTextGrid::save(tempDirectory + QString("%1.TextGrid").arg(filenameBase), tiers);
    delete tiers;

    // Execute anonymisation Praat script
    QString script = QDir::homePath() + "/Praaline/plugins/anonymiser/praaline_anonymiser.praat";
    QStringList scriptArguments;

    scriptArguments << tempDirectory << ".wav" << ".TextGrid" << "_anon.wav" << "WAV" << "1" << "buzz" <<
                       QString::number(pitchAnalysisDuration) << QString::number(pitchAnalysisTimestep) <<
                       ((automaticMinMaxF0) ? "yes" : "no") <<
                       QString::number(minF0) << QString::number(maxF0) <<
                       QString::number(scaleIntensity) << "yes";
    // Run Praat script
    executePraatScript(script, scriptArguments);

    // Move file from temporary directory to the desired location
    fileMoveReplacingDestination(tempDirectory + QString("%1_anon.wav").arg(filenameBase), anonymisedFilename);
}
