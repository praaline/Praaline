/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _AUDIO_PULSE_AUDIO_TARGET_H_
#define _AUDIO_PULSE_AUDIO_TARGET_H_

#ifdef HAVE_LIBPULSE

#include <pulse/pulseaudio.h>

#include <QObject>
#include <QMutex>
#include "base/Thread.h"

#include "AudioCallbackPlayTarget.h"

class AudioCallbackPlaySource;

class AudioPulseAudioTarget : public AudioCallbackPlayTarget
{
    Q_OBJECT

public:
    AudioPulseAudioTarget(AudioCallbackPlaySource *source);
    virtual ~AudioPulseAudioTarget();

    virtual void shutdown();

    virtual bool isOK() const;

    virtual double getCurrentTime() const;

public slots:
    virtual void sourceModelReplaced();

protected:
    void streamWrite(sv_frame_t);
    void streamStateChanged();
    void contextStateChanged();

    static void streamWriteStatic(pa_stream *, size_t, void *);
    static void streamStateChangedStatic(pa_stream *, void *);
    static void streamOverflowStatic(pa_stream *, void *);
    static void streamUnderflowStatic(pa_stream *, void *);
    static void contextStateChangedStatic(pa_context *, void *);

    QMutex m_mutex;

    class MainLoopThread : public Thread
    {
    public:
        MainLoopThread(pa_mainloop *loop) : Thread(NonRTThread), m_loop(loop) { } //!!! or RTThread
        virtual void run() {
            int rv = 0;
            pa_mainloop_run(m_loop, &rv); //!!! check return value from this, and rv
        }

    private:
        pa_mainloop *m_loop;
    };

    pa_mainloop *m_loop;
    pa_mainloop_api *m_api;
    pa_context *m_context;
    pa_stream *m_stream;
    pa_sample_spec m_spec;

    MainLoopThread *m_loopThread;

    int m_bufferSize;
    int m_sampleRate;
    int m_latency;
    bool m_done;
};

#endif /* HAVE_PULSEAUDIO */

#endif

