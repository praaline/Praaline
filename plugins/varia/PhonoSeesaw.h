#ifndef PHONOSEESAW_H
#define PHONOSEESAW_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
}
}

struct PhonoSeesawData;

class PhonoSeesaw
{
public:
    PhonoSeesaw();
    ~PhonoSeesaw();

    void resetDistributionCounts();
    QString accumulateRefDistribution(Praaline::Core::CorpusCommunication *com);
    QString calculateRefDistribution();

    QString addUnitsToListOfAvailable(Praaline::Core::CorpusCommunication *com);
    void calculateDistributionFromPhonemeList(QStringList phonemes, QMap<QString, int> &distributionCounts,
                                              QMap<QString, double> &distributionFreqs);
    QString checkSelection(QList<int> selection, int &countRejects);
    QString makeBestSelection(int numberOfUnits, int numberOfAttempts);

private:
    PhonoSeesawData *d;

    int randInt(int low, int high);
    QList<int> selection(int numberOfUnits);

};

#endif // PHONOSEESAW_H
