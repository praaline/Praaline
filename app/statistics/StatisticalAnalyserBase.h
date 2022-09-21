#ifndef STATISTICALANALYSERBASE_H
#define STATISTICALANALYSERBASE_H

#include <QString>
#include <QList>
#include <QMap>

#include "PraalineCore/Statistics/StatisticalMeasureDefinition.h"

// Base class for statistical analysers that support aggregating their measures per Communication or per Speaker

class StatisticalAnalyserBase
{
public:
    StatisticalAnalyserBase();
    virtual ~StatisticalAnalyserBase();

    virtual QStringList measureIDsForCommunication() = 0;
    virtual QStringList measureIDsForSpeaker() = 0;
    virtual QStringList vectorMeasureIDsForCommunication() = 0;
    virtual QStringList vectorMeasureIDsForSpeaker() = 0;
    virtual Praaline::Core::StatisticalMeasureDefinition measureDefinition(const QString &measureID) = 0;

    virtual QMap<QString, QList<double> > aggregateMeasureCom(
            const QString &measureID, const QStringList &groupAttributeIDsCom) = 0;
    virtual QMap<QString, QList<double> > aggregateMeasureSpk(
            const QString &measureID, const QStringList &groupAttributeIDsCom, const QStringList &groupAttributeIDsSpk) = 0;
};

#endif // STATISTICALANALYSERBASE_H
