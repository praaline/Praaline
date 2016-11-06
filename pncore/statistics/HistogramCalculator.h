#ifndef HISTOGRAMCALCULATOR_H
#define HISTOGRAMCALCULATOR_H

// Based on QGIS qgshistogram.
// May 2015 (C) 2015 by Nyall Dawson nyall dot dawson at gmail dot com
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

#include "pncore_global.h"
#include <QList>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT HistogramCalculator
{
public:

    HistogramCalculator();
    virtual ~HistogramCalculator();

    void setValues(const QList<double>& values);

    double optimalBinWidth() const;
    int optimalNumberBins() const;

    QList<double> binEdges(int bins) const;
    QList<int> counts(int bins) const;

private:
    QList<double> m_values;
    double m_max;
    double m_min;
    double m_IQR;

    void prepareValues();
};

} // namespace Core
} // namespace Praaline


#endif // HISTOGRAMCALCULATOR_H
