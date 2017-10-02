#ifndef SILENTPAUSEMANIPULATOR_H
#define SILENTPAUSEMANIPULATOR_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct SilentPauseManipulatorData;

class SilentPauseManipulator
{
public:
    SilentPauseManipulator();
    ~SilentPauseManipulator();

    QString process(QPointer<Praaline::Core::CorpusCommunication> com);
private:
    SilentPauseManipulatorData *d;
};

#endif // SILENTPAUSEMANIPULATOR_H
