#include <QString>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

#include "LOCASF.h"

LOCASF::LOCASF()
{

}

QString LOCASF::noteProsodicBoundaryOnSyll(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        tiersAll = com->corpus()->repository()->annotations()->getTiersAllSpeakers(annotationID);
        foreach (QString speakerID, tiersAll.keys()) {
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            IntervalTier *tier_tokmwu = tiers->getIntervalTierByName("tok_mwu");
            IntervalTier *tier_sequence = tiers->getIntervalTierByName("sequence");
            IntervalTier *tier_rection = tiers->getIntervalTierByName("rection");

            for (int isyll = 0; isyll < tier_syll->count(); ++isyll) {
                Interval *syll = tier_syll->interval(isyll);
                // Expert annotation: extract boundary type (only ///, //, hesi and 0).
                // --------------------------------------------------------------------------------------------------------
                QString expertBoundary = syll->attribute("boundary").toString();
                QString expertBoundaryType = "0";
                if      (expertBoundary.contains("///"))   expertBoundaryType = "///";
                else if (expertBoundary.contains("//"))    expertBoundaryType = "//";
                else if (expertBoundary.contains("hesi"))  expertBoundaryType = "hesi";
                // else if (expertBoundary.contains("#"))     expertBoundaryType = "#";
                if (syll->isPauseSilent()) expertBoundaryType = "";
                syll->setAttribute("boundaryType", expertBoundaryType);

                // Syntax
                // --------------------------------------------------------------------------------------------------------
                Interval *tok_mwu = (tier_tokmwu) ? tier_tokmwu->intervalAtTime(syll->tCenter()) : 0;
                Interval *sequence = (tier_sequence) ? tier_sequence->intervalAtTime(syll->tCenter()) : 0;
                Interval *rection = (tier_rection) ? tier_rection->intervalAtTime(syll->tCenter()) : 0;

                RealTime t10ns = RealTime::fromNanoseconds(10);
                Interval *tok_mwu_next = (tier_tokmwu) ? tier_tokmwu->intervalAtTime(syll->tMax() + t10ns) : 0;
                Interval *sequence_next = (tier_sequence) ? tier_sequence->intervalAtTime(syll->tMax() + t10ns) : 0;
                Interval *rection_next = (tier_rection) ? tier_rection->intervalAtTime(syll->tMax() + t10ns) : 0;

                QString tok_mwu_text = (tok_mwu) ? tok_mwu->text() : "";
                if (tok_mwu_next) tok_mwu_text = tok_mwu_text.append("|").append(tok_mwu_next->text());
                QString sequence_text = (sequence) ? sequence->text() : "";
                if (sequence_next) sequence_text = sequence_text.append("|").append(sequence_next->text());
                QString rection_text = (rection) ? rection->text() : "";
                if (rection_next) rection_text = rection_text.append("|").append(rection_next->text());

                QString syntacticBoundaryType = "0";
                if ((tok_mwu) && (tok_mwu != tok_mwu_next)) syntacticBoundaryType = "MWU";
                if ((sequence) && (sequence != sequence_next)) syntacticBoundaryType = "SEQ";
                if ((rection) && (rection != rection_next)) {
                    syntacticBoundaryType = "REC";
                    if (rection_text.contains("md|")) syntacticBoundaryType = "MD";
                }
                if (syll->isPauseSilent()) syntacticBoundaryType = "";
                syll->setAttribute("boundarySyntactic", syntacticBoundaryType);
            }
            com->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
        }
        qDeleteAll(tiersAll);
        ret = ret.append(annotationID).append("\tUpdated syll tier with boundary information.");
    }
    return ret;
}
