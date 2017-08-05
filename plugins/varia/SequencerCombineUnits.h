#ifndef SEQUENCERCOMBINEUNITS_H
#define SEQUENCERCOMBINEUNITS_H


#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct SequencerCombineUnitsData;

class SequencerCombineUnits
{
public:
    SequencerCombineUnits();
    ~SequencerCombineUnits();

    QString sequencesLevelA() const;
    void setSequencesLevelA(const QString &levelID);

    QString sequencesLevelB() const;
    void setSequencesLevelB(const QString &levelID);

    QString sequencesLevelCombined() const;
    void setSequencesLevelCombined(const QString &levelID);

    QString createSequences(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    SequencerCombineUnitsData *d;
};

#endif // SEQUENCERCOMBINEUNITS_H
