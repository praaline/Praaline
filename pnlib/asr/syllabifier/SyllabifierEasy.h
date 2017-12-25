#ifndef SYLLABIFIEREASY_H
#define SYLLABIFIEREASY_H

#include <QString>

namespace Praaline {

namespace Core {
class IntervalTier;
}

namespace ASR {

class SyllabifierEasy
{
public:
    SyllabifierEasy();

    static Praaline::Core::IntervalTier *syllabify(Praaline::Core::IntervalTier *tier_phone);

private:
    static int getCategory(Praaline::Core::IntervalTier *tier_syll, int index, int offset);
    static int phoneCategory(const QString &phone);
    static void delcurrent(Praaline::Core::IntervalTier *tier_syll, int index, const QString &ruleName);
};

} // namespace ASR
} // namespace Praaline

#endif // SYLLABIFIEREASY_H
