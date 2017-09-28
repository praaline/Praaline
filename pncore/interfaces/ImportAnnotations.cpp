#include <QString>
#include <QList>
#include <QMap>

#include "corpus/Corpus.h"
#include "corpus/CorpusCommunication.h"
#include "corpus/CorpusAnnotation.h"
#include "datastore/CorpusRepository.h"
#include "datastore/AnnotationDatastore.h"
#include "annotation/AnnotationTierGroup.h"
#include "annotation/IntervalTier.h"
#include "annotation/PointTier.h"

#include "ImportAnnotations.h"

namespace Praaline {
namespace Core {

ImportAnnotations::ImportAnnotations()
{

}

// static
ImportAnnotations::SpeakerPolicy ImportAnnotations::speakerPolicyFromInt(int i)
{
    if      (i == 0) return ImportAnnotations::SpeakerPolicySingle;
    else if (i == 1) return ImportAnnotations::SpeakerPolicyTierNames;
    else if (i == 2) return ImportAnnotations::SpeakerPolicyIntervals;
    else if (i == 3) return ImportAnnotations::SpeakerPolicyPrimaryAndSecondary;
    return ImportAnnotations::SpeakerPolicySingle;
}

// static
bool ImportAnnotations::importBasic(Corpus *corpus, CorpusCommunication *com, CorpusAnnotation *annot,
                                    SpeakerPolicy speakerPolicy, QString speakerID,
                                    AnnotationTierGroup *inputTiers, QList<TierCorrespondance> &correspondances)
{
    bool replaceExistingTiers = false;

    if (!corpus) return false;
    if (!corpus->repository()) return false;
    if (!corpus->repository()->annotations()) return false;

    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;
    QString spkID;

    // Step 0: Get tiers for current speaker if they already exist
    tiersAll = corpus->repository()->annotations()->getTiersAllSpeakers(annot->ID());
    // Step 1a: Add a tier for each level and speaker
    foreach (TierCorrespondance c, correspondances) {
        // Run through the level correspondences only at this stage
        if (c.annotationLevelID.isEmpty() || !c.annotationAttributeID.isEmpty()) continue;
        // Get the input tier and adjust its properties
        AnnotationTier *tierL = inputTiers->tier(inputTiers->getTierIndexByName(c.tierName));
        if (!tierL) continue;
        tierL->setName(c.annotationLevelID);
        tierL->setProperty("inputTierName", c.tierName);
        // Speaker name
        if (speakerPolicy == SpeakerPolicyTierNames)
            spkID = c.tierName;
        else
            spkID = speakerID;
        // If there is no AnnotationTierGroup for the specified speaker, create one
        if (!tiersAll.contains(spkID))
            tiersAll.insert(spkID, new AnnotationTierGroup());
        // Add the tier for the specified speaker. The option replaceExistingTiers defines what will happen if the tier
        // already exists: true = it will be replaced by the tier in the imported annotation vs false = the existing tier
        // will be kept and it will be attempted to correlate its annotations with the imported annotations
        if (tiersAll[spkID]->tierNames().contains(c.annotationLevelID)) {
            if (replaceExistingTiers) tiersAll[spkID]->addTierReplacing(tierL);
        } else {
            tiersAll[spkID]->addTier(tierL);
        }
        // Add the speakerto the corpus if needed
        if (!corpus->hasSpeaker(spkID))
            corpus->addSpeaker(new CorpusSpeaker(spkID));
    }
    // Step 1b: Add participations of speakers
    foreach (QString spkID, tiersAll.keys()) {
        if (!corpus->hasParticipation(com->ID(), spkID)) {
            corpus->addParticipation(com->ID(), spkID, QObject::tr("Participant"));
        }
    }
    // Step 2: Update attributes for each level and speaker
    foreach (TierCorrespondance c, correspondances) {
        if (c.annotationLevelID.isEmpty() || c.annotationAttributeID.isEmpty()) continue;
        AnnotationTier *tierA = inputTiers->tier(c.tierName);
        if (!tierA) continue;
        // Speaker ID as defined by the policy
        if (speakerPolicy == SpeakerPolicyTierNames)
            spkID = c.tierName;
        else
            spkID = speakerID;
        if (!tiersAll.contains(spkID)) continue;
        AnnotationTierGroup *tiers = tiersAll.value(spkID);
        // corresponding level
        AnnotationTier *tierL = tiers->tier(c.annotationLevelID);
        if (!tierL) continue;
        // apply correspondance
        if (tierL->tierType() == AnnotationTier::TierType_Intervals && tierA->tierType() == AnnotationTier::TierType_Intervals) {
            IntervalTier *tierLI = qobject_cast<IntervalTier *>(tierL);
            IntervalTier *tierAI = qobject_cast<IntervalTier *>(tierA);
            for (int i = 0; i < tierLI->count(); ++i) {
                Interval *intv = tierAI->intervalAtTime(tierLI->interval(i)->tCenter());
                if (intv) tierLI->interval(i)->setAttribute(c.annotationAttributeID, intv->text());
            }
        }
        else if (tierL->tierType() == AnnotationTier::TierType_Intervals && tierA->tierType() == AnnotationTier::TierType_Points) {
            IntervalTier *tierLI = qobject_cast<IntervalTier *>(tierL);
            PointTier *tierAP = qobject_cast<PointTier *>(tierA);
            for (int i = 0; i < tierLI->count(); ++i) {
                Point *pnt = tierAP->pointAtTime(tierLI->interval(i)->tCenter());
                if (pnt) tierLI->interval(i)->setAttribute(c.annotationAttributeID, pnt->text());
            }
        }
        else if (tierL->tierType() == AnnotationTier::TierType_Points && tierA->tierType() == AnnotationTier::TierType_Intervals) {
            PointTier *tierLP = qobject_cast<PointTier *>(tierL);
            IntervalTier *tierAI = qobject_cast<IntervalTier *>(tierA);
            for (int i = 0; i < tierLP->count(); ++i) {
                Interval *intv = tierAI->intervalAtTime(tierLP->point(i)->time());
                if (intv) tierLP->point(i)->setAttribute(c.annotationAttributeID, intv->text());
            }
        }
        else if (tierL->tierType() == AnnotationTier::TierType_Points && tierA->tierType() == AnnotationTier::TierType_Points) {
            PointTier *tierLP = qobject_cast<PointTier *>(tierL);
            PointTier *tierAP = qobject_cast<PointTier *>(tierA);
            for (int i = 0; i < tierLP->count(); ++i) {
                Point *pnt = tierAP->pointAtTime(tierLP->point(i)->time());
                if (pnt) tierLP->point(i)->setAttribute(c.annotationAttributeID, pnt->text());
            }
        }
    }
    corpus->repository()->annotations()->saveTiersAllSpeakers(annot->ID(), tiersAll);
    qDeleteAll(tiersAll);

    return true;
}


} // namespace Core
} // namespace Praaline
