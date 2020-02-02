#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QMutex>
#include "LongSoundAligner.h"

#include "PraalineCore/Corpus/Corpus.h"
#include "PraalineCore/Annotation/AnnotationTierGroup.h"
#include "PraalineCore/Annotation/IntervalTier.h"
#include "PraalineCore/Datastore/CorpusRepository.h"
#include "PraalineCore/Datastore/AnnotationDatastore.h"
#include "PraalineCore/Structure/AnnotationStructure.h"
using namespace Praaline::Core;

#include "PraalineASR/Sphinx/SphinxRecogniser.h"
using namespace Praaline::ASR;

struct LongSoundAlignerData
{
    QString tiername_auto_syll;
    QString tiername_auto_syll_nucl;
    QString tiername_auto_utterance;
    QString tiername_auto_hypseg;
    QString attributename_acoustic_model;
    QString attributename_language_model;
};

LongSoundAligner::LongSoundAligner(QObject *parent) :
    QObject(parent), d(new LongSoundAlignerData)
{
    d->tiername_auto_syll = "auto_syll";
    d->tiername_auto_syll_nucl = "auto_syll_nucl";
    d->tiername_auto_utterance = "transcription"; //"auto_utterance";
    d->tiername_auto_hypseg = "auto_hypseg_%1";
    d->attributename_acoustic_model = "acoustic_model";
    d->attributename_language_model = "language_model";
}

LongSoundAligner::~LongSoundAligner()
{
    delete d;
}

// Annotation structure tools
bool LongSoundAligner::createRecognitionLevel(CorpusRepository *repository, int recognitionStep)
{
    if (!repository) return false;
    QString levelID = QString(d->tiername_auto_hypseg).arg(recognitionStep);
    if (repository->annotationStructure()->hasLevel(levelID)) return true;
    AnnotationStructureLevel *level = new AnnotationStructureLevel(levelID, AnnotationStructureLevel::IndependentIntervalsLevel,
                                                                   levelID, QString("Long sound alignment step %1").arg(recognitionStep));
    if (!repository->annotations()->createAnnotationLevel(level)) return false;
    repository->annotationStructure()->addLevel(level);
    AnnotationStructureAttribute *attr;
    attr = new AnnotationStructureAttribute("text_from_dic", "Text from Dic", "Text from pronunciation dictionary");
    if (!repository->annotations()->createAnnotationAttribute(levelID, attr)) return false;
    level->addAttribute(attr);
    attr = new AnnotationStructureAttribute("acoustic_score", "Acoustic score", "Acoustic score for this word", DataType::Double);
    if (!repository->annotations()->createAnnotationAttribute(levelID, attr)) return false;
    level->addAttribute(attr);
    attr = new AnnotationStructureAttribute("lm_score", "LM score", "Language model score for this word", DataType::Double);
    if (!repository->annotations()->createAnnotationAttribute(levelID, attr)) return false;
    level->addAttribute(attr);
    return true;
}

// Long sound alignment steps
bool LongSoundAligner::createUtterancesFromProsogramAutosyll(CorpusCommunication *com)
{
    if (!com) return false;
    SpeakerAnnotationTierGroupMap tiersAll;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        // Normally, there should be one speaker (with a speaker ID equal to the communication ID), created by
        // running Prosogram in automatic segmentation mode.
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_auto_syll = tiers->getIntervalTierByName(d->tiername_auto_syll);
            if (!tier_auto_syll) continue;
            IntervalTier *tier_auto_syll_nucl = tiers->getIntervalTierByName(d->tiername_auto_syll_nucl);
            if (!tier_auto_syll_nucl) continue;
            foreach (Interval *syll, tier_auto_syll->intervals()) {
                QList<Interval *> nuclei = tier_auto_syll_nucl->getIntervalsContainedIn(syll);
                foreach (Interval *nucleus, nuclei) {
                    if (nucleus->text() == "a") syll->setText("syl");
                }
                if (syll->text() != "syl") syll->setText("_");
            }
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_auto_syll);
            tier_auto_syll->mergeIdenticalAnnotations("_");
            QList<Interval *> pauses;
            foreach (Interval *syll, tier_auto_syll->intervals())
                if (syll->text() == "_") pauses << syll;
            QList<Interval *> longPauses;
            foreach (Interval *pause ,pauses) {
                if (pause->duration().toDouble() > 0.300) {
                    Interval *longPause = pause->clone();
                    longPause->setText("_");
                    longPauses << longPause;
                }
            }
            // Creating a new tier and passing only the annotated pause intervals will create blank intervals
            // between them - these will be the automatically-detected utternaces.
            IntervalTier *tier_auto_utterance = new IntervalTier(d->tiername_auto_utterance, longPauses,
                                                                 tier_auto_syll->tMin(), tier_auto_syll->tMax());
            tiers->addTier(tier_auto_utterance);
            com->repository()->annotations()->saveTier(annot->ID(), speakerID, tier_auto_utterance);
        }
        qDeleteAll(tiersAll);
    }
    return true;
}

bool LongSoundAligner::recognise(CorpusCommunication *com, int recognitionStep)
{
    static QMutex mutex;

    QPointer<SphinxRecogniser> sphinx = new SphinxRecogniser(this);
    sphinx->setAttributeNames(d->attributename_acoustic_model, d->attributename_language_model, "MLLR");
    if (recognitionStep == 0) {
        sphinx->setUseMLLRMatrixFromAttribute(false);
        sphinx->setUseLanguageModelFromAttribute(false);
    } else {
        sphinx->setUseMLLRMatrixFromAttribute(true);
        sphinx->setUseLanguageModelFromAttribute(true);
    }
    SpeakerAnnotationTierGroupMap tiersAll;
    if (!com) return false;
    if (!com->repository()) return false;
    if (!com->hasRecordings()) {
        com->setProperty("LSA_status", "NoRecordings");
        return false;
    }
    CorpusRecording *rec = com->recordings().first();
    if (!rec) return false;
    foreach (CorpusAnnotation *annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        mutex.lock();
        tiersAll = com->repository()->annotations()->getTiersAllSpeakers(annotationID);
        mutex.unlock();
        foreach (QString speakerID, tiersAll.keys()) {
            AnnotationTierGroup *tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_auto_utterance = tiers->getIntervalTierByName(d->tiername_auto_utterance);
            if (!tier_auto_utterance) continue;
            QList<Interval *> utterances;
            QList<Interval *> segmentation;
            foreach (Interval *intv, tier_auto_utterance->intervals()) {
                if (intv->tMin() > RealTime(300, 0)) break; // first 300 seconds limit
                if (intv->text() != "_") utterances << intv;
            }
            sphinx->recogniseUtterances_MFC(com, rec->ID(), utterances, segmentation);
            // Segment (word) hypotheses
            IntervalTier *tier_auto_hypseg = new IntervalTier(QString(d->tiername_auto_hypseg).arg(recognitionStep),
                                                              segmentation, RealTime(0, 0), rec->duration());
            foreach (Interval *intv, tier_auto_hypseg->intervals()) {
                intv->setAttribute("text_from_dic", intv->text());
                intv->setText(intv->text().replace("<sil>", "").replace("[b]", "").replace("[i]", "")
                              .replace("(1)", "").replace("(2)", "").replace("(3)", "").replace("(4)", "")
                              .replace("<s>", "_").replace("</s>", "_"));
            }
            tier_auto_hypseg->fillEmptyWith("", "_");
            tier_auto_hypseg->mergeIdenticalAnnotations("_");
            // Save
            mutex.lock();
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_auto_hypseg);
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_auto_utterance);
            mutex.unlock();
        }
        qDeleteAll(tiersAll);
    }
    com->setProperty("LSA_status", "First300");
    return true;
}


