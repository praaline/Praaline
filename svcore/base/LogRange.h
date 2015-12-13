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

#ifndef _LOG_RANGE_H_
#define _LOG_RANGE_H_

#include <vector>
#include "Debug.h"

class LogRange
{
public:
    /**
     * Map a linear range onto a logarithmic range.  min and max are
     * passed as the extents of the linear range and returned as the
     * extents of the logarithmic range.  thresh is the minimum value
     * for the log range, to be used if the linear range spans zero.
     */
    static void mapRange(double &min, double &max, double thresh = -10);

    /**
     * Map a value onto a logarithmic range.  This just means taking
     * the base-10 log of the absolute value, or using the threshold
     * value if the absolute value is zero.
     */
    static double map(double value, double thresh = -10);

    /**
     * Map a value from the logarithmic range back again.  This just
     * means taking the value'th power of ten.
     */
    static double unmap(double value);

    /**
     * Estimate whether a set of values would be more properly shown
     * using a logarithmic than a linear scale.  This is only ever
     * going to be a rough guess.
     */
    static bool useLogScale(std::vector<double> values);

};

#endif
