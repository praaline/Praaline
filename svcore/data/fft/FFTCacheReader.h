/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2009 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _FFT_CACHE_READER_H_
#define _FFT_CACHE_READER_H_

#include "FFTCacheStorageType.h"
#include <stddef.h>

class FFTCacheReader
{
public:
    virtual ~FFTCacheReader() { }

    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
	
    virtual float getMagnitudeAt(int x, int y) const = 0;
    virtual float getNormalizedMagnitudeAt(int x, int y) const = 0;
    virtual float getMaximumMagnitudeAt(int x) const = 0;
    virtual float getPhaseAt(int x, int y) const = 0;

    virtual void getValuesAt(int x, int y, float &real, float &imag) const = 0;
    virtual void getMagnitudesAt(int x, float *values, int minbin, int count, int step) const = 0;

    virtual bool haveSetColumnAt(int x) const = 0;

    virtual FFTCache::StorageType getStorageType() const = 0;
};

#endif
