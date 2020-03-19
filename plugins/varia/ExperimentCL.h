#ifndef EXPERIMENTCL_H
#define EXPERIMENTCL_H

#include <QString>

namespace Praaline {
namespace Core {
class CorpusCommunication;
}
}

struct ExperimentCLData;

class ExperimentCL
{
public:
    ExperimentCL();
    ~ExperimentCL();

    QString extractTimingIntervals(Praaline::Core::CorpusCommunication *com);
    QString readDTMF(Praaline::Core::CorpusCommunication *com);
    QString createExpeBlocks(Praaline::Core::CorpusCommunication *com);

    QString setResultsRepository(Praaline::Core::CorpusCommunication *com);
    QString importCSV_ETU_FR01(const QString &filename);
    QString importCSV_ETU_FR02(const QString &filename);
    QString importCSV_ETU_FR03(const QString &filename);
    QString importCSV_ETU_EN01(const QString &filename);
    QString importCSV_ETU_EN02(const QString &filename);
    QString importCSV_ETU_EN03(const QString &filename);



private:
    ExperimentCLData *d;
};

#endif // EXPERIMENTCL_H
