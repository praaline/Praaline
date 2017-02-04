#ifndef LONGSOUNDALIGNER_H
#define LONGSOUNDALIGNER_H

#include <QObject>
#include <QString>
#include <QPointer>

#include "pncore/corpus/Corpus.h"

struct LongSoundAlignerData;

class LongSoundAligner : public QObject
{
    Q_OBJECT
public:
    explicit LongSoundAligner(QObject *parent = 0);
    ~LongSoundAligner();

    bool createRecognitionLevel(Praaline::Core::CorpusRepository *repository, int recognitionStep);
    bool createUtterancesFromProsogramAutosyll(QPointer<Praaline::Core::CorpusCommunication> com);
    bool recognise(QPointer<Praaline::Core::CorpusCommunication> com, int recognitionStep);

private:
    LongSoundAlignerData *d;
};

#endif // LONGSOUNDALIGNER_H
