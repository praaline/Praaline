#include <QString>
#include <QHash>
#include <QPair>
#include <QFile>
#include <QTextStream>
#include <QSharedPointer>

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Interfaces/Praat/PraatTextGrid.h"
#include "PraalineCore/Interfaces/Phon/PhonTranscription.h"
using namespace Praaline::Core;

#include "SophieScripts.h"

struct SophieScriptsData {
    int i;
};

SophieScripts::SophieScripts() : d(new SophieScriptsData())
{
}

SophieScripts::~SophieScripts()
{
    delete d;
}

QString SophieScripts::importPhonAnnotation(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    if (com->recordingsCount() == 0) return "Error: No recordings";
    QString filepath = QString(com->recordings().first()->filePath()).replace(".wav", ".xml");
    PhonTranscription phonAnnotation;
    if (!phonAnnotation.load(filepath)) {
        return QString("Error reading Phon file %1").arg(filepath);
    }
    // Create annotation if not already present
    if (com->annotationsCount() == 0) {
        com->addAnnotation(new CorpusAnnotation(com->ID()));
    }
    QString speakerID = phonAnnotation.participantIDs().first();
    if (speakerID.isEmpty()) speakerID = com->ID();

    QList<Interval *> intv_tokens;
    foreach (PhonTranscription::Segment seg, phonAnnotation.segments()) {
        QString text = seg.orthography.join(" ").trimmed();
        intv_tokens << new Interval(seg.startTime, seg.startTime + seg.duration, text);
    }
    IntervalTier *tier_transcription = new IntervalTier("transcription", intv_tokens);
    com->repository()->annotations()->saveTier(com->ID(), speakerID, tier_transcription);

    return ret;
}
