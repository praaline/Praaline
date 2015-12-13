/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "LogRange.h"
#include "system/System.h"

#include <algorithm>
#include <iostream>
#include <cmath>

void
LogRange::mapRange(double &min, double &max, double logthresh)
{
    if (min > max) std::swap(min, max);
    if (max == min) max = min + 1;

//    cerr << "LogRange::mapRange: min = " << min << ", max = " << max << endl;

    if (min >= 0.f) {

        max = log10(max); // we know max != 0

        if (min == 0.f) min = std::min(logthresh, max);
        else min = log10(min);

//        cerr << "LogRange::mapRange: positive: min = " << min << ", max = " << max << endl;

    } else if (max <= 0.f) {
        
        min = log10(-min); // we know min != 0
        
        if (max == 0.f) max = std::min(logthresh, min);
        else max = log10(-max);
        
        std::swap(min, max);
        
//        cerr << "LogRange::mapRange: negative: min = " << min << ", max = " << max << endl;

    } else {
        
        // min < 0 and max > 0
        
        max = log10(std::max(max, -min));
        min = std::min(logthresh, max);

//        cerr << "LogRange::mapRange: spanning: min = " << min << ", max = " << max << endl;
    }

    if (min == max) min = max - 1;
}        

double
LogRange::map(double value, double thresh)
{
    if (value == 0.f) return thresh;
    return log10(fabs(value));
}

double
LogRange::unmap(double value)
{
    return pow(10.0, value);
}

static double
sd(const std::vector<double> &values, int start, int n)
{
    double sum = 0.f, mean = 0.f, variance = 0.f;
    for (int i = 0; i < n; ++i) {
        sum += values[start + i];
    }
    mean = sum / n;
    for (int i = 0; i < n; ++i) {
        double diff = values[start + i] - mean;
        variance += diff * diff;
    }
    variance = variance / n;
    return sqrt(variance);
}

bool
LogRange::useLogScale(std::vector<double> values)
{
    // Principle: Partition the data into two sets around the median;
    // calculate the standard deviation of each set; if the two SDs
    // are very different, it's likely that a log scale would be good.

    int n = int(values.size());
    if (n < 4) return false;
    std::sort(values.begin(), values.end());
    int mi = n / 2;

    double sd0 = sd(values, 0, mi);
    double sd1 = sd(values, mi, n - mi);

    cerr << "LogRange::useLogScale: sd0 = "
              << sd0 << ", sd1 = " << sd1 << endl;

    if (sd0 == 0 || sd1 == 0) return false;

    // I wonder what method of determining "one sd much bigger than
    // the other" would be appropriate here...
    if (std::max(sd0, sd1) / std::min(sd0, sd1) > 10.) return true;
    else return false;
}
    
