#ifndef CONLLUREADER_H
#define CONLLUREADER_H

#include <QString>

#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

class CoNLLUReader
{
public:
    static bool readCoNLLUtoIntervalTier(const QString &filename, AnnotationTierGroup *group);

    CoNLLUReader();
};

#endif // CONLLUREADER_H
