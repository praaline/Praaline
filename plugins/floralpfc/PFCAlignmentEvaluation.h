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
    class EvaluationResults {
    public:
        QString ID;
        QString groupID;
        QString speakerID;
        QString regionData;
        QString regionModel;
        long insertions;
        long deletions;
        long phonemesLessThan20ms;
        long phonemesLessThan40ms;
        long phonemesTotal;
        QString filenameComparative;

        double percentageLessThan20ms() const {
            return ((double) phonemesLessThan20ms) / ((double) phonemesTotal);
        }
        double percentageLessThan40ms() const {
            return ((double) phonemesLessThan40ms) / ((double) phonemesTotal);
        }
        EvaluationResults() : insertions(0), deletions(0),
            phonemesLessThan20ms(0), phonemesLessThan40ms(0), phonemesTotal(0)
        {}
        QString toString() const;
    };

    PFCAlignmentEvaluation();
    ~PFCAlignmentEvaluation();

    QList<EvaluationResults> evaluate(Praaline::Core::CorpusCommunication *com,
                                      const QString &directoryTextGridA, const QString nameA,
                                      const QString &directoryTextGridB, const QString nameB,
                                      const QString &directoryTextgridCompare);

    QString evaluate_Individual_RegionStyle(Praaline::Core::CorpusCommunication *com);
    QString evaluate_RegionStyle_RegionStyle(Praaline::Core::CorpusCommunication *com);

    void pivotReset();
    QStringList pivotList(const QString &style);
    QStringList pivotTable(const QString &style, int tolerance);

private:
    PFCAlignmentEvaluationData *d;
};

#endif // PFCALIGNMENTEVALUATION_H
