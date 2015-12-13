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

#ifndef _FFT_FILE_CACHE_WRITER_H_
#define _FFT_FILE_CACHE_WRITER_H_

#include "FFTCacheStorageType.h"
#include "FFTCacheWriter.h"
#include "data/fileio/MatrixFile.h"

class FFTFileCacheWriter : public FFTCacheWriter
{
public:
    FFTFileCacheWriter(QString fileBase,
                       FFTCache::StorageType storageType,
                       int width, int height);
    ~FFTFileCacheWriter();

    int getWidth() const;
    int getHeight() const;

    void setColumnAt(int x, float *mags, float *phases, float factor);
    void setColumnAt(int x, float *reals, float *imags);

    static size_t getCacheSize(int width, int height,
                               FFTCache::StorageType type);

    bool haveSetColumnAt(int x) const;

    void allColumnsWritten();

    QString getFileBase() const;
    FFTCache::StorageType getStorageType() const { return m_storageType; }

protected:
    char *m_writebuf;

    void setNormalizationFactorToWritebuf(float newfactor) {
        int h = m_mfc->getHeight();
        if (h < m_factorSize) return;
        if (m_storageType != FFTCache::Compact) {
            ((float *)m_writebuf)[h - 1] = newfactor;
        } else {
            union {
                float f;
                uint16_t u[2];
            } factor;
            factor.f = newfactor;
            ((uint16_t *)m_writebuf)[h - 2] = factor.u[0];
            ((uint16_t *)m_writebuf)[h - 1] = factor.u[1];
        }
    }            

    QString m_fileBase;
    FFTCache::StorageType m_storageType;
    int m_factorSize;
    MatrixFile *m_mfc;
};

#endif
