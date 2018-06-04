#ifndef PFCALIGNMENTEVALUATION_H
#define PFCALIGNMENTEVALUATION_H

#include <QString>
#include <QPointer>

namespace Praaline {
namespace Core {
class CorpusCommunication;
class Interval;
}
}

struct PFCAlignmentEvaluationData;

class PFCAlignmentEvaluation
{
public:
    PFCAlignmentEvaluation();
    ~PFCAlignmentEvaluation();

    QString evaluate(QPointer<Praaline::Core::CorpusCommunication> com,
                     const QString &directoryTextGridA, const QString nameA,
                     const QString &directoryTextGridB, const QString nameB,
                     const QString &directoryTextgridCompare);

    QString evaluate_Individual_RegionStyle(QPointer<Praaline::Core::CorpusCommunication> com);
    QString evaluate_RegionStyle_RegionStyle(QPointer<Praaline::Core::CorpusCommunication> com);

private:
    PFCAlignmentEvaluationData *d;
};

#endif // PFCALIGNMENTEVALUATION_H
