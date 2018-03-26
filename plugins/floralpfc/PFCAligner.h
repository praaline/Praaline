#ifndef PFCALIGNER_H
#define PFCALIGNER_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class Interval;
}
}

struct PFCAlignerData;

class PFCAligner
{
public:
    PFCAligner();
    ~PFCAligner();

private:
    PFCAlignerData *d;
};

#endif // PFCALIGNER_H
