#ifndef ORFEOREADER_H
#define ORFEOREADER_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class ORFEOReader
{
public:
    ORFEOReader();

    QString readOrfeoFile(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // ORFEOREADER_H
