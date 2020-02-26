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

private:
    ExperimentCLData *d;
};

#endif // EXPERIMENTCL_H
