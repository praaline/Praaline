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

#ifndef _FFT_MEMORY_CACHE_H_
#define _FFT_MEMORY_CACHE_H_

#include "FFTCacheReader.h"
#include "FFTCacheWriter.h"
#include "FFTCacheStorageType.h"
#include "base/ResizeableBitset.h"
#include "base/Profiler.h"

#include <QReadWriteLock>

/**
 * In-memory FFT cache.  For this we want to cache magnitude with
 * enough resolution to have gain applied afterwards and determine
 * whether something is a peak or not, and also cache phase rather
 * than only phase-adjusted frequency so that we don't have to
 * recalculate if switching between phase and magnitude displays.  At
 * the same time, we don't want to take up too much memory.  It's not
 * expected to be accurate enough to be used as input for DSP or
 * resynthesis code.
 *
 * This implies probably 16 bits for a normalized magnitude and at
 * most 16 bits for phase.
 *
 * Each column's magnitudes are expected to be stored normalized
 * to [0,1] with respect to the column, so the normalization
 * factor should be calculated before all values in a column, and
 * set appropriately.
 */

class FFTMemoryCache : public FFTCacheReader, public FFTCacheWriter
{
public:
    FFTMemoryCache(FFTCache::StorageType storageType,
                   int width, int height);
    ~FFTMemoryCache();
	
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
	
    float getMagnitudeAt(int x, int y) const {
        if (m_storageType == FFTCache::Rectangular) {
            Profiler profiler("FFTMemoryCache::getMagnitudeAt: cart to polar");
            return sqrtf(m_freal[x][y] * m_freal[x][y] +
                         m_fimag[x][y] * m_fimag[x][y]);
        } else {
            return getNormalizedMagnitudeAt(x, y) * m_factor[x];
        }
    }
    
    float getNormalizedMagnitudeAt(int x, int y) const {
        if (m_storageType == FFTCache::Rectangular) return getMagnitudeAt(x, y) / m_factor[x];
        else if (m_storageType == FFTCache::Polar) return m_fmagnitude[x][y];
        else return float(m_magnitude[x][y]) / 65535.f;
    }
    
    float getMaximumMagnitudeAt(int x) const {
        return m_factor[x];
    }
    
    float getPhaseAt(int x, int y) const {
        if (m_storageType == FFTCache::Rectangular) {
            Profiler profiler("FFTMemoryCache::getValuesAt: cart to polar");
            return atan2f(m_fimag[x][y], m_freal[x][y]);
        } else if (m_storageType == FFTCache::Polar) {
            return m_fphase[x][y];
        } else {
            int16_t i = (int16_t)m_phase[x][y];
            return float(i / 32767.0 * M_PI);
        }
    }
    
    void getValuesAt(int x, int y, float &real, float &imag) const {
        if (m_storageType == FFTCache::Rectangular) {
            real = m_freal[x][y];
            imag = m_fimag[x][y];
        } else {
            Profiler profiler("FFTMemoryCache::getValuesAt: polar to cart");
            float mag = getMagnitudeAt(x, y);
            float phase = getPhaseAt(x, y);
            real = mag * cosf(phase);
            imag = mag * sinf(phase);
        }
    }

    void getMagnitudesAt(int x, float *values, int minbin, int count, int step) const
    {
        if (m_storageType == FFTCache::Rectangular) {
            for (int i = 0; i < count; ++i) {
                int y = i * step + minbin;
                values[i] = sqrtf(m_freal[x][y] * m_freal[x][y] +
                                  m_fimag[x][y] * m_fimag[x][y]);
            }
        } else if (m_storageType == FFTCache::Polar) {
            for (int i = 0; i < count; ++i) {
                int y = i * step + minbin;
                values[i] = m_fmagnitude[x][y] * m_factor[x];
            }
        } else {
            for (int i = 0; i < count; ++i) {
                int y = i * step + minbin;
                values[i] = float(double(m_magnitude[x][y]) * m_factor[x] / 65535.0);
            }
        }
    }

    bool haveSetColumnAt(int x) const {
        m_colsetLock.lockForRead();
        bool have = m_colset.get(x);
        m_colsetLock.unlock();
        return have;
    }

    void setColumnAt(int x, float *mags, float *phases, float factor);

    void setColumnAt(int x, float *reals, float *imags);

    void allColumnsWritten() { } 

    static size_t getCacheSize(int width, int height,
                               FFTCache::StorageType type);

    FFTCache::StorageType getStorageType() const { return m_storageType; }

private:
    int m_width;
    int m_height;
    uint16_t **m_magnitude;
    uint16_t **m_phase;
    float **m_fmagnitude;
    float **m_fphase;
    float **m_freal;
    float **m_fimag;
    float *m_factor;
    FFTCache::StorageType m_storageType;
    ResizeableBitset m_colset;
    mutable QReadWriteLock m_colsetLock;

    void initialise();

    void setNormalizationFactor(int x, float factor) {
        if (x < m_width) m_factor[x] = factor;
    }
    
    void setMagnitudeAt(int x, int y, float mag) {
         // norm factor must already be set
        setNormalizedMagnitudeAt(x, y, mag / m_factor[x]);
    }
    
    void setNormalizedMagnitudeAt(int x, int y, float norm) {
        if (x < m_width && y < m_height) {
            if (m_storageType == FFTCache::Polar) m_fmagnitude[x][y] = norm;
            else m_magnitude[x][y] = uint16_t(norm * 65535.0);
        }
    }
    
    void setPhaseAt(int x, int y, float phase) {
        // phase in range -pi -> pi
        if (x < m_width && y < m_height) {
            if (m_storageType == FFTCache::Polar) m_fphase[x][y] = phase;
            else m_phase[x][y] = uint16_t(int16_t((phase * 32767) / M_PI));
        }
    }

    void initialise(uint16_t **&);
    void initialise(float **&);
};


#endif

