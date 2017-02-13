#ifndef HISTOGRAMCALCULATOR_H
#define HISTOGRAMCALCULATOR_H

/*
    Praaline - Core module - Statistics
    Copyright (c) 2011-2017 George Christodoulides

    This file is based on:
    QGIS qgshistogram.
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
#include <QList>

namespace Praaline {
namespace Core {

class PRAALINE_CORE_SHARED_EXPORT HistogramCalculator
{
public:

    HistogramCalculator();
    virtual ~HistogramCalculator();

    void setValues(const QList<double>& values);
    void setMinimum(double min);
    void setMaximum(double max);

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
