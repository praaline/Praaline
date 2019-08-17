#ifndef ORFEO_H
#define ORFEO_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class ORFEO
{
public:
    ORFEO();

    QString readOrfeoFile(Praaline::Core::CorpusCommunication *com);
    QString mapTokensToDisMo(Praaline::Core::CorpusCommunication *com);
    QString createSentenceUnits(Praaline::Core::CorpusCommunication *com);

};

#endif // ORFEO_H
