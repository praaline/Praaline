#include <QString>
#include <QList>
#include <QPair>
#include <QMap>
#include <QPointer>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

#include "pncore/corpus/Corpus.h"
#include "pncore/corpus/CorpusCommunication.h"
#include "pncore/corpus/CorpusParticipation.h"
using namespace Praaline::Core;

#include "svcore/data/model/SparseTimeValueModel.h"
#include "AnnotationGridPointModel.h"
#include "ProsogramTonalSegmentModel.h"
#include "ProsogramModel.h"

#include "pncore/interfaces/praat/PraatPointTierFile.h"
#include "pncore/interfaces/praat/PraatTextGrid.h"

struct ProsogramModelData {
    ProsogramModelData () : sampleRate(0), segments(0), phones(0), sylls(0),
        vuvregions(0), pitch(0), intensity(0) {}

    sv_samplerate_t sampleRate;
    QPointer<ProsogramTonalSegmentModel> segments;
    QPointer<AnnotationGridPointModel> phones;
    QPointer<AnnotationGridPointModel> sylls;
    QPointer<AnnotationGridPointModel> vuvregions;
    QPointer<SparseTimeValueModel> pitch;
    QPointer<SparseTimeValueModel> intensity;
};

ProsogramModel::ProsogramModel(sv_samplerate_t sampleRate, QPointer<CorpusRecording> rec) :
    d(new ProsogramModelData)
{
    d->sampleRate = sampleRate;
    d->segments = new ProsogramTonalSegmentModel(sampleRate, 1, true);
    d->phones = new AnnotationGridPointModel(sampleRate, 1, true);
    d->sylls = new AnnotationGridPointModel(sampleRate, 1, true);
    d->vuvregions = new AnnotationGridPointModel(sampleRate, 1, true);
    d->pitch = new SparseTimeValueModel(sampleRate, 1, false);
    d->intensity = new SparseTimeValueModel(sampleRate, 1, false);

    if (!rec) return;
    QPointer<Corpus> corpus = rec->corpus();
    if (!corpus) return;
    QString prosoPath = rec->property("ProsogramPath").toString();
    if (prosoPath.isEmpty()) {
        QFileInfo info(rec->filePath());
        prosoPath = info.absoluteDir().absolutePath() + "/prosogram/";
    }
    QList<QPointer<CorpusParticipation> > participations = corpus->participationsForCommunication(rec->communicationID());
    if (participations.isEmpty()) {
        readProsogramFiles(sampleRate, "", d->segments,
                           prosoPath + QString("%1_nucl.TextGrid").arg(rec->ID()),
                           prosoPath + QString("%1_styl.PitchTier").arg(rec->ID()),
                           prosoPath + QString("%1.PitchTier").arg(rec->ID()),
                           prosoPath + QString("%1.IntensityTier").arg(rec->ID()));

    } else {
        foreach (QPointer<CorpusParticipation> participation, participations) {
            if (!participation) continue;
            readProsogramFiles(sampleRate, participation->speakerID(), d->segments,
                               prosoPath + QString("%1_%2_nucl.TextGrid").arg(rec->ID()).arg(participation->speakerID()),
                               prosoPath + QString("%1_%2_styl.PitchTier").arg(rec->ID()).arg(participation->speakerID()),
                               prosoPath + QString("%1_%2.PitchTier").arg(rec->ID()).arg(participation->speakerID()),
                               prosoPath + QString("%1_%2.IntensityTier").arg(rec->ID()).arg(participation->speakerID()));
        }
    }
}

bool ProsogramModel::readProsogramFiles(sv_samplerate_t sampleRate, const QString &speakerID,
                                        QPointer<ProsogramTonalSegmentModel> segments,
                                        const QString &filenameNuclei, const QString &filenameStylPitchTier,
                                        const QString &filenamePitchTier, const QString &filenameIntensityTier)
{
    QMap<RealTime, double> pitchStylised;
    if (!PraatPointTierFile::load(filenameStylPitchTier, pitchStylised))
        return false;
    QPointer<AnnotationTierGroup> tiers_nuclei = new AnnotationTierGroup();
    if (!PraatTextGrid::load(filenameNuclei, tiers_nuclei))
        return false;
    // Tonal segments
    IntervalTier *tier_pointer = tiers_nuclei->getIntervalTierByName("pointer");
    if (!tier_pointer) return false;
    foreach (Interval *intv, tier_pointer->intervals()) {
        bool ok = false;
        int index = intv->text().toInt(&ok);
        if (!ok) continue;
        double f0HzStart = pitchStylised.value(intv->tMin());
        double f0HzEnd = pitchStylised.value(intv->tMax());
        segments->addPoint(ProsogramTonalSegment(RealTime::realTime2Frame(intv->tMin(), sampleRate),
                                                 RealTime::realTime2Frame(intv->duration(), sampleRate),
                                                 speakerID, f0HzStart, f0HzEnd, index));
    }
    // Phones
    IntervalTier *tier_phone = tiers_nuclei->getIntervalTierByName("phone");
    if (tier_phone) {
        for (int i = 0; i < tier_phone->count(); ++i) {
            Interval *intv = tier_phone->interval(i);
            d->phones->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(intv->tMin(), sampleRate),
                                                    RealTime::realTime2Frame(intv->duration(), sampleRate),
                                                    speakerID, "phone", i, intv->text()));
        }
    }
    // Syllables
    IntervalTier *tier_syll = tiers_nuclei->getIntervalTierByName("syll");
    if (tier_syll) {
        for (int i = 0; i < tier_syll->count(); ++i) {
            Interval *intv = tier_syll->interval(i);
            d->sylls->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(intv->tMin(), sampleRate),
                                                   RealTime::realTime2Frame(intv->duration(), sampleRate),
                                                   speakerID, "syll", i, intv->text()));
        }
    }
    // Voiced-Unvoiced regions
    IntervalTier *tier_vuv = tiers_nuclei->getIntervalTierByName("vuv");
    if (tier_vuv) {
        for (int i = 0; i < tier_vuv->count(); ++i) {
            Interval *intv = tier_vuv->interval(i);
            d->vuvregions->addPoint(AnnotationGridPoint(RealTime::realTime2Frame(intv->tMin(), sampleRate),
                                                        RealTime::realTime2Frame(intv->duration(), sampleRate),
                                                        speakerID, "vuv", i, intv->text()));
        }
    }
    // Pitch
    QMap<RealTime, double> pitch;
    if (PraatPointTierFile::load(filenamePitchTier, pitch)) {
        for(QMap<RealTime, double>::iterator i = pitch.begin(); i != pitch.end(); ++i)
            // Converting Hz to semitones (rel to 1 Hz)
            d->pitch->addPoint(TimeValuePoint(RealTime::realTime2Frame(i.key(), sampleRate),
                                              12.0 * log2(i.value()), ""));
    }
    d->pitch->setResolution(RealTime::realTime2Frame(RealTime::fromSeconds(0.02), sampleRate));
    // Intensity
    QMap<RealTime, double> intensity;
    if (PraatPointTierFile::load(filenameIntensityTier, intensity)) {
        for(QMap<RealTime, double>::iterator i = intensity.begin(); i != intensity.end(); ++i)
            d->intensity->addPoint(TimeValuePoint(RealTime::realTime2Frame(i.key(), sampleRate), i.value(), ""));
    }
    return true;
}

ProsogramModel::~ProsogramModel()
{
    if (d) delete d;
}

sv_frame_t ProsogramModel::getStartFrame() const
{
    return d->segments->getStartFrame();
}

sv_frame_t ProsogramModel::getEndFrame() const
{
    return d->segments->getEndFrame();
}

sv_samplerate_t ProsogramModel::getSampleRate() const
{
    return d->sampleRate;
}

void ProsogramModel::toXml(QTextStream &out, QString indent, QString extraAttributes) const
{

}

QPointer<ProsogramTonalSegmentModel> ProsogramModel::segmentModel()
{
    return d->segments;
}

QPointer<AnnotationGridPointModel> ProsogramModel::phoneModel()
{
    return d->phones;
}

QPointer<AnnotationGridPointModel> ProsogramModel::syllModel()
{
    return d->sylls;
}

QPointer<AnnotationGridPointModel> ProsogramModel::vuvRegionModel()
{
    return d->vuvregions;
}

QPointer<SparseTimeValueModel> ProsogramModel::pitchModel()
{
    return d->pitch;
}

QPointer<SparseTimeValueModel> ProsogramModel::intensityModel()
{
    return d->intensity;
}


