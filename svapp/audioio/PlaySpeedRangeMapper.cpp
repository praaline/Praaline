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

#include "PlaySpeedRangeMapper.h"

#include <iostream>
#include <cmath>

PlaySpeedRangeMapper::PlaySpeedRangeMapper(int minpos, int maxpos) :
    m_minpos(minpos),
    m_maxpos(maxpos)
{
}

int
PlaySpeedRangeMapper::getPositionForValue(double value) const
{
    // value is percent
    double factor = getFactorForValue(value);
    int position = getPositionForFactor(factor);
    return position;
}

int
PlaySpeedRangeMapper::getPositionForValueUnclamped(double value) const
{
    // We don't really provide this
    return getPositionForValue(value);
}

int
PlaySpeedRangeMapper::getPositionForFactor(double factor) const
{
    bool slow = (factor > 1.0);

    if (!slow) factor = 1.0 / factor;
    
    int half = (m_maxpos + m_minpos) / 2;

    factor = sqrt((factor - 1.0) * 1000.0);
    int position = int(lrint(((factor * (half - m_minpos)) / 100.0) + m_minpos));

    if (slow) {
        position = half - position;
    } else {
        position = position + half;
    }

//    cerr << "value = " << value << " slow = " << slow << " factor = " << factor << " position = " << position << endl;

    return position;
}

double
PlaySpeedRangeMapper::getValueForPosition(int position) const
{
    double factor = getFactorForPosition(position);
    double pc = getValueForFactor(factor);
    return pc;
}

double
PlaySpeedRangeMapper::getValueForPositionUnclamped(int position) const
{
    // We don't really provide this
    return getValueForPosition(position);
}

double
PlaySpeedRangeMapper::getValueForFactor(double factor) const
{
    double pc;
    if (factor < 1.0) pc = ((1.0 / factor) - 1.0) * 100.0;
    else pc = (1.0 - factor) * 100.0;
//    cerr << "position = " << position << " percent = " << pc << endl;
    return pc;
}

double
PlaySpeedRangeMapper::getFactorForValue(double value) const
{
    // value is percent
    
    double factor;

    if (value <= 0) {
        factor = 1.0 - (value / 100.0);
    } else {
        factor = 1.0 / (1.0 + (value / 100.0));
    }

//    cerr << "value = " << value << " factor = " << factor << endl;
    return factor;
}

double
PlaySpeedRangeMapper::getFactorForPosition(int position) const
{
    bool slow = false;

    if (position < m_minpos) position = m_minpos;
    if (position > m_maxpos) position = m_maxpos;

    int half = (m_maxpos + m_minpos) / 2;

    if (position < half) {
        slow = true;
        position = half - position;
    } else {
        position = position - half;
    }

    // position is between min and half (inclusive)

    double factor;

    if (position == m_minpos) {
        factor = 1.0;
    } else {
        factor = ((position - m_minpos) * 100.0) / (half - m_minpos);
        factor = 1.0 + (factor * factor) / 1000.f;
    }

    if (!slow) factor = 1.0 / factor;

//    cerr << "position = " << position << " slow = " << slow << " factor = " << factor << endl;

    return factor;
}

QString
PlaySpeedRangeMapper::getUnit() const
{
    return "%";
}
