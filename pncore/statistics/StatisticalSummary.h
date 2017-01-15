#ifndef STATISTICALSUMMARY_H
#define STATISTICALSUMMARY_H

/*
    Praaline - Core module - Statistics
    Copyright (c) 2011-2017 George Christodoulides

    This file is based on:
    QGIS qgsstatisticalsummary.h
    May 2015 (C) 2015 by Nyall Dawson nyall dot dawson at gmail dot com

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "pncore_global.h"
#include <QMap>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT StatisticalSummary
{
public:
    enum Statistic
    {
        Count = 1,
        Sum = 2,
        Mean = 4,
        Median = 8,
        StDev = 16,
        StDevSample = 32,
        Min = 64,
        Max = 128,
        Range = 256,
        Minority = 512,
        Majority = 1024,
        Variety = 2048,
        FirstQuartile = 4096,
        ThirdQuartile = 8192,
        InterQuartileRange = 16384,
        All = Count | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety | FirstQuartile | ThirdQuartile | InterQuartileRange
    };
    Q_DECLARE_FLAGS(Statistics, Statistic)

    StatisticalSummary(const StatisticalSummary::Statistics& stats = All);
    StatisticalSummary(const QList<double> &values, const StatisticalSummary::Statistics& stats = All);
    virtual ~StatisticalSummary();


    Statistics statistics() const { return m_statistics; }
    void setStatistics(const Statistics& stats) { m_statistics = stats; }

    void reset();
    void calculate(const QList<double> &values);

    double statistic(Statistic stat) const;

    int count() const                   { return m_count; }
    double sum() const                  { return m_sum; }
    double mean() const                 { return m_mean; }
    double median() const               { return m_median; }
    double min() const                  { return m_min; }
    double max() const                  { return m_max; }
    double range() const                { return m_max - m_min; }
    double stDev() const                { return m_stdev; }
    double sampleStDev() const          { return m_sampleStdev; }
    int variety() const                 { return m_valueCount.count(); }
    double minority() const             { return m_minority; }
    double majority() const             { return m_majority; }
    double firstQuartile() const        { return m_firstQuartile; }
    double thirdQuartile() const        { return m_thirdQuartile; }
    double interQuartileRange() const   { return m_thirdQuartile - m_firstQuartile; }

    static QString displayName(Statistic statistic);

private:
    Statistics m_statistics;
    int m_count;
    double m_sum;
    double m_mean;
    double m_median;
    double m_min;
    double m_max;
    double m_stdev;
    double m_sampleStdev;
    double m_minority;
    double m_majority;
    double m_firstQuartile;
    double m_thirdQuartile;
    QMap< double, int > m_valueCount;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatisticalSummary::Statistics)

} // namespace Core
} // namespace Praaline

#endif // STATISTICALSUMMARY_H
