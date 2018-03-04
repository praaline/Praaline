#ifndef LOCASF_H
#define LOCASF_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
}
}

class LOCASF
{
public:
    LOCASF();

    static QString noteProsodicBoundaryOnSyll(QPointer<Praaline::Core::CorpusCommunication> com);
    static QString exportProsodicBoundariesAnalysisTable(QPointer<Praaline::Core::Corpus> corpus);
};

#endif // LOCASF_H
