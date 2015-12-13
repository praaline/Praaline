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

#ifndef _CODED_AUDIO_FILE_READER_H_
#define _CODED_AUDIO_FILE_READER_H_

#include "AudioFileReader.h"

#include <sndfile.h>
#include <QMutex>
#include <QReadWriteLock>

class WavFileReader;
class Serialiser;
class Resampler;

class CodedAudioFileReader : public AudioFileReader
{
    Q_OBJECT

public:
    virtual ~CodedAudioFileReader();

    enum CacheMode {
        CacheInTemporaryFile,
        CacheInMemory
    };

    virtual SampleBlock getInterleavedFrames(sv_frame_t start, sv_frame_t count) const;

    virtual sv_samplerate_t getNativeRate() const { return m_fileRate; }

    virtual QString getLocalFilename() const { return m_cacheFileName; }
    
    /// Intermediate cache means all CodedAudioFileReaders are quickly seekable
    virtual bool isQuicklySeekable() const { return true; }

signals:
    void progress(int);

protected:
    CodedAudioFileReader(CacheMode cacheMode, 
                         sv_samplerate_t targetRate,
                         bool normalised);

    void initialiseDecodeCache(); // samplerate, channels must have been set

    // may throw InsufficientDiscSpace:
    void addSamplesToDecodeCache(float **samples, sv_frame_t nframes);
    void addSamplesToDecodeCache(float *samplesInterleaved, sv_frame_t nframes);
    void addSamplesToDecodeCache(const SampleBlock &interleaved);

    // may throw InsufficientDiscSpace:
    void finishDecodeCache();

    bool isDecodeCacheInitialised() const { return m_initialised; }

    void startSerialised(QString id);
    void endSerialised();

private:
    void pushBuffer(float *interleaved, sv_frame_t sz, bool final);
    void pushBufferResampling(float *interleaved, sv_frame_t sz, double ratio, bool final);
    void pushBufferNonResampling(float *interleaved, sv_frame_t sz);

protected:
    QMutex m_cacheMutex;
    CacheMode m_cacheMode;
    SampleBlock m_data;
    mutable QReadWriteLock m_dataLock;
    bool m_initialised;
    Serialiser *m_serialiser;
    sv_samplerate_t m_fileRate;

    QString m_cacheFileName;
    SNDFILE *m_cacheFileWritePtr;
    WavFileReader *m_cacheFileReader;
    float *m_cacheWriteBuffer;
    sv_frame_t m_cacheWriteBufferIndex;
    sv_frame_t m_cacheWriteBufferSize; // frames

    Resampler *m_resampler;
    float *m_resampleBuffer;
    sv_frame_t m_fileFrameCount;

    bool m_normalised;
    float m_max;
    float m_gain;
};

#endif
