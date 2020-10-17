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

QString SophieScripts::addPhoneticAnnotationToTokens(Praaline::Core::CorpusCommunication *com)
{
    QString ret;
    if (!com) return "Error: No communication";
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_segment = tiers->getIntervalTierByName("segment");
            if (!tier_segment) { ret.append("No segment tier\n"); continue; }
            IntervalTier *tier_tok_min = tiers->getIntervalTierByName("tok_min");
            if (!tier_tok_min) { ret.append("No tok_min tier\n"); continue; }

            foreach (Interval *tok_min, tier_tok_min->intervals()) {
                tok_min->setAttribute("ipa_model", "");
                tok_min->setAttribute("ipa_actual", "");
            }

            foreach (Interval *segment, tier_segment->intervals()) {
                QStringList ipa_model = segment->attribute("ipa_model").toString().split(" ");
                QStringList ipa_actual = segment->attribute("ipa_actual").toString().split(" ");
                QList<Interval *> tokens = tier_tok_min->getIntervalsContainedIn(segment);
                for (int i = 0; (i < tokens.count()) && (i < ipa_model.count()); ++i)
                    tokens[i]->setAttribute("ipa_model", ipa_model[i]);
                for (int i = 0; (i < tokens.count()) && (i < ipa_actual.count()); ++i)
                    tokens[i]->setAttribute("ipa_actual", ipa_actual[i]);
            }

            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_tok_min);
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}
















