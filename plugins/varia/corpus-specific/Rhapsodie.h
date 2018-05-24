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

struct RhapsodieData;

class Rhapsodie
{
public:
    Rhapsodie();
    ~Rhapsodie();

    QString prepareMultiSpeakerTextgrids(QPointer<Praaline::Core::CorpusCommunication> com);
    QString updateSyllables(QPointer<Praaline::Core::CorpusCommunication> com);
    QString loadPitch(QPointer<Praaline::Core::CorpusCommunication> com);
    QString readProsodicConstituencyTree(QPointer<Praaline::Core::CorpusCommunication> com);
    QString noteProsodicBoundaryOnSyll(QPointer<Praaline::Core::CorpusCommunication> com);
    QString importMicrosyntaxCONLL(QPointer<Praaline::Core::CorpusCommunication> com);
    QString importMicrosyntaxTabular(QPointer<Praaline::Core::CorpusCommunication> com);
    QString exportProsodicBoundariesAnalysisTable(QPointer<Praaline::Core::Corpus> corpus);
    QString findCONLLUCorrespondancePrepare(QPointer<Praaline::Core::Corpus> corpus);
    QString findCONLLUCorrespondanceMatch(QPointer<Praaline::Core::Corpus> corpus);


private:
    RhapsodieData *d;
};

#endif // RHAPSODIE_H
