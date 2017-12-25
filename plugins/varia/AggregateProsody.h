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

    static QString averageOnTokens(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString averageContours(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // AGGREGATEPROSODY_H
