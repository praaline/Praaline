#ifndef SYLLABIFIEREASY_H
#define SYLLABIFIEREASY_H

#include <QString>
#include "PraalineCore/Base/RealTime.h"
#include "PraalineASR/PraalineASR_Global.h"

namespace Praaline {

namespace Core {
class IntervalTier;
}

namespace ASR {

class PRAALINE_ASR_SHARED_EXPORT SyllabifierEasy
{
public:
    SyllabifierEasy();

    bool syllabify(Praaline::Core::IntervalTier *tier_phone, Praaline::Core::IntervalTier *tier_syll,
                   RealTime from, RealTime to);
    Praaline::Core::IntervalTier *createSyllableTier(Praaline::Core::IntervalTier *tier_phone);

private:
    int getCategory(Praaline::Core::IntervalTier *tier_syll, int index, int offset);
    int phoneCategory(const QString &phone);
    void delcurrent(Praaline::Core::IntervalTier *tier_syll, int index, const QString &ruleName);
};

} // namespace ASR
} // namespace Praaline

#endif // SYLLABIFIEREASY_H
