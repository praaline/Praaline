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

#include "FFTMemoryCache.h"
#include "system/System.h"

#include <iostream>
#include <cstdlib>

//#define DEBUG_FFT_MEMORY_CACHE 1

FFTMemoryCache::FFTMemoryCache(FFTCache::StorageType storageType,
                               int width, int height) :
    m_width(width),
    m_height(height),
    m_magnitude(0),
    m_phase(0),
    m_fmagnitude(0),
    m_fphase(0),
    m_freal(0),
    m_fimag(0),
    m_factor(0),
    m_storageType(storageType)
{
#ifdef DEBUG_FFT_MEMORY_CACHE
    cerr << "FFTMemoryCache[" << this << "]::FFTMemoryCache (type "
              << m_storageType << "), size " << m_width << "x" << m_height << endl;
#endif

    initialise();
}

FFTMemoryCache::~FFTMemoryCache()
{
#ifdef DEBUG_FFT_MEMORY_CACHE
    cerr << "FFTMemoryCache[" << this << "]::~FFTMemoryCache" << endl;
#endif

    for (int i = 0; i < m_width; ++i) {
	if (m_magnitude && m_magnitude[i]) free(m_magnitude[i]);
	if (m_phase && m_phase[i]) free(m_phase[i]);
	if (m_fmagnitude && m_fmagnitude[i]) free(m_fmagnitude[i]);
	if (m_fphase && m_fphase[i]) free(m_fphase[i]);
        if (m_freal && m_freal[i]) free(m_freal[i]);
        if (m_fimag && m_fimag[i]) free(m_fimag[i]);
    }

    if (m_magnitude) free(m_magnitude);
    if (m_phase) free(m_phase);
    if (m_fmagnitude) free(m_fmagnitude);
    if (m_fphase) free(m_fphase);
    if (m_freal) free(m_freal);
    if (m_fimag) free(m_fimag);
    if (m_factor) free(m_factor);
}

void
FFTMemoryCache::initialise()
{
    Profiler profiler("FFTMemoryCache::initialise");

    int width = m_width, height = m_height;

#ifdef DEBUG_FFT_MEMORY_CACHE
    cerr << "FFTMemoryCache[" << this << "]::initialise(" << width << "x" << height << " = " << width*height << ")" << endl;
#endif

    if (m_storageType == FFTCache::Compact) {
        initialise(m_magnitude);
        initialise(m_phase);
    } else if (m_storageType == FFTCache::Polar) {
        initialise(m_fmagnitude);
        initialise(m_fphase);
    } else {
        initialise(m_freal);
        initialise(m_fimag);
    }

    m_colset.resize(width);

    m_factor = (float *)realloc(m_factor, width * sizeof(float));

    m_width = width;
    m_height = height;

#ifdef DEBUG_FFT_MEMORY_CACHE
    cerr << "done, width = " << m_width << " height = " << m_height << endl;
#endif
}

void
FFTMemoryCache::initialise(uint16_t **&array)
{
    array = (uint16_t **)malloc(m_width * sizeof(uint16_t *));
    if (!array) throw std::bad_alloc();
    MUNLOCK(array, m_width * sizeof(uint16_t *));

    for (int i = 0; i < m_width; ++i) {
	array[i] = (uint16_t *)malloc(m_height * sizeof(uint16_t));
	if (!array[i]) throw std::bad_alloc();
	MUNLOCK(array[i], m_height * sizeof(uint16_t));
    }
}

void
FFTMemoryCache::initialise(float **&array)
{
    array = (float **)malloc(m_width * sizeof(float *));
    if (!array) throw std::bad_alloc();
    MUNLOCK(array, m_width * sizeof(float *));

    for (int i = 0; i < m_width; ++i) {
	array[i] = (float *)malloc(m_height * sizeof(float));
	if (!array[i]) throw std::bad_alloc();
	MUNLOCK(array[i], m_height * sizeof(float));
    }
}

void
FFTMemoryCache::setColumnAt(int x, float *mags, float *phases, float factor)
{
    Profiler profiler("FFTMemoryCache::setColumnAt: from polar");

    setNormalizationFactor(x, factor);

    if (m_storageType == FFTCache::Rectangular) {
        Profiler subprof("FFTMemoryCache::setColumnAt: polar to cart");
        for (int y = 0; y < m_height; ++y) {
            m_freal[x][y] = mags[y] * cosf(phases[y]);
            m_fimag[x][y] = mags[y] * sinf(phases[y]);
        }
    } else {
        for (int y = 0; y < m_height; ++y) {
            setMagnitudeAt(x, y, mags[y]);
            setPhaseAt(x, y, phases[y]);
        }
    }

    m_colsetLock.lockForWrite();
    m_colset.set(x);
    m_colsetLock.unlock();
}

void
FFTMemoryCache::setColumnAt(int x, float *reals, float *imags)
{
    Profiler profiler("FFTMemoryCache::setColumnAt: from cart");

    float max = 0.0;

    switch (m_storageType) {

    case FFTCache::Rectangular:
        for (int y = 0; y < m_height; ++y) {
            m_freal[x][y] = reals[y];
            m_fimag[x][y] = imags[y];
            float mag = sqrtf(reals[y] * reals[y] + imags[y] * imags[y]);
            if (mag > max) max = mag;
        }
        break;

    case FFTCache::Compact:
    case FFTCache::Polar:
    {
        Profiler subprof("FFTMemoryCache::setColumnAt: cart to polar");
        for (int y = 0; y < m_height; ++y) {
            float mag = sqrtf(reals[y] * reals[y] + imags[y] * imags[y]);
            float phase = atan2f(imags[y], reals[y]);
            reals[y] = mag;
            imags[y] = phase;
            if (mag > max) max = mag;
        }
        break;
    }
    };

    if (m_storageType == FFTCache::Rectangular) {
        m_factor[x] = max;
        m_colsetLock.lockForWrite();
        m_colset.set(x);
        m_colsetLock.unlock();
    } else {
        setColumnAt(x, reals, imags, max);
    }
}

size_t
FFTMemoryCache::getCacheSize(int width, int height, FFTCache::StorageType type)
{
    size_t sz = 0;

    switch (type) {

    case FFTCache::Compact:
        sz = (height * 2 + 1) * width * sizeof(uint16_t);
        break;

    case FFTCache::Polar:
    case FFTCache::Rectangular:
        sz = (height * 2 + 1) * width * sizeof(float);
        break;
    }

    return sz;
}

