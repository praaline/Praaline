/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "RangeMapper.h"
#include "system/System.h"

#include <cassert>
#include <cmath>

#include <iostream>

LinearRangeMapper::LinearRangeMapper(int minpos, int maxpos,
				     double minval, double maxval,
                                     QString unit, bool inverted) :
    m_minpos(minpos),
    m_maxpos(maxpos),
    m_minval(minval),
    m_maxval(maxval),
    m_unit(unit),
    m_inverted(inverted)
{
    assert(m_maxval != m_minval);
    assert(m_maxpos != m_minpos);
}

int
LinearRangeMapper::getPositionForValue(double value) const
{
    int position = getPositionForValueUnclamped(value);
    if (position < m_minpos) position = m_minpos;
    if (position > m_maxpos) position = m_maxpos;
    return position;
}

int
LinearRangeMapper::getPositionForValueUnclamped(double value) const
{
    int position = m_minpos +
        int(lrint(((value - m_minval) / (m_maxval - m_minval))
                  * (m_maxpos - m_minpos)));
    if (m_inverted) return m_maxpos - (position - m_minpos);
    else return position;
}

double
LinearRangeMapper::getValueForPosition(int position) const
{
    if (position < m_minpos) position = m_minpos;
    if (position > m_maxpos) position = m_maxpos;
    double value = getValueForPositionUnclamped(position);
    return value;
}

double
LinearRangeMapper::getValueForPositionUnclamped(int position) const
{
    if (m_inverted) position = m_maxpos - (position - m_minpos);
    double value = m_minval +
        ((double(position - m_minpos) / double(m_maxpos - m_minpos))
         * (m_maxval - m_minval));
    return value;
}

LogRangeMapper::LogRangeMapper(int minpos, int maxpos,
                               double minval, double maxval,
                               QString unit, bool inverted) :
    m_minpos(minpos),
    m_maxpos(maxpos),
    m_unit(unit),
    m_inverted(inverted)
{
    convertMinMax(minpos, maxpos, minval, maxval, m_minlog, m_ratio);

//    cerr << "LogRangeMapper: minpos " << minpos << ", maxpos "
//              << maxpos << ", minval " << minval << ", maxval "
//              << maxval << ", minlog " << m_minlog << ", ratio " << m_ratio
//              << ", unit " << unit << endl;

    assert(m_maxpos != m_minpos);

    m_maxlog = (m_maxpos - m_minpos) / m_ratio + m_minlog;

//    cerr << "LogRangeMapper: maxlog = " << m_maxlog << endl;
}

void
LogRangeMapper::convertMinMax(int minpos, int maxpos,
                              double minval, double maxval,
                              double &minlog, double &ratio)
{
    static double thresh = powf(10, -10);
    if (minval < thresh) minval = thresh;
    minlog = log10(minval);
    ratio = (maxpos - minpos) / (log10(maxval) - minlog);
}

void
LogRangeMapper::convertRatioMinLog(double ratio, double minlog,
                                   int minpos, int maxpos,
                                   double &minval, double &maxval)
{
    minval = pow(10, minlog);
    maxval = pow(10, (maxpos - minpos) / ratio + minlog);
}

int
LogRangeMapper::getPositionForValue(double value) const
{
    int position = getPositionForValueUnclamped(value);
    if (position < m_minpos) position = m_minpos;
    if (position > m_maxpos) position = m_maxpos;
    return position;
}

int
LogRangeMapper::getPositionForValueUnclamped(double value) const
{
    static double thresh = pow(10, -10);
    if (value < thresh) value = thresh;
    int position = int(lrint((log10(value) - m_minlog) * m_ratio)) + m_minpos;
    if (m_inverted) return m_maxpos - (position - m_minpos);
    else return position;
}

double
LogRangeMapper::getValueForPosition(int position) const
{
    if (position < m_minpos) position = m_minpos;
    if (position > m_maxpos) position = m_maxpos;
    double value = getValueForPositionUnclamped(position);
    return value;
}

double
LogRangeMapper::getValueForPositionUnclamped(int position) const
{
    if (m_inverted) position = m_maxpos - (position - m_minpos);
    double value = pow(10, (position - m_minpos) / m_ratio + m_minlog);
    return value;
}

InterpolatingRangeMapper::InterpolatingRangeMapper(CoordMap pointMappings,
                                                   QString unit) :
    m_mappings(pointMappings),
    m_unit(unit)
{
    for (CoordMap::const_iterator i = m_mappings.begin(); 
         i != m_mappings.end(); ++i) {
        m_reverse[i->second] = i->first;
    }
}

int
InterpolatingRangeMapper::getPositionForValue(double value) const
{
    int pos = getPositionForValueUnclamped(value);
    CoordMap::const_iterator i = m_mappings.begin();
    if (pos < i->second) pos = i->second;
    i = m_mappings.end(); --i;
    if (pos > i->second) pos = i->second;
    return pos;
}

int
InterpolatingRangeMapper::getPositionForValueUnclamped(double value) const
{
    double p = interpolate(&m_mappings, value);
    return int(lrint(p));
}

double
InterpolatingRangeMapper::getValueForPosition(int position) const
{
    double val = getValueForPositionUnclamped(position);
    CoordMap::const_iterator i = m_mappings.begin();
    if (val < i->first) val = i->first;
    i = m_mappings.end(); --i;
    if (val > i->first) val = i->first;
    return val;
}

double
InterpolatingRangeMapper::getValueForPositionUnclamped(int position) const
{
    return interpolate(&m_reverse, position);
}

template <typename T>
double
InterpolatingRangeMapper::interpolate(T *mapping, double value) const
{
    // lower_bound: first element which does not compare less than value
    typename T::const_iterator i =
        mapping->lower_bound(typename T::key_type(value));

    if (i == mapping->begin()) {
        // value is less than or equal to first element, so use the
        // gradient from first to second and extend it
        ++i;
    }

    if (i == mapping->end()) {
        // value is off the end, so use the gradient from penultimate
        // to ultimate and extend it
        --i;
    }

    typename T::const_iterator j = i;
    --j;

    double gradient = double(i->second - j->second) / double(i->first - j->first);

    return j->second + (value - j->first) * gradient;
}

AutoRangeMapper::AutoRangeMapper(CoordMap pointMappings,
                                 QString unit) :
    m_mappings(pointMappings),
    m_unit(unit)
{
    m_type = chooseMappingTypeFor(m_mappings);

    CoordMap::const_iterator first = m_mappings.begin();
    CoordMap::const_iterator last = m_mappings.end();
    --last;

    switch (m_type) {
    case StraightLine:
        m_mapper = new LinearRangeMapper(first->second, last->second,
                                         first->first, last->first,
                                         unit, false);
        break;
    case Logarithmic:
        m_mapper = new LogRangeMapper(first->second, last->second,
                                      first->first, last->first,
                                      unit, false);
        break;
    case Interpolating:
        m_mapper = new InterpolatingRangeMapper(m_mappings, unit);
        break;
    }
}

AutoRangeMapper::~AutoRangeMapper()
{
    delete m_mapper;
}

AutoRangeMapper::MappingType
AutoRangeMapper::chooseMappingTypeFor(const CoordMap &mappings)
{
    // how do we work out whether a linear/log mapping is "close enough"?

    CoordMap::const_iterator first = mappings.begin();
    CoordMap::const_iterator last = mappings.end();
    --last;

    LinearRangeMapper linm(first->second, last->second,
                           first->first, last->first,
                           "", false);

    bool inadequate = false;

    for (CoordMap::const_iterator i = mappings.begin();
         i != mappings.end(); ++i) {
        int candidate = linm.getPositionForValue(i->first);
        int diff = candidate - i->second;
        if (diff < 0) diff = -diff;
        if (diff > 1) {
//            cerr << "AutoRangeMapper::chooseMappingTypeFor: diff = " << diff
//                 << ", straight-line mapping inadequate" << endl;
            inadequate = true;
            break;
        }
    }

    if (!inadequate) {
        return StraightLine;
    }

    LogRangeMapper logm(first->second, last->second,
                        first->first, last->first,
                        "", false);

    inadequate = false;

    for (CoordMap::const_iterator i = mappings.begin();
         i != mappings.end(); ++i) {
        int candidate = logm.getPositionForValue(i->first);
        int diff = candidate - i->second;
        if (diff < 0) diff = -diff;
        if (diff > 1) {
//            cerr << "AutoRangeMapper::chooseMappingTypeFor: diff = " << diff
//                 << ", log mapping inadequate" << endl;
            inadequate = true;
            break;
        }
    }

    if (!inadequate) {
        return Logarithmic;
    }

    return Interpolating;
}

int
AutoRangeMapper::getPositionForValue(double value) const
{
    return m_mapper->getPositionForValue(value);
}

double
AutoRangeMapper::getValueForPosition(int position) const
{
    return m_mapper->getValueForPosition(position);
}

int
AutoRangeMapper::getPositionForValueUnclamped(double value) const
{
    return m_mapper->getPositionForValueUnclamped(value);
}

double
AutoRangeMapper::getValueForPositionUnclamped(int position) const
{
    return m_mapper->getValueForPositionUnclamped(position);
}
