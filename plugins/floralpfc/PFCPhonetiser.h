#ifndef PFCPHONETISER_H
#define PFCPHONETISER_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class Interval;
}
}

struct PFCPhonetiserData;

class PFCPhonetiser
{
public:
    PFCPhonetiser();
    ~PFCPhonetiser();

    QString phonetiseFromDictionary(QPointer<Praaline::Core::CorpusCommunication> com);
    QString phonetiseFromAdditionalList(QPointer<Praaline::Core::CorpusCommunication> com);
    QString applyPFCLiaison(QPointer<Praaline::Core::CorpusCommunication> com);
    QString applyPFCSchwa(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    PFCPhonetiserData *d;
};

#endif // PFCPHONETISER_H
