#ifndef SVBRIDGE_H
#define SVBRIDGE_H

#include <QString>
#include "pncore/annotation/IntervalTier.h"
using namespace Praaline::Core;

class SVBridge
{
public:
    static bool saveSVTimeInstantsLayer(const QString &filename, unsigned int sampleRate, IntervalTier *tier, const QString &attribute = QString());

private:
    SVBridge();
    ~SVBridge();
};

#endif // SVBRIDGE_H
