#ifndef CORPUSIMPORTER_H
#define CORPUSIMPORTER_H

#include <QString>
#include <QMap>
#include <QPointer>

#include "pncore/annotation/AnnotationTierGroup.h"
using namespace Praaline::Core;

class CorpusImporter
{
public:
    CorpusImporter();

    static bool readPerceo(const QString &filename, QMap<QString, QPointer<AnnotationTierGroup> > &tiers);
};

#endif // CORPUSIMPORTER_H
