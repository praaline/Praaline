#include <QSharedPointer>
#include <QString>
#include <QMap>
#include "pncore/corpus/Corpus.h"
#include "pncore/datastore/AnnotationDatastore.h"
#include "pncore/annotation/AnnotationTierGroup.h"
#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/PointTier.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"
#include "pncore/interfaces/praat/PraatPitchFile.h"
using namespace Praaline::Core;

#include "Rhapsodie.h"

Rhapsodie::Rhapsodie()
{

}

QString Rhapsodie::updateSyllables(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Open textgrid
        QSharedPointer<AnnotationTierGroup> txg(new AnnotationTierGroup());
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/TextGrids-fev2013/";
        if (!PraatTextGrid::load(path + annotationID + "-Pro.TextGrid" , txg.data())) {
            ret.append("TextGrid not found"); continue;
        }
        // Get Rhapsodie TGs syllable, prom, hes, contour
        IntervalTier *rh_syll = txg->getIntervalTierByName("syllabe");
        if (!rh_syll) { ret.append(annotationID).append("\tNo tier syll"); continue; }
        IntervalTier *rh_prom = txg->getIntervalTierByName("prom");
        if (!rh_prom) { ret.append(annotationID).append("\tNo tier prom"); continue; }
        IntervalTier *rh_hes = txg->getIntervalTierByName("hes");
        if (!rh_hes) { ret.append(annotationID).append("\tNo tier hes"); continue; }
        IntervalTier *rh_contour = txg->getIntervalTierByName("contour");
        // contour is optional
        if (rh_syll->count() != rh_prom->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - prom %2").arg(rh_syll->count()).arg(rh_prom->count())); continue;
        }
        if (rh_syll->count() != rh_hes->count()) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - hes %2").arg(rh_syll->count()).arg(rh_hes->count())); continue;
        }
        if ((rh_contour) && (rh_syll->count() != rh_contour->count())) {
            ret.append(annotationID).append(QString("\tCount mismatch syll %1 - contour %2").arg(rh_syll->count()).arg(rh_contour->count())); continue;
        }
        // Move all data to rh_syll
        for (int i = 0; i < rh_syll->count(); ++i) {
            rh_syll->at(i)->setAttribute("rh_prom", rh_prom->at(i)->text());
            rh_syll->at(i)->setAttribute("rh_hes", rh_hes->at(i)->text());
            if (rh_contour) rh_syll->at(i)->setAttribute("rh_contour", rh_contour->at(i)->text());
        }
        // Update syllables in tiers, per speaker, note progress
        tiersAll = com->corpus()->repository()->annotations()->getTiersAllSpeakers(annotationID, QStringList() << "syll");
        foreach (QString speakerID, tiersAll.keys()) {
            bool OK = true;
            QPointer<AnnotationTierGroup> tiers = tiersAll.value(speakerID);
            if (!tiers) continue;
            IntervalTier *tier_syll = tiers->getIntervalTierByName("syll");
            if (!tier_syll) continue;
            // Check syllable correspondance
            foreach (Interval *syll, tier_syll->intervals()) {
                if (syll->isPauseSilent()) continue;
                Interval *rhs = rh_syll->intervalAtTime(syll->tCenter());
                if (rhs->text() != syll->text()) {
                    ret.append(annotationID).append("\t").append(speakerID).append("\t");
                    ret.append(QString::number(syll->tMin().toDouble())).append("\t");
                    ret.append(syll->text()).append("\t").append(rhs->text()).append("\n");
                    OK = false;
                }
            }
            if (OK) {
                // Update syllables
                foreach (Interval *syll, tier_syll->intervals()) {
                    if (syll->isPauseSilent()) {
                        syll->setAttribute("rh_prom", "_");
                        syll->setAttribute("rh_hes", "_");
                        syll->setAttribute("rh_contour", "_");
                        continue;
                    }
                    Interval *rhs = rh_syll->intervalAtTime(syll->tCenter());
                    syll->setAttribute("rh_prom", rhs->attribute("rh_prom"));
                    syll->setAttribute("rh_hes", rhs->attribute("rh_hes"));
                    syll->setAttribute("rh_contour", rhs->attribute("rh_contour"));
                }
                com->corpus()->repository()->annotations()->saveTier(annotationID, speakerID, tier_syll);
                ret.append(annotationID).append("\t").append(speakerID).append(QString("\t%1\tUpdated\n").arg(tier_syll->count()));
            }
        }
        qDeleteAll(tiersAll);
    }
    return ret;
}

QString Rhapsodie::loadPitch(QPointer<CorpusCommunication> com)
{
    QString ret;
    if (!com) return "Error";
    if (!com->corpus()) return "Error";
    QMap<QString, QPointer<AnnotationTierGroup> > tiersAll;

    foreach (QPointer<CorpusAnnotation> annot, com->annotations()) {
        if (!annot) continue;
        QString annotationID = annot->ID();
        // Load pitch file
        QString path = QDir::homePath() + "/Dropbox/CORPORA/Rhapsodie_files/Rhap-courbes-liss/";
        PraatPitch pitch;
        if (!PraatPitchFile::load(path + annotationID + "-liss.Pitch", pitch)) {
            ret.append("Pitch not found"); continue;
        }

        PointTier *pitch_tier = new PointTier("pitch_smooth");
        RealTime t = pitch.tstart;
        QList<Point *> points;
        for (int i = 0; i < pitch.frames.count(); ++i) {
            PraatPitchFrame frame = pitch.frames.at(i);
            t = t + pitch.dx;
            Point *p = new Point(t);
            p->setAttribute("frequency", frame.candidates.first().frequency);
            points << p;
        }
        pitch_tier->addPoints(points);
        com->repository()->annotations()->saveTier(annotationID, "pitch", pitch_tier);
        ret.append(annotationID).append(" Pitch imported");
    }
    return ret;
}
