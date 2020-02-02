#ifndef LONGSOUNDALIGNER_H
#define LONGSOUNDALIGNER_H

#include <QObject>
#include <QString>
#include <QPointer>

#include "PraalineCore/Corpus/Corpus.h"

struct LongSoundAlignerData;

class LongSoundAligner : public QObject
{
    Q_OBJECT
public:
    explicit LongSoundAligner(QObject *parent = nullptr);
    ~LongSoundAligner();

    bool createRecognitionLevel(Praaline::Core::CorpusRepository *repository, int recognitionStep);
    bool createUtterancesFromProsogramAutosyll(Praaline::Core::CorpusCommunication *com);
    bool recognise(Praaline::Core::CorpusCommunication *com, int recognitionStep);

private:
    LongSoundAlignerData *d;
};

#endif // LONGSOUNDALIGNER_H
