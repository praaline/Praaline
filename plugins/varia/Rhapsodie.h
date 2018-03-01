#ifndef RHAPSODIE_H
#define RHAPSODIE_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}


class Rhapsodie
{
public:
    Rhapsodie();

    static QString updateSyllables(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString loadPitch(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString readProsodicConstituencyTree(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // RHAPSODIE_H
