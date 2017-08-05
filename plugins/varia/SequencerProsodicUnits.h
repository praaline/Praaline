#ifndef SEQUENCERPROSODICUNITS_H
#define SEQUENCERPROSODICUNITS_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct SequencerProsodicUnitsData;

class SequencerProsodicUnits
{
public:
    SequencerProsodicUnits();
    ~SequencerProsodicUnits();

    QString levelTokens() const;
    void setLevelTokens(const QString &levelID);

    QString levelSyllables() const;
    void setLevelSyllables(const QString &levelID);

    QString attributeBoundary() const;
    void setAttributeBoundary(const QString &attributeID);

    QString createSequencesFromProsodicBoundaries(QPointer<Praaline::Core::CorpusCommunication> com, const QString &boundaryLabel);

private:
    SequencerProsodicUnitsData *d;
};

#endif // SEQUENCERPROSODICUNITS_H
