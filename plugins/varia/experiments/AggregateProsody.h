#ifndef AGGREGATEPROSODY_H
#define AGGREGATEPROSODY_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class AggregateProsody
{
public:
    AggregateProsody();

    static QString markTargetSyllables(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString averageOnTokens(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString averageContours(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString readPCAdata(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString calculatePairwiseDistances(QPointer<Praaline::Core::CorpusCommunication> com);

    static QString percentagesProminence(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // AGGREGATEPROSODY_H
