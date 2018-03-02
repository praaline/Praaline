#ifndef LOCASF_H
#define LOCASF_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class LOCASF
{
public:
    LOCASF();

    static QString noteProsodicBoundaryOnSyll(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // LOCASF_H
