#ifndef SYLLABIFIEREASY_H
#define SYLLABIFIEREASY_H

#include <QString>

namespace Praaline {
namespace Core {
class IntervalTier;
}
}


class SyllabifierEasy
{
public:
    SyllabifierEasy();

    static int getCategory(Praaline::Core::IntervalTier *tier_syll, int index, int offset);
    static int phoneCategory(const QString &phone);
};

#endif // SYLLABIFIEREASY_H
