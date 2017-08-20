#ifndef STATISTICALMEASUREAGGREGATOR_H
#define STATISTICALMEASUREAGGREGATOR_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>

class StatisticalMeasureAggregator
{
public:
    StatisticalMeasureAggregator();

    QMap<QString, QList<double> > aggregateMeasureCom(
            const QString &measureID, const QStringList &groupAttributeIDsCom);
    QMap<QString, QList<double> > aggregateMeasureSpk(
            const QString &measureID, const QStringList &groupAttributeIDsCom, const QStringList &groupAttributeIDsSpk);
};

#endif // STATISTICALMEASUREAGGREGATOR_H
