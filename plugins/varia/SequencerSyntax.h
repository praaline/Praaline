#ifndef SEQUENCERSYNTAX_H
#define SEQUENCERSYNTAX_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

class SequencerSyntax
{
public:
    SequencerSyntax();

    static QString checkGroupingAnnotation(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString createSequencesFromGroupingAnnotation(QPointer<Praaline::Core::CorpusCommunication> com);
};

#endif // SEQUENCERSYNTAX_H
