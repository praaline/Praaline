#ifndef ANALYSERTEMPORAL_H
#define ANALYSERTEMPORAL_H

#include <QObject>
#include <QPointer>
#include <QTextStream>
#include "pncore/statistics/StatisticalMeasureDefinition.h"

struct AnalyserTemporalData;

namespace Praaline {
namespace Core {
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

    static QStringList measureIDsForCommunication();
    static QStringList measureIDsForSpeaker();
    static QStringList vectorMeasureIDsForCommunication();
    static QStringList vectorMeasureIDsForSpeaker();
    static StatisticalMeasureDefinition measureDefinition(const QString &measureID);

    QStringList speakerIDs() const;
    double measureCom(const QString &measureID) const;
    double measureSpk(const QString &speakerID, const QString &measureID) const;
    QList<double> vectorMeasureCom(const QString &measureID) const;
    QList<double> vectorMeasureSpk(const QString &speakerID, const QString &measureID) const;

    void calculate(QPointer<CorpusCommunication> com);

signals:

public slots:

private:
    AnalyserTemporalData *d;
};

#endif // ANALYSERTEMPORAL_H
