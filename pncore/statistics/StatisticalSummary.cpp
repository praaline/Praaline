#include "StatisticalSummary.h"

// StatisticalSummary
//
// Adapted from StatisticalSummary.h in QGIS
// Date                 : May 2015
// Copyright            : (C) 2015 by Nyall Dawson
// Email                : nyall dot dawson at gmail dot com
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// This class is considered CRITICAL and any change MUST be accompanied with
// full unit tests in testStatisticalSummary.cpp.

#include "StatisticalSummary.h"
#include <limits>
#include <qmath.h>
#include <QString>
#include <QObject>

namespace Praaline {
namespace Core {

StatisticalSummary::StatisticalSummary(const Statistics& stats)
    : m_statistics(stats)
{
    reset();
}

StatisticalSummary::StatisticalSummary(const QList<double> &values, const StatisticalSummary::Statistics& stats)
    : m_statistics(stats)
{
    calculate(values);
}

StatisticalSummary::~StatisticalSummary()
{
}

void StatisticalSummary::reset()
{
    m_count = 0;
    m_sum = 0;
    m_mean = 0;
    m_median = 0;
    m_min = std::numeric_limits<double>::max();
    m_max = -std::numeric_limits<double>::max();
    m_stdev = 0;
    m_sampleStdev = 0;
    m_minority = 0;
    m_majority = 0;
    m_firstQuartile = 0;
    m_thirdQuartile = 0;
    m_valueCount.clear();
}

void StatisticalSummary::calculate(const QList<double> &values)
{
    reset();

    foreach (double value, values) {
        m_count++;
        m_sum += value;
        m_min = qMin(m_min, value);
        m_max = qMax(m_max, value);

        if (m_statistics & StatisticalSummary::Majority || m_statistics & StatisticalSummary::Minority || m_statistics & StatisticalSummary::Variety)
            m_valueCount.insert(value, m_valueCount.value(value, 0) + 1);
    }

    if (m_count == 0)
        return;

    m_mean = m_sum / m_count;

    if (m_statistics & StatisticalSummary::StDev || m_statistics & StatisticalSummary::StDevSample) {
        double sumSquared = 0;
        foreach (double value, values) {
            double diff = value - m_mean;
            sumSquared += diff * diff;
        }
        m_stdev = qPow(sumSquared / values.count(), 0.5);
        m_sampleStdev = qPow(sumSquared / (values.count() - 1), 0.5);
    }

    QList<double> sorted;
    if (   m_statistics & StatisticalSummary::Median
         || m_statistics & StatisticalSummary::FirstQuartile
         || m_statistics & StatisticalSummary::ThirdQuartile
         || m_statistics & StatisticalSummary::InterQuartileRange)
    {
        sorted = values;
        qSort(sorted.begin(), sorted.end());
        bool even = (m_count % 2) < 1;
        if (even) {
            m_median = (sorted[m_count / 2 - 1] + sorted[m_count / 2]) / 2.0;
        }
        else { // odd
            m_median = sorted[(m_count + 1) / 2 - 1];
        }
    }

    if (m_statistics & StatisticalSummary::FirstQuartile || m_statistics & StatisticalSummary::InterQuartileRange) {
        if ((m_count % 2) < 1) {
            int halfCount = m_count / 2;
            bool even = (halfCount % 2) < 1;
            if (even) {
                m_firstQuartile = (sorted[halfCount / 2 - 1] + sorted[halfCount / 2]) / 2.0;
            }
            else { //odd
                m_firstQuartile = sorted[(halfCount  + 1) / 2 - 1];
            }
        }
        else {
            int halfCount = m_count / 2 + 1;
            bool even = (halfCount % 2) < 1;
            if (even) {
                m_firstQuartile = (sorted[halfCount / 2 - 1] + sorted[halfCount / 2]) / 2.0;
            }
            else { //odd
                m_firstQuartile = sorted[(halfCount  + 1) / 2 - 1];
            }
        }
    }

    if (m_statistics & StatisticalSummary::ThirdQuartile || m_statistics & StatisticalSummary::InterQuartileRange) {
        if ((m_count % 2) < 1) {
            int halfCount = m_count / 2;
            bool even = (halfCount % 2) < 1;
            if (even) {
                m_thirdQuartile = (sorted[halfCount + halfCount / 2 - 1] + sorted[halfCount + halfCount / 2]) / 2.0;
            }
            else  { //odd
                m_thirdQuartile = sorted[(halfCount + 1) / 2 - 1 + halfCount];
            }
        }
        else {
            int halfCount = m_count / 2 + 1;
            bool even = (halfCount % 2) < 1;
            if (even) {
                m_thirdQuartile = (sorted[halfCount + halfCount / 2 - 2] + sorted[halfCount + halfCount / 2 - 1]) / 2.0;
            }
            else { //odd
                m_thirdQuartile = sorted[(halfCount + 1) / 2 - 2 + halfCount];
            }
        }
    }

    if (m_statistics & StatisticalSummary::Minority || m_statistics & StatisticalSummary::Majority)
    {
        QList<int> valueCounts = m_valueCount.values();
        qSort(valueCounts.begin(), valueCounts.end());
        if (m_statistics & StatisticalSummary::Minority) {
            m_minority = m_valueCount.key(valueCounts.first());
        }
        if (m_statistics & StatisticalSummary::Majority) {
            m_majority = m_valueCount.key(valueCounts.last());
        }
    }
}

double StatisticalSummary::statistic(StatisticalSummary::Statistic stat) const
{
    switch (stat)
    {
    case Count:
        return m_count;
    case Sum:
        return m_sum;
    case Mean:
        return m_mean;
    case Median:
        return m_median;
    case StDev:
        return m_stdev;
    case StDevSample:
        return m_sampleStdev;
    case Min:
        return m_min;
    case Max:
        return m_max;
    case Range:
        return m_max - m_min;
    case Minority:
        return m_minority;
    case Majority:
        return m_majority;
    case Variety:
        return m_valueCount.count();
    case FirstQuartile:
        return m_firstQuartile;
    case ThirdQuartile:
        return m_thirdQuartile;
    case InterQuartileRange:
        return m_thirdQuartile - m_firstQuartile;
    case All:
        return 0;
    }
    return 0;
}

QString StatisticalSummary::displayName(StatisticalSummary::Statistic statistic)
{
    switch (statistic)
    {
    case Count:
        return QObject::tr("Count");
    case Sum:
        return QObject::tr("Sum");
    case Mean:
        return QObject::tr("Mean");
    case Median:
        return QObject::tr("Median");
    case StDev:
        return QObject::tr("StDev");
    case StDevSample:
        return QObject::tr("StDev(S)");
    case Min:
        return QObject::tr("Minimum");
    case Max:
        return QObject::tr("Maximum");
    case Range:
        return QObject::tr("Range");
    case Minority:
        return QObject::tr("Minority");
    case Majority:
        return QObject::tr("Majority");
    case Variety:
        return QObject::tr("Variety");
    case FirstQuartile:
        return QObject::tr("Q1");
    case ThirdQuartile:
        return QObject::tr("Q3");
    case InterQuartileRange:
        return QObject::tr("IQR");
    case All:
        return QString();
    }
    return QString();
}

} // namespace Core
} // namespace Praaline
