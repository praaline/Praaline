#ifndef MEASURES_H
#define MEASURES_H

#include "pncore_global.h"
#include <QString>
#include <QPair>

namespace Praaline {
namespace Core {

class IntervalTier;

class PRAALINE_CORE_SHARED_EXPORT Measures
{
public:
    static QPair<int, int> window(IntervalTier *tier, int i, int windowLeft, int windowRight, bool pauseBlocksWindow = true);
    static bool mean(double &mean, IntervalTier *tier, QString attributeName, int i, int windowLeft, int windowRight,
                     bool pauseBlocksWindow = true, QString checkAttribute = QString());
    static double mean(IntervalTier *tier, QString attributeName, int i, int windowLeft, int windowRight,
                       bool pauseBlocksWindow = true, QString checkAttribute = QString());
    static double relative(IntervalTier *tier, QString attributeName, int i, int windowLeft, int windowRight,
                           bool pauseBlocksWindow = true, QString checkAttribute = QString(), bool logarithmic = false);

private:
    Measures() {}
};

} // namespace Core
} // namespace Praaline

#endif // MEASURES_H
