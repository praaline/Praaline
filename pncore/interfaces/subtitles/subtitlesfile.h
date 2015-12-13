#ifndef SUBTITLESFILE_H
#define SUBTITLESFILE_H

#include "pncore_global.h"
#include <QString>
#include "interfaces/interfacetextfile.h"
#include "annotation/intervaltier.h"

class PRAALINE_CORE_SHARED_EXPORT SubtitlesFile : InterfaceTextFile
{

public:
    // Public static methods to read and write SRT files
    static bool loadSRT(const QString &filename, IntervalTier *tier);
    static bool saveSRT(const QString &filename, IntervalTier *tier);
};

#endif // SUBTITLESFILE_H
