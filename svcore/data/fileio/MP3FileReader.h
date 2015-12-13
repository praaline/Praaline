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

#ifndef _MP3_FILE_READER_H_
#define _MP3_FILE_READER_H_

#ifdef HAVE_MAD

#include "CodedAudioFileReader.h"

#include "base/Thread.h"
#include <mad.h>

#include <set>

class ProgressReporter;

class MP3FileReader : public CodedAudioFileReader
{
    Q_OBJECT

public:
    enum DecodeMode {
        DecodeAtOnce, // decode the file on construction, with progress
        DecodeThreaded // decode in a background thread after construction
    };

    MP3FileReader(FileSource source,
                  DecodeMode decodeMode,
                  CacheMode cacheMode,
                  sv_samplerate_t targetRate = 0,
                  bool normalised = false,
                  ProgressReporter *reporter = 0);
    virtual ~MP3FileReader();

    virtual QString getError() const { return m_error; }

    virtual QString getLocation() const { return m_source.getLocation(); }
    virtual QString getTitle() const { return m_title; }
    virtual QString getMaker() const { return m_maker; }
    virtual TagMap getTags() const { return m_tags; }
    
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
    QString m_maker;
    TagMap m_tags;
    sv_frame_t m_fileSize;
    double m_bitrateNum;
    int m_bitrateDenom;
    int m_completion;
    bool m_done;

    unsigned char *m_filebuffer;
    float **m_samplebuffer;
    int m_samplebuffersize;

    ProgressReporter *m_reporter;
    bool m_cancelled;

    struct DecoderData
    {
	unsigned char const *start;
	unsigned long length;
	MP3FileReader *reader;
    };

    bool decode(void *mm, sv_frame_t sz);
    enum mad_flow accept(struct mad_header const *, struct mad_pcm *);

    static enum mad_flow input(void *, struct mad_stream *);
    static enum mad_flow output(void *, struct mad_header const *, struct mad_pcm *);
    static enum mad_flow error(void *, struct mad_stream *, struct mad_frame *);

    class DecodeThread : public Thread
    {
    public:
        DecodeThread(MP3FileReader *reader) : m_reader(reader) { }
        virtual void run();

    protected:
        MP3FileReader *m_reader;
    };

    DecodeThread *m_decodeThread;

    void loadTags();
    QString loadTag(void *vtag, const char *name);
};

#endif

#endif
