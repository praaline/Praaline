#ifndef LATEXTRANSCRIPTIONBUILDER_H
#define LATEXTRANSCRIPTIONBUILDER_H

#include <QString>
#include <QPair>

namespace Praaline{
namespace Core {
class IntervalTier;
}
}

class LaTexTranscriptionBuilder
{
public:
    LaTexTranscriptionBuilder();

    QList<QPair<int, int> > calculateIndicesForSegmentation(Praaline::Core::IntervalTier *tokens);
    QString createTikzDependencyText(Praaline::Core::IntervalTier *tokens, int indexFrom, int indexTo);
};

#endif // LATEXTRANSCRIPTIONBUILDER_H
