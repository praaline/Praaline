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

#ifndef _PLAY_SPEED_RANGE_MAPPER_H_
#define _PLAY_SPEED_RANGE_MAPPER_H_

#include "base/RangeMapper.h"

class PlaySpeedRangeMapper : public RangeMapper
{
public:
    PlaySpeedRangeMapper(int minpos, int maxpos);

    virtual int getPositionForValue(double value) const;
    virtual int getPositionForValueUnclamped(double value) const;

    virtual double getValueForPosition(int position) const;
    virtual double getValueForPositionUnclamped(int position) const;

    int getPositionForFactor(double factor) const;
    double getValueForFactor(double factor) const;

    double getFactorForPosition(int position) const;
    double getFactorForValue(double value) const;

    virtual QString getUnit() const;
    
protected:
    int m_minpos;
    int m_maxpos;
};


#endif
