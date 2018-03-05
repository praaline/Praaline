#ifndef MYEXPERIMENTS_H
#define MYEXPERIMENTS_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

#include "pncore/corpus/Corpus.h"
using namespace Praaline::Core;

class MyExperiments
{
public:
    MyExperiments();

    static void createTextgridsFromAutosyll(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
    static void createBasicMetadata(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
    static void renameRecordings(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
    static void updateTranscriptionMode(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);

    static void mergePauses(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
};

#endif // MYEXPERIMENTS_H
