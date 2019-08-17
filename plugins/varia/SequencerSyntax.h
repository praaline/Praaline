#ifndef SEQUENCERSYNTAX_H
#define SEQUENCERSYNTAX_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct SequencerSyntaxData;

class SequencerSyntax
{
public:
    SequencerSyntax();
    ~SequencerSyntax();

    QString checkGroupingAnnotation(Praaline::Core::CorpusCommunication *com);
    QString createSequencesFromGroupingAnnotation(Praaline::Core::CorpusCommunication *com);

private:
    SequencerSyntaxData *d;
};

#endif // SEQUENCERSYNTAX_H
