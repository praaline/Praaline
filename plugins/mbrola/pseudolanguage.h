#ifndef PSEUDOLANGUAGE_H
#define PSEUDOLANGUAGE_H

#include <QPointer>
#include "PraalineCore/Corpus/CorpusCommunication.h"
#include "PraalineCore/Annotation/IntervalTier.h"

class PseudoLanguage
{
public:
    PseudoLanguage();

    bool createListOfPossibleDiphones(const QList<Praaline::Core::CorpusCommunication *> &communications,
                                      const QString &filename, int cutOffThreshold = 0);
    void substitutePhonemes(Praaline::Core::IntervalTier *tier_phones, const QString &attributeID);
};

#endif // PSEUDOLANGUAGE_H
