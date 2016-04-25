#ifndef PROSODYCOURSE_H
#define PROSODYCOURSE_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QList>

class Corpus;
class CorpusCommunication;

class ProsodyCourse
{
public:
    ProsodyCourse();

    static void exportMultiTierTextgrids(Corpus *corpus, QPointer<CorpusCommunication> com);
    static void temporalVariables(Corpus *corpus, QPointer<CorpusCommunication> com);
};

#endif // PROSODYCOURSE_H
