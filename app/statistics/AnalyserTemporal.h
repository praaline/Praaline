#ifndef ANALYSERTEMPORAL_H
#define ANALYSERTEMPORAL_H

#include <QObject>
#include <QPointer>
#include <QTextStream>
#include "pncore/statistics/StatisticalMeasureDefinition.h"

struct AnalyserTemporalData;

namespace Praaline {
namespace Core {
class Corpus;
class CorpusCommunication;
class IntervalTier;
}
}
using namespace Praaline::Core;

class AnalyserTemporal : public QObject
{
    Q_OBJECT
public:
    explicit AnalyserTemporal(QObject *parent = 0);
    virtual ~AnalyserTemporal();

    static StatisticalMeasureDefinition measureDefinitionForCommunication(const QString &measureID);
    static StatisticalMeasureDefinition measureDefinitionForSpeaker(const QString &measureID);
    static QList<QString> measureIDsForCommunication();
    static QList<QString> measureIDsForSpeaker();

    double measureCom(const QString &measureID) const;
    double measureSpk(const QString &speakerID, const QString &measureID) const;
    QList<QString> speakerIDs() const;

    void calculate(QPointer<Corpus> corpus, QPointer<CorpusCommunication> com,
                   QTextStream &pauseListSIL, QTextStream &pauseListFIL);

signals:

public slots:

private:
    AnalyserTemporalData *d;
    static QHash<QString, StatisticalMeasureDefinition> measureDefsCom;
    static QHash<QString, StatisticalMeasureDefinition> measureDefsSpk;
};

#endif // ANALYSERTEMPORAL_H
