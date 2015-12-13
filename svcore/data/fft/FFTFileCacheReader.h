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

#ifndef _FFT_FILE_CACHE_READER_H_
#define _FFT_FILE_CACHE_READER_H_

#include "data/fileio/MatrixFile.h"
#include "FFTCacheReader.h"
#include "FFTCacheStorageType.h"

class FFTFileCacheWriter;

class FFTFileCacheReader : public FFTCacheReader
{
public:
    FFTFileCacheReader(FFTFileCacheWriter *);
    ~FFTFileCacheReader();

    int getWidth() const;
    int getHeight() const;
	
    float getMagnitudeAt(int x, int y) const;
    float getNormalizedMagnitudeAt(int x, int y) const;
    float getMaximumMagnitudeAt(int x) const;
    float getPhaseAt(int x, int y) const;

    void getValuesAt(int x, int y, float &real, float &imag) const;
    void getMagnitudesAt(int x, float *values, int minbin, int count, int step) const;

    bool haveSetColumnAt(int x) const;

    static size_t getCacheSize(int width, int height,
                               FFTCache::StorageType type);

    FFTCache::StorageType getStorageType() const { return m_storageType; }

protected:
    mutable char *m_readbuf;
    mutable int m_readbufCol;
    mutable int m_readbufWidth;
    mutable bool m_readbufGood;

    float getFromReadBufStandard(int x, int y) const {
        float v;
        if (m_readbuf &&
            (m_readbufCol == x || (m_readbufWidth > 1 && m_readbufCol+1 == x))) {
            v = ((float *)m_readbuf)[(x - m_readbufCol) * m_mfc->getHeight() + y];
            return v;
        } else {
            populateReadBuf(x);
            v = getFromReadBufStandard(x, y);
            return v;
        }
    }

    float getFromReadBufCompactUnsigned(int x, int y) const {
        float v;
        if (m_readbuf &&
            (m_readbufCol == x || (m_readbufWidth > 1 && m_readbufCol+1 == x))) {
            v = ((uint16_t *)m_readbuf)[(x - m_readbufCol) * m_mfc->getHeight() + y];
            return v;
        } else {
            populateReadBuf(x);
            v = getFromReadBufCompactUnsigned(x, y);
            return v;
        }
    }

    float getFromReadBufCompactSigned(int x, int y) const {
        float v;
        if (m_readbuf &&
            (m_readbufCol == x || (m_readbufWidth > 1 && m_readbufCol+1 == x))) {
            v = ((int16_t *)m_readbuf)[(x - m_readbufCol) * m_mfc->getHeight() + y];
            return v;
        } else {
            populateReadBuf(x);
            v = getFromReadBufCompactSigned(x, y);
            return v;
        }
    }

    void populateReadBuf(int x) const;

    float getNormalizationFactor(int col) const {
        int h = m_mfc->getHeight();
        if (h < m_factorSize) return 0;
        if (m_storageType != FFTCache::Compact) {
            return getFromReadBufStandard(col, h - 1);
        } else {
            union {
                float f;
                uint16_t u[2];
            } factor;
            if (!m_readbuf ||
                !(m_readbufCol == col ||
                  (m_readbufWidth > 1 && m_readbufCol+1 == col))) {
                populateReadBuf(col);
            }
            int ix = (col - m_readbufCol) * m_mfc->getHeight() + h;
            factor.u[0] = ((uint16_t *)m_readbuf)[ix - 2];
            factor.u[1] = ((uint16_t *)m_readbuf)[ix - 1];
            return factor.f;
        }
    }
 
    FFTCache::StorageType m_storageType;
    int m_factorSize;
    MatrixFile *m_mfc;
};

#endif
