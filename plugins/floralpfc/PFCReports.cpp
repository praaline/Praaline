#include <QString>
#include <QList>
#include <QPointer>
#include <QMap>
#include <QRegularExpression>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/datastore/CorpusRepository.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
using namespace Praaline::Core;

#include "WordAlign.h"
#include "PFCReports.h"

struct PFCReportsData {
    int i;
};

PFCReports::PFCReports() : d(new PFCReportsData)
{
}

PFCReports::~PFCReports()
{
    delete d;
}

QString PFCReports::corpusCoverageStatistics(QPointer<Corpus> corpus)
{
    QString ret;
    if (!corpus) return "No corpus";
    QString sep = "\t";
    foreach (QPointer<CorpusCommunication> com, corpus->communications()) {
        QString ID = com->ID();
        QString region = ID.left(3);

        QString task;
        if      (ID.right(1) == "m") task = "WORDS";
        else if (ID.right(1) == "t") task = "TEXT";
        else if (ID.right(1) == "g") task = "GUIDED";
        else if (ID.right(1) == "l") task = "LIBRE";
        else                         task = "OTHER";
        com->setProperty("task", task);

        RealTime totalDuration;
        int countRecordings = com->recordingIDs().count();
        foreach (QPointer<CorpusRecording> rec, com->recordings()) {
            totalDuration = totalDuration + rec->duration();
        }

        int countAnnotations = com->annotationIDs().count();

        ret.append(region).append(sep).append(ID).append(sep).append(task).append(sep);
        ret.append(QString::number(countRecordings)).append(sep).append(QString::number(countAnnotations)).append(sep);
        ret.append(QString::number(totalDuration.toDouble())).append("\n");
    }
    return ret;
}

QString PFCReports::reportCorrections(QPointer<Praaline::Core::CorpusCommunication> com)
{
    if (!com) return "No Communication";
    QString ret;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        AnnotationTierGroup *tiers = com->repository()->annotations()->
                getTiers(annot->ID(), annot->ID(), QStringList() << "transcription" << "transcription_original");
        IntervalTier *transcription = tiers->getIntervalTierByName("transcription");
        if (!transcription) continue;
        IntervalTier *transcription_original = tiers->getIntervalTierByName("transcription_original");
        if (!transcription_original) continue;
        if (transcription->count() != transcription_original->count())
            return com->ID() + " Changed number of transcription intervals!" ;
        for (int i = 0; i < transcription->count(); ++i) {
            Interval *original = transcription_original->at(i);
            Interval *modified = transcription->at(i);
            if (original->isPauseSilent() && modified->isPauseSilent()) continue;
            QStringList tokensOriginal = original->text().split(" ", QString::SkipEmptyParts);
            QStringList tokensModified = modified->text().split(" ", QString::SkipEmptyParts);
            QStringList liaisonOriginal = original->attribute("liaison").toString().split(" ", QString::SkipEmptyParts);
            QStringList liaisonModified = modified->attribute("liaison").toString().split(" ", QString::SkipEmptyParts);
            QStringList schwaOriginal = original->attribute("schwa").toString().split(" ", QString::SkipEmptyParts);
            QStringList schwaModified = modified->attribute("schwa").toString().split(" ", QString::SkipEmptyParts);
            WordAlign alignTokens;  alignTokens.align(tokensOriginal, tokensModified);
            WordAlign alignLiaison; alignLiaison.align(liaisonOriginal, liaisonModified);
            WordAlign alignSchwa;   alignSchwa.align(schwaOriginal, schwaModified);
            if (alignTokens.WER() > 0.0) {
                ret.append(com->ID()).append("\t").append(QString::number(i)).append("\t");
                ret.append(alignTokens.alignmentTextOriginal("latex")).append("\n");
                ret.append(alignTokens.alignmentTextModified("latex")).append("\n");
            }
        }
        delete tiers;
    }
    return ret;
}

