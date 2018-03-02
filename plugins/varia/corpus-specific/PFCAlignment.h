#ifndef PFCALIGNMENT_H
#define PFCALIGNMENT_H

#include <QString>
#include <QPointer>

#include "pncore/corpus/Corpus.h"

class PFCAlignment
{
public:
    PFCAlignment();

    static QString detectOffCharacters(const QPointer<Praaline::Core::CorpusCommunication> com, const QString &tierName);

};

#endif // PFCALIGNMENT_H
