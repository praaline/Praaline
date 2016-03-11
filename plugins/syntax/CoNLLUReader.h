#ifndef CONLLUREADER_H
#define CONLLUREADER_H

#include <QString>
class AnnotationTierGroup;

class CoNLLUReader
{
public:
    static bool readCoNLLUtoIntervalTier(const QString &filename, AnnotationTierGroup *group);

    CoNLLUReader();
};

#endif // CONLLUREADER_H
