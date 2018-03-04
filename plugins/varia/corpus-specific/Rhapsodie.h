#ifndef RHAPSODIE_H
#define RHAPSODIE_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
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
    static QString noteProsodicBoundaryOnSyll(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString importMicrosyntaxCONLL(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString importMicrosyntaxTabular(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString exportProsodicBoundariesAnalysisTable(QPointer<Praaline::Core::Corpus> corpus);
};

#endif // RHAPSODIE_H
