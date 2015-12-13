/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2012 Chris Cannam and QMUL.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _COREAUDIO_FILE_READER_H_
#define _COREAUDIO_FILE_READER_H_

#ifdef HAVE_COREAUDIO

#include "CodedAudioFileReader.h"

#include "base/Thread.h"

#include <set>

class ProgressReporter;

class CoreAudioFileReader : public CodedAudioFileReader
{
    Q_OBJECT

public:
    enum DecodeMode {
        DecodeAtOnce, // decode the file on construction, with progress
        DecodeThreaded // decode in a background thread after construction
    };

    CoreAudioFileReader(FileSource source,
                        DecodeMode decodeMode,
                        CacheMode cacheMode,
                        sv_samplerate_t targetRate = 0,
                        bool normalised = false,
                        ProgressReporter *reporter = 0);
    virtual ~CoreAudioFileReader();

    virtual QString getError() const { return m_error; }
    virtual QString getLocation() const { return m_source.getLocation(); }
    virtual QString getTitle() const { return m_title; }
    
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
    QString m_title;

    class D;
    D *m_d;

    ProgressReporter *m_reporter;
    bool m_cancelled;
    int m_completion;

    class DecodeThread : public Thread
    {
    public:
       // DecodeThread(QuickTimeFileReader *reader) : m_reader(reader) { }
        virtual void run();

    protected:
       // QuickTimeFileReader *m_reader;
    };

    DecodeThread *m_decodeThread;
};

#endif

#endif
