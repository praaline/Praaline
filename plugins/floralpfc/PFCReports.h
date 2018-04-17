#ifndef PFCREPORTS_H
#define PFCREPORTS_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
}
}

struct PFCReportsData;

class PFCReports
{
public:
    PFCReports();
    ~PFCReports();

    QString corpusCoverageStatistics(QPointer<Praaline::Core::Corpus> corpus);
    QString reportCorrections(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    PFCReportsData *d;
};

#endif // PFCREPORTS_H
