#ifndef CPROMDISS_H
#define CPROMDISS_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}


class CPROMDISS
{
public:
    CPROMDISS();

    static QString transferBasicTiers(Praaline::Core::CorpusCommunication *com);
    static QString combineTiers(Praaline::Core::CorpusCommunication *com);
    static QString movePointAnnotationToInterval(Praaline::Core::CorpusCommunication *com);
};

#endif // CPROMDISS_H
