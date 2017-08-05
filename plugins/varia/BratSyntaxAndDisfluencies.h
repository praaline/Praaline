#ifndef BRATSYNTAXANDDISFLUENCIES_H
#define BRATSYNTAXANDDISFLUENCIES_H

#include <QString>
#include <QList>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class BratSyntaxAndDisfluencies
{
public:
    BratSyntaxAndDisfluencies();

    static QString getHTML(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // BRATSYNTAXANDDISFLUENCIES_H
