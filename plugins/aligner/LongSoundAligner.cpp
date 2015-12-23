#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QPointer>
#include "LongSoundAligner.h"
#include "pncore/corpus/corpus.h"
#include "pncore/annotation/annotationtiergroup.h"
#include "sphinx/sphinxrecogniser.h"

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
    d->tiername_auto_utterance = "auto_utterance";
    d->tiername_auto_hypseg = "auto_hypseg_%1";
    d->attributename_acoustic_model = "acoustic_model";
    d->attributename_language_model = "language_model";
}

LongSoundAligner::~LongSoundAligner()
{
    delete d;
}

// Annotation structure tools
bool LongSoundAligner::createRecognitionLevel(QPointer<Corpus> corpus, int recognitionStep)
{
    if (!corpus) return false;
    QString levelID = QString(d->tiername_auto_hypseg).arg(recognitionStep);
    if (corpus->annotationStructure()->hasLevel(levelID)) return true;
    AnnotationStructureLevel *level = new AnnotationStructureLevel(levelID, AnnotationStructureLevel::IndependentLevel,
                                                                   levelID, QString("Long sound alignment step %1").arg(recognitionStep));
    if (!corpus->datastoreAnnotations()->createAnnotationLevel(level)) return false;
    corpus->annotationStructure()->addLevel(level);
    AnnotationStructureAttribute *attr;
    attr = new AnnotationStructureAttribute("text_from_dic", "Text from Dic", "Text from pronunciation dictionary");
    if (!corpus->datastoreAnnotations()->createAnnotationAttribute(levelID, attr)) return false;
    level->addAttribute(attr);
    attr = new AnnotationStructureAttribute("acoustic_score", "Acoustic score", "Acoustic score for this word", "double", 0);
    if (!corpus->datastoreAnnotations()->createAnnotationAttribute(levelID, attr)) return false;
    level->addAttribute(attr);
    attr = new AnnotationStructureAttribute("lm_score", "LM score", "Language model score for this word", "double", 0);
    if (!corpus->datastoreAnnotations()->createAnnotationAttribute(levelID, attr)) return false;
    level->addAttribute(attr);
    corpus->save();
    return true;
}

// Long sound alignment steps
bool LongSoundAligner::createUtterancesFromProsogramAutosyll(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com)
{
    if (!com) return false;
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
        // Normally, there should be one speaker (with a speaker ID equal to the communication ID), created by
        // running Prosogram in automatic segmentation mode.
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
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
            corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_auto_syll);
            tier_auto_syll->mergeIdenticalAnnotations("_");
            QList<Interval *> pauses;
            foreach (Interval *syll, tier_auto_syll->intervals())
                if (syll->text() == "_") pauses << syll;
            QList<Interval *> longPauses;
            foreach (Interval *pause ,pauses) {
                if (pause->duration().toDouble() > 0.300) {
                    Interval *longPause = new Interval(pause);
                    longPause->setText("_");
                    longPauses << longPause;
                }
            }
            // Creating a new tier and passing only the annotated pause intervals will create blank intervals
            // between them - these will be the automatically-detected utternaces.
            IntervalTier *tier_auto_utterance = new IntervalTier(d->tiername_auto_utterance,
                                                                 tier_auto_syll->tMin(), tier_auto_syll->tMax(),
                                                                 longPauses);
            tiers->addTier(tier_auto_utterance);
            corpus->datastoreAnnotations()->saveTier(annot->ID(), speakerID, tier_auto_utterance);
        }
        qDeleteAll(tiersAll);
    }
    return true;
}

bool LongSoundAligner::recognise(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com, int recognitionStep)
{
    QPointer<SphinxRecogniser> sphinx = new SphinxRecogniser(this);
    sphinx->setAttributeNames(d->attributename_acoustic_model, d->attributename_language_model);
    if (recognitionStep == 0) {
        sphinx->setUseMLLR(false);
        sphinx->setUseSpecialisedLM(true);
    } else {
        sphinx->setUseMLLR(true);
        sphinx->setUseSpecialisedLM(true);
    }
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    if (!com) return false;
    if (!com->hasRecordings()) return false;
    QPointer<CorpusRecording> rec = com->recordings().first();
    if (!rec) return false;
    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = corpus->datastoreAnnotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_auto_utterance = tiers->getIntervalTierByName(d->tiername_auto_utterance);
            if (!tier_auto_utterance) continue;
            QList<Interval *> utterances;
            QList<Interval *> segmentation;
            foreach (Interval *intv, tier_auto_utterance->intervals()) {
                if (intv->text() != "_") utterances << intv;
            }
            sphinx->recogniseUtterances_MFC(com, rec->ID(), utterances, segmentation);
            // Segment (word) hypotheses
            IntervalTier *tier_auto_hypseg = new IntervalTier(QString(d->tiername_auto_hypseg).arg(recognitionStep),
                                                              RealTime(0, 0), rec->duration(), segmentation);
            foreach (Interval *intv, tier_auto_hypseg->intervals()) {
                intv->setAttribute("text_from_dic", intv->text());
                intv->setText(intv->text().replace("<sil>", "").replace("[b]", "").replace("[i]", "")
                              .replace("(1)", "").replace("(2)", "").replace("(3)", "").replace("(4)", "")
                              .replace("<s>", "_").replace("</s>", "_"));
            }
            tier_auto_hypseg->fillEmptyAnnotationsWith("_");
            tier_auto_hypseg->mergeIdenticalAnnotations("_");
            // Save
            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_auto_hypseg);
            corpus->datastoreAnnotations()->saveTier(annotationID, speakerID, tier_auto_utterance);
        }
        qDeleteAll(tiersAll);
    }
    return true;
}


