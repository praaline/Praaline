#include "HistogramCalculator.h"

// Based on QGIS HistogramCalculator.
// May 2015 (C) 2015 by Nyall Dawson nyall dot dawson at gmail dot com
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include <qmath.h>
#include "annotation/AnnotationTier.h"
#include "StatisticalSummary.h"
#include "HistogramCalculator.h"

namespace Praaline {
namespace Core {

HistogramCalculator::HistogramCalculator()
    : m_max(0), m_min(0), m_IQR(0)
{
}

HistogramCalculator::~HistogramCalculator()
{
}

void HistogramCalculator::prepareValues()
{
    qSort(m_values.begin(), m_values.end());

    StatisticalSummary s;
    s.setStatistics(StatisticalSummary::Max | StatisticalSummary::Min | StatisticalSummary::InterQuartileRange);
    s.calculate(m_values);
    m_min = s.min();
    m_max = s.max();
    m_IQR = s.interQuartileRange();
}

void HistogramCalculator::setValues(const QList<double> &values)
{
    m_values = values;
    prepareValues();
}

void HistogramCalculator::setMinimum(double min)
{
    m_min = min;
}

void HistogramCalculator::setMaximum(double max)
{
    m_max = max;
}

double HistogramCalculator::optimalBinWidth() const
{
    //Freedman-Diaconis rule
    return 2.0 * m_IQR * qPow(m_values.count(), -1 / 3.0);
}

int HistogramCalculator::optimalNumberBins() const
{
    return ceil((m_max - m_min) / optimalBinWidth());
}

QList<double> HistogramCalculator::binEdges(int bins) const
{
    double binWidth = (m_max - m_min) / bins;
    QList<double> edges;
    edges.reserve(bins + 1);
    edges << m_min;
    double current = m_min;
    for (int i = 0; i < bins; ++i) {
        current += binWidth;
        edges << current;
    }
    return edges;
}

QList<int> HistogramCalculator::counts(int bins) const
{
    QList<double> edges = binEdges(bins);
    QList<int> binCounts;
    binCounts.reserve(bins);
    int currentValueIndex = 0;
    for (int i = 0; i < bins; ++i) {
        int count = 0;
        while (currentValueIndex < m_values.count() && m_values.at(currentValueIndex) < edges.at(i + 1)) {
            count++;
            currentValueIndex++;
            if (currentValueIndex >= m_values.count())
                break;
        }
        binCounts << count;
    }
    if (currentValueIndex < m_values.count()) {
        //last value needs to be added
        binCounts[bins - 1] = binCounts.last() + 1;
    }
    return binCounts;
}

} // namespace Core
} // namespace Praaline
