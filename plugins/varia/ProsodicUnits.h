#ifndef PROSODICUNITS_H
#define PROSODICUNITS_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class Interval;
}
}

struct ProsodicUnitsData;

class ProsodicUnits
{
public:
    ProsodicUnits();
    ~ProsodicUnits();

    bool isLexical(Praaline::Core::Interval *token);
    QString categorise_CLI_INT_LEX(Praaline::Core::Interval *token);

    QString createProsodicUnits(Praaline::Core::CorpusCommunication *com);
    QString transcriptionInProsodicUnits(Praaline::Core::CorpusCommunication *com);

private:
    ProsodicUnitsData *d;
};

#endif // PROSODICUNITS_H
