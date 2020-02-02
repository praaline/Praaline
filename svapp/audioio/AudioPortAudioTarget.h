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

#ifndef _AUDIO_PORT_AUDIO_TARGET_H_
#define _AUDIO_PORT_AUDIO_TARGET_H_

#ifdef HAVE_PORTAUDIO_2_0

// This code requires PortAudio v19 -- it won't work with v18.

#include <portaudio.h>

#include <QObject>

#include "AudioCallbackPlayTarget.h"

#include "PraalineCore/Base/BaseTypes.h"

class AudioCallbackPlaySource;

class AudioPortAudioTarget : public AudioCallbackPlayTarget
{
    Q_OBJECT

public:
    AudioPortAudioTarget(AudioCallbackPlaySource *source);
    virtual ~AudioPortAudioTarget();

    virtual void shutdown();

    virtual bool isOK() const;

    virtual double getCurrentTime() const;

public slots:
    virtual void sourceModelReplaced();

protected:
    int process(const void *input, void *output, sv_frame_t frames,
                const PaStreamCallbackTimeInfo *timeInfo,
                PaStreamCallbackFlags statusFlags);

    static int processStatic(const void *, void *, unsigned long,
                             const PaStreamCallbackTimeInfo *,
                             PaStreamCallbackFlags, void *);

    PaStream *m_stream;

    int m_bufferSize;
    int m_sampleRate;
    int m_latency;
    bool m_prioritySet;
    bool m_done;
};

#endif /* HAVE_PORTAUDIO */

#endif

