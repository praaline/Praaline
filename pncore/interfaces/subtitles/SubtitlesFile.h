#ifndef SUBTITLESFILE_H
#define SUBTITLESFILE_H

#include "pncore_global.h"
#include <QString>
#include "interfaces/InterfaceTextFile.h"
#include "annotation/IntervalTier.h"

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT SubtitlesFile : InterfaceTextFile
{

public:
    // Public static methods to read and write SRT files
    static bool loadSRT(const QString &filename, IntervalTier *tier, const QString &attributeID = QString());
    static bool saveSRT(const QString &filename, IntervalTier *tier, const QString &attributeID = QString());
};

} // namespace Core
} // namespace Praaline

#endif // SUBTITLESFILE_H
