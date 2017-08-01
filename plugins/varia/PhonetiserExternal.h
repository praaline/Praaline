#ifndef PHONETISEREXTERNAL_H
#define PHONETISEREXTERNAL_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class PhonetiserExternal
{
public:
    PhonetiserExternal();

    static QString exportToPhonetiser(QPointer<CorpusCommunication> com);
    static QString importFromPhonetiser(QPointer<CorpusCommunication> com);
};

#endif // PHONETISEREXTERNAL_H
