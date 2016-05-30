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
    static void syllableTables(Corpus *corpus);
};

#endif // PROSODYCOURSE_H
