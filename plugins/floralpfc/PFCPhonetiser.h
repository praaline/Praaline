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

    void reset();
    QString loadPhonetisationDictionary();
    QString phonetiseFromDictionary(Praaline::Core::CorpusCommunication *com);
    QString writeListOfWordsOOV(const QString &filename);
    QString writeListOfWordsFalseStarts(const QString &filename);
    QString convertPhonetisationsToStars(const QString &phonetisations);
    void applyPFCLiaison(Praaline::Core::Interval *intv);
    void applyPFCSchwa(Praaline::Core::Interval *intv);

private:
    PFCPhonetiserData *d;
};

#endif // PFCPHONETISER_H
