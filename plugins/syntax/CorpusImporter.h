#ifndef CORPUSIMPORTER_H
#define CORPUSIMPORTER_H

#include <QString>
#include <QMap>
class AnnotationTierGroup;

class CorpusImporter
{
public:
    CorpusImporter();

    static bool readPerceo(const QString &filename, QMap<QString, QPointer<AnnotationTierGroup> > &tiers);
};

#endif // CORPUSIMPORTER_H
