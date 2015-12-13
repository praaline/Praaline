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

#include "FFTFileCacheWriter.h"

#include "fileio/MatrixFile.h"

#include "base/Profiler.h"
#include "base/Thread.h"
#include "base/Exceptions.h"

#include <iostream>

//#define DEBUG_FFT_FILE_CACHE_WRITER 1


// The underlying matrix has height (m_height * 2 + 1).  In each
// column we store magnitude at [0], [2] etc and phase at [1], [3]
// etc, and then store the normalization factor (maximum magnitude) at
// [m_height * 2].  In compact mode, the factor takes two cells.

FFTFileCacheWriter::FFTFileCacheWriter(QString fileBase,
                                       FFTCache::StorageType storageType,
                                       int width, int height) :
    m_writebuf(0),
    m_fileBase(fileBase),
    m_storageType(storageType),
    m_factorSize(storageType == FFTCache::Compact ? 2 : 1),
    m_mfc(new MatrixFile
          (fileBase, MatrixFile::WriteOnly, 
           int((storageType == FFTCache::Compact) ? sizeof(uint16_t) : sizeof(float)),
           width, height * 2 + m_factorSize))
{
#ifdef DEBUG_FFT_FILE_CACHE_WRITER
    cerr << "FFTFileCacheWriter: storage type is " << (storageType == FFTCache::Compact ? "Compact" : storageType == FFTCache::Polar ? "Polar" : "Rectangular") << ", size " << width << "x" << height << endl;
#endif
    m_mfc->setAutoClose(true);
    m_writebuf = new char[(height * 2 + m_factorSize) * m_mfc->getCellSize()];
}

FFTFileCacheWriter::~FFTFileCacheWriter()
{
    if (m_writebuf) delete[] m_writebuf;
    delete m_mfc;
}

QString
FFTFileCacheWriter::getFileBase() const
{
    return m_fileBase;
}

int
FFTFileCacheWriter::getWidth() const
{
    return m_mfc->getWidth();
}

int
FFTFileCacheWriter::getHeight() const
{
    int mh = m_mfc->getHeight();
    if (mh > m_factorSize) return (mh - m_factorSize) / 2;
    else return 0;
}

bool
FFTFileCacheWriter::haveSetColumnAt(int x) const
{
    return m_mfc->haveSetColumnAt(x);
}

void
FFTFileCacheWriter::setColumnAt(int x, float *mags, float *phases, float factor)
{
    int h = getHeight();

    switch (m_storageType) {

    case FFTCache::Compact:
        for (int y = 0; y < h; ++y) {
            ((uint16_t *)m_writebuf)[y * 2] = uint16_t((mags[y] / factor) * 65535.0);
            ((uint16_t *)m_writebuf)[y * 2 + 1] = uint16_t(int16_t((phases[y] * 32767) / M_PI));
        }
        break;

    case FFTCache::Rectangular:
        for (int y = 0; y < h; ++y) {
            ((float *)m_writebuf)[y * 2] = mags[y] * cosf(phases[y]);
            ((float *)m_writebuf)[y * 2 + 1] = mags[y] * sinf(phases[y]);
        }
        break;

    case FFTCache::Polar:
        for (int y = 0; y < h; ++y) {
            ((float *)m_writebuf)[y * 2] = mags[y];
            ((float *)m_writebuf)[y * 2 + 1] = phases[y];
        }
        break;
    }

    static float maxFactor = 0;
    if (factor > maxFactor) maxFactor = factor;
#ifdef DEBUG_FFT_FILE_CACHE_WRITER
    cerr << "Column " << x << ": normalization factor: " << factor << ", max " << maxFactor << " (height " << getHeight() << ")" << endl;
#endif

    setNormalizationFactorToWritebuf(factor);

    m_mfc->setColumnAt(x, m_writebuf);
}

void
FFTFileCacheWriter::setColumnAt(int x, float *real, float *imag)
{
    int h = getHeight();

    float factor = 0.0f;

    switch (m_storageType) {

    case FFTCache::Compact:
        for (int y = 0; y < h; ++y) {
            float mag = sqrtf(real[y] * real[y] + imag[y] * imag[y]);
            if (mag > factor) factor = mag;
        }
        for (int y = 0; y < h; ++y) {
            float mag = sqrtf(real[y] * real[y] + imag[y] * imag[y]);
            float phase = atan2f(imag[y], real[y]);
            ((uint16_t *)m_writebuf)[y * 2] = uint16_t((mag / factor) * 65535.0);
            ((uint16_t *)m_writebuf)[y * 2 + 1] = uint16_t(int16_t((phase * 32767) / M_PI));
        }
        break;

    case FFTCache::Rectangular:
        for (int y = 0; y < h; ++y) {
            ((float *)m_writebuf)[y * 2] = real[y];
            ((float *)m_writebuf)[y * 2 + 1] = imag[y];
            float mag = sqrtf(real[y] * real[y] + imag[y] * imag[y]);
            if (mag > factor) factor = mag;
        }
        break;

    case FFTCache::Polar:
        for (int y = 0; y < h; ++y) {
            float mag = sqrtf(real[y] * real[y] + imag[y] * imag[y]);
            if (mag > factor) factor = mag;
            ((float *)m_writebuf)[y * 2] = mag;
            float phase = atan2f(imag[y], real[y]);
            ((float *)m_writebuf)[y * 2 + 1] = phase;
        }
        break;
    }

    static float maxFactor = 0;
    if (factor > maxFactor) maxFactor = factor;
#ifdef DEBUG_FFT_FILE_CACHE_WRITER
    cerr << "[RI] Column " << x << ": normalization factor: " << factor << ", max " << maxFactor << " (height " << getHeight() << ")" << endl;
#endif

    setNormalizationFactorToWritebuf(factor);

    m_mfc->setColumnAt(x, m_writebuf);
}

size_t
FFTFileCacheWriter::getCacheSize(int width, int height,
                                 FFTCache::StorageType type)
{
    return (height * 2 + (type == FFTCache::Compact ? 2 : 1)) * width *
        (type == FFTCache::Compact ? sizeof(uint16_t) : sizeof(float)) +
        2 * sizeof(int); // matrix file header size
}

void
FFTFileCacheWriter::allColumnsWritten()
{
#ifdef DEBUG_FFT_FILE_CACHE_WRITER
    cerr << "FFTFileCacheWriter::allColumnsWritten" << endl;
#endif
    m_mfc->close();
}

