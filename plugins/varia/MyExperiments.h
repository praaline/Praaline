#ifndef MYEXPERIMENTS_H
#define MYEXPERIMENTS_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>
class Corpus;
class CorpusCommunication;

class MyExperiments
{
public:
    MyExperiments();

    static void createTextgridsFromAutosyll(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
    static void createBasicMetadata(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com);
};

#endif // MYEXPERIMENTS_H
