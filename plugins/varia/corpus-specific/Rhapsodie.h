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

    QString prepareMultiSpeakerTextgrids(Praaline::Core::CorpusCommunication *com);
    QString updateSyllables(Praaline::Core::CorpusCommunication *com);
    QString loadPitch(Praaline::Core::CorpusCommunication *com);
    QString readProsodicConstituencyTree(Praaline::Core::CorpusCommunication *com);
    QString noteProsodicBoundaryOnSyll(Praaline::Core::CorpusCommunication *com);
    QString importMicrosyntaxCONLL(Praaline::Core::CorpusCommunication *com);
    QString importMicrosyntaxTabular(Praaline::Core::CorpusCommunication *com);
    QString exportProsodicBoundariesAnalysisTable(QPointer<Praaline::Core::Corpus> corpus);
    QString findCONLLUCorrespondancePrepare(QPointer<Praaline::Core::Corpus> corpus);
    QString findCONLLUCorrespondanceMatch(QPointer<Praaline::Core::Corpus> corpus);
    QString importCONLLU(Praaline::Core::CorpusCommunication *com);

private:
    RhapsodieData *d;
};

#endif // RHAPSODIE_H
