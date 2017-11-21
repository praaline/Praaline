/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2007 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _RESAMPLING_WAV_FILE_READER_H_
#define _RESAMPLING_WAV_FILE_READER_H_

#include "CodedAudioFileReader.h"

#include "base/Thread.h"

#include <set>

class WavFileReader;
class ProgressReporter;

class ResamplingWavFileReader : public CodedAudioFileReader
{
    Q_OBJECT
public:
    enum ResampleMode {
        ResampleAtOnce, // resample the file on construction, with progress dialog
        ResampleThreaded // resample in a background thread after construction
    };

    ResamplingWavFileReader(FileSource source,
                            ResampleMode resampleMode,
                            CacheMode cacheMode,
                            size_t targetRate = 0,
                            ProgressReporter *reporter = 0);
    virtual ~ResamplingWavFileReader();

    virtual QString getError() const { return m_error; }
    virtual QString getLocation() const { return m_source.getLocation(); }
    static void getSupportedExtensions(std::set<QString> &extensions);
    static bool supportsExtension(QString ext);
    static bool supportsContentType(QString type);
    static bool supports(FileSource &source);

    virtual int getDecodeCompletion() const { return m_completion; }

    virtual bool isUpdating() const {
        return m_decodeThread && m_decodeThread->isRunning();
    }

public slots:
    void cancelled();

protected:
    FileSource m_source;
    QString m_path;
    QString m_error;
    bool m_cancelled;
    size_t m_processed;
    int m_completion;

    WavFileReader *m_original;
    ProgressReporter *m_reporter;

    void addBlock(const SampleBlock &frames);
    
    class DecodeThread : public Thread
    {
    public:
        DecodeThread(ResamplingWavFileReader *reader) : m_reader(reader) { }
        virtual void run();

    protected:
        ResamplingWavFileReader *m_reader;
    };

    DecodeThread *m_decodeThread;
};

#endif

