#ifndef CORPUSIMPORTER_H
#define CORPUSIMPORTER_H

#include <QString>
#include <QMap>
#include <QPointer>

#include "PraalineCore/Annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

class CorpusImporter
{
public:
    CorpusImporter();

    static bool readPerceo(const QString &filename, SpeakerAnnotationTierGroupMap &tiers);
};

#endif // CORPUSIMPORTER_H
