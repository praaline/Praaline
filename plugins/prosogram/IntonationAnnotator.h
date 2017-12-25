#ifndef INTONATIONANNOTATOR_H
#define INTONATIONANNOTATOR_H

#include <QPointer>
#include <QPair>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class IntervalTier;
class Interval;
}
}

struct IntonationAnnotatorData;

class IntonationAnnotator
{
public:
    class PitchRangeInfo {
    public:
        PitchRangeInfo() :
            countSyll(0), topHz(0), bottomHz(0), medianHz(0)
        {}

        int countSyll;
        double topHz;
        double bottomHz;
        double medianHz;
        double topST() const { return 12.0 * log2(topHz); }
        double bottomST() const { return 12.0 * log2(bottomHz); }
        double medianST() const { return 12.0 * log2(medianHz); }
        double rangeHz() const { return topHz - bottomHz; }
        double rangeST() const { return topST() - bottomST(); }
        double upperST() const { return topST() - medianST(); }
        double lowerST() const { return medianST() - bottomST(); }
    };

    IntonationAnnotator();
    virtual ~IntonationAnnotator();

    PitchRangeInfo pitchRangeInfo(const QString &speakerID) const;

    void estimatePitchRange(QPointer<Praaline::Core::Corpus> corpus, const QStringList &speakerIDs = QStringList());
    void annotate(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    QString classifyPitchInterval(const PitchRangeInfo &pitchRange, double fromHz, double toHz);
    QString pitchLevelBasedOnPitchSpan(const PitchRangeInfo &pitchRange, Praaline::Core::IntervalTier *tier_syll, int indexTargetSyll);
    QString pitchLevelBasedOnLocalPitchChange(const PitchRangeInfo &pitchRange, Praaline::Core::IntervalTier *tier_syll, int indexTargetSyll);
    QString pitchLevelInferredFromIntrasyllabicMvt(const PitchRangeInfo &pitchRange, Praaline::Core::IntervalTier *tier_syll, int indexTargetSyll);
    QString pitchLevelExtrapolated(const PitchRangeInfo &pitchRange, Praaline::Core::Interval *target_syll, Praaline::Core::Interval *ref_syll);
    void pitchLevelExtrapolated(const PitchRangeInfo &pitchRange, Praaline::Core::IntervalTier *tier_syll);
    void pitchLevelForPlateaus(const PitchRangeInfo &pitchRange, Praaline::Core::IntervalTier *tier_syll);


    IntonationAnnotatorData *d;
};

#endif // INTONATIONANNOTATOR_H
