#ifndef INTERVALTIERCOMBINATIONS_H
#define INTERVALTIERCOMBINATIONS_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct IntervalTierCombinationsData;

class IntervalTierCombinations
{
public:
    IntervalTierCombinations();
    ~IntervalTierCombinations();

    QString intervalsLevelA() const;
    void setIntervalsLevelA(const QString &levelID);

    QString intervalsLevelB() const;
    void setIntervalsLevelB(const QString &levelID);

    QString intervalsLevelCombined() const;
    void setIntervalsLevelCombined(const QString &levelID);

    QString combineIntervalTiers(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    IntervalTierCombinationsData *d;
};

#endif // INTERVALTIERCOMBINATIONS_H
