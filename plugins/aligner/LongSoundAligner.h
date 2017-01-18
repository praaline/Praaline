#ifndef LONGSOUNDALIGNER_H
#define LONGSOUNDALIGNER_H

#include <QObject>
#include <QString>
#include <QPointer>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

struct LongSoundAlignerData;

class LongSoundAligner : public QObject
{
    Q_OBJECT
public:
    explicit LongSoundAligner(QObject *parent = 0);
    ~LongSoundAligner();

    bool createRecognitionLevel(CorpusRepository *repository, int recognitionStep);
    bool createUtterancesFromProsogramAutosyll(QPointer<CorpusCommunication> com);
    bool recognise(QPointer<CorpusCommunication> com, int recognitionStep);

private:
    LongSoundAlignerData *d;
};

#endif // LONGSOUNDALIGNER_H
