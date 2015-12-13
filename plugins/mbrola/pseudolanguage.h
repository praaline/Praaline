#ifndef PSEUDOLANGUAGE_H
#define PSEUDOLANGUAGE_H

#include <QPointer>
#include "pncore/corpus/corpus.h"

class PseudoLanguage
{
public:
    PseudoLanguage();

    bool createListOfPossibleDiphones(Corpus *corpus, QList<QPointer<CorpusCommunication> > communications,
                                      const QString &filename, int cutOffThreshold = 0);
    void substitutePhonemes(IntervalTier *tier_phones, const QString &attributeID);
};

#endif // PSEUDOLANGUAGE_H
