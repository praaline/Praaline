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

    QString loadPhonetisationDictionary();
    QString phonetiseFromDictionary(QPointer<Praaline::Core::CorpusCommunication> com);
    QString writeListOfWordsOOV(const QString &filename);
    QString writeListOfWordsFalseStarts(const QString &filename);
    QString applyPFCLiaison(QPointer<Praaline::Core::CorpusCommunication> com);
    QString applyPFCSchwa(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    PFCPhonetiserData *d;
    QString convertSphinxToSampa(const QString &);
};

#endif // PFCPHONETISER_H
