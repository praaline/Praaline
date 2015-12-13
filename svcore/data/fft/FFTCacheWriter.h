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

#ifndef _FFT_CACHE_WRITER_H_
#define _FFT_CACHE_WRITER_H_

#include <stddef.h>

class FFTCacheWriter
{
public:
    virtual ~FFTCacheWriter() { }

    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;

    virtual void setColumnAt(int x, float *mags, float *phases, float factor) = 0;
    virtual void setColumnAt(int x, float *reals, float *imags) = 0;

    virtual bool haveSetColumnAt(int x) const = 0;

    virtual void allColumnsWritten() = 0; // notify cache to close

    virtual FFTCache::StorageType getStorageType() const = 0;
};

#endif

