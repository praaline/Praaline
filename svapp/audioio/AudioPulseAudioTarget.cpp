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

#ifdef HAVE_LIBPULSE

#include "AudioPulseAudioTarget.h"
#include "AudioCallbackPlaySource.h"

#include <QMutexLocker>

#include <iostream>
#include <cassert>
#include <cmath>

#define DEBUG_AUDIO_PULSE_AUDIO_TARGET 1
//#define DEBUG_AUDIO_PULSE_AUDIO_TARGET_PLAY 1

AudioPulseAudioTarget::AudioPulseAudioTarget(AudioCallbackPlaySource *source) :
    AudioCallbackPlayTarget(source),
    m_mutex(QMutex::Recursive),
    m_loop(0),
    m_api(0),
    m_context(0),
    m_stream(0),
    m_loopThread(0),
    m_bufferSize(0),
    m_sampleRate(0),
    m_latency(0),
    m_done(false)
{
#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget: Initialising for PulseAudio" << endl;
#endif

    m_loop = pa_mainloop_new();
    if (!m_loop) {
        cerr << "ERROR: AudioPulseAudioTarget: Failed to create main loop" << endl;
        return;
    }

    m_api = pa_mainloop_get_api(m_loop);

    //!!! handle signals how?

    m_bufferSize = 20480;
    m_sampleRate = 44100;
    if (m_source && (m_source->getSourceSampleRate() != 0)) {
	m_sampleRate = int(m_source->getSourceSampleRate());
    }
    m_spec.rate = m_sampleRate;
    m_spec.channels = 2;
    m_spec.format = PA_SAMPLE_FLOAT32NE;

#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget: Creating context" << endl;
#endif

    m_context = pa_context_new(m_api, source->getClientName().toLocal8Bit().data());
    if (!m_context) {
        cerr << "ERROR: AudioPulseAudioTarget: Failed to create context object" << endl;
        return;
    }

    pa_context_set_state_callback(m_context, contextStateChangedStatic, this);

#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget: Connecting to default server..." << endl;
#endif

    pa_context_connect(m_context, 0, // default server
                       (pa_context_flags_t)PA_CONTEXT_NOAUTOSPAWN, 0);

#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget: Starting main loop" << endl;
#endif

    m_loopThread = new MainLoopThread(m_loop);
    m_loopThread->start();

#ifdef DEBUG_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget: initialised OK" << endl;
#endif
}

AudioPulseAudioTarget::~AudioPulseAudioTarget()
{
    cerr << "AudioPulseAudioTarget::~AudioPulseAudioTarget()" << endl;

    if (m_source) {
        m_source->setTarget(0, m_bufferSize);
    }

    shutdown();

    QMutexLocker locker(&m_mutex);

    if (m_stream) pa_stream_unref(m_stream);

    if (m_context) pa_context_unref(m_context);

    if (m_loop) {
        pa_signal_done();
        pa_mainloop_free(m_loop);
    }

    m_stream = 0;
    m_context = 0;
    m_loop = 0;

    cerr << "AudioPulseAudioTarget::~AudioPulseAudioTarget() done" << endl;
}

void 
AudioPulseAudioTarget::shutdown()
{
    m_done = true;
}

bool
AudioPulseAudioTarget::isOK() const
{
    return (m_context != 0);
}

double
AudioPulseAudioTarget::getCurrentTime() const
{
    if (!m_stream) return 0.0;
    
    pa_usec_t usec = 0;
    pa_stream_get_time(m_stream, &usec);
    return double(usec) / 1000000.0;
}

void
AudioPulseAudioTarget::sourceModelReplaced()
{
    m_source->setTargetSampleRate(m_sampleRate);
}

void
AudioPulseAudioTarget::streamWriteStatic(pa_stream *,
                                         size_t length,
                                         void *data)
{
    AudioPulseAudioTarget *target = (AudioPulseAudioTarget *)data;
    
//    assert(stream == target->m_stream);

    target->streamWrite(length);
}

void
AudioPulseAudioTarget::streamWrite(sv_frame_t requested)
{
#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET_PLAY
    cout << "AudioPulseAudioTarget::streamWrite(" << requested << ")" << endl;
#endif
    if (m_done) return;

    QMutexLocker locker(&m_mutex);

    pa_usec_t latency = 0;
    int negative = 0;
    if (!pa_stream_get_latency(m_stream, &latency, &negative)) {
        int latframes = int(double(latency) / 1000000.0 * double(m_sampleRate));
        if (latframes > 0) m_source->setTargetPlayLatency(latframes);
    }

    static float *output = 0;
    static float **tmpbuf = 0;
    static int tmpbufch = 0;
    static sv_frame_t tmpbufsz = 0;

    int sourceChannels = m_source->getSourceChannelCount();

    // Because we offer pan, we always want at least 2 channels
    if (sourceChannels < 2) sourceChannels = 2;

    sv_frame_t nframes = requested / (sourceChannels * sizeof(float));

    if (nframes > m_bufferSize) {
        cerr << "WARNING: AudioPulseAudioTarget::streamWrite: nframes " << nframes << " > m_bufferSize " << m_bufferSize << endl;
    }

#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET_PLAY
    cout << "AudioPulseAudioTarget::streamWrite: nframes = " << nframes << endl;
#endif

    if (!tmpbuf || tmpbufch != sourceChannels || int(tmpbufsz) < nframes) {

	if (tmpbuf) {
	    for (int i = 0; i < tmpbufch; ++i) {
		delete[] tmpbuf[i];
	    }
	    delete[] tmpbuf;
	}

        if (output) {
            delete[] output;
        }

	tmpbufch = sourceChannels;
	tmpbufsz = nframes;
	tmpbuf = new float *[tmpbufch];

	for (int i = 0; i < tmpbufch; ++i) {
	    tmpbuf[i] = new float[tmpbufsz];
	}

        output = new float[tmpbufsz * tmpbufch];
    }
	
    sv_frame_t received = m_source->getSourceSamples(nframes, tmpbuf);

#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET_PLAY
    cerr << "requested " << nframes << ", received " << received << endl;

    if (received < nframes) {
        cerr << "*** WARNING: Wrong number of frames received" << endl;
    }
#endif

    float peakLeft = 0.0, peakRight = 0.0;

    for (int ch = 0; ch < 2; ++ch) {
	
	float peak = 0.0;

        // PulseAudio samples are interleaved
        for (int i = 0; i < nframes; ++i) {
            if (i < received) {
                output[i * 2 + ch] = tmpbuf[ch][i] * m_outputGain;
                float sample = fabsf(output[i * 2 + ch]);
                if (sample > peak) peak = sample;
            } else {
                output[i * 2 + ch] = 0;
            }
        }

	if (ch == 0) peakLeft = peak;
	if (ch == 1) peakRight = peak;
    }

#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET_PLAY
    cerr << "calling pa_stream_write with "
              << nframes * tmpbufch * sizeof(float) << " bytes" << endl;
#endif

    pa_stream_write(m_stream, output, nframes * tmpbufch * sizeof(float),
                    0, 0, PA_SEEK_RELATIVE);

    m_source->setOutputLevels(peakLeft, peakRight);

    return;
}

void
AudioPulseAudioTarget::streamStateChangedStatic(pa_stream *,
                                                void *data)
{
    AudioPulseAudioTarget *target = (AudioPulseAudioTarget *)data;
    
//    assert(stream == target->m_stream);

    target->streamStateChanged();
}

void
AudioPulseAudioTarget::streamStateChanged()
{
#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget::streamStateChanged" << endl;
#endif
    QMutexLocker locker(&m_mutex);

    switch (pa_stream_get_state(m_stream)) {

    case PA_STREAM_UNCONNECTED:
    case PA_STREAM_CREATING:
    case PA_STREAM_TERMINATED:
        break;

    case PA_STREAM_READY:
    {
        cerr << "AudioPulseAudioTarget::streamStateChanged: Ready" << endl;
        
        pa_usec_t latency = 0;
        int negative = 0;
        if (pa_stream_get_latency(m_stream, &latency, &negative)) {
            cerr << "AudioPulseAudioTarget::streamStateChanged: Failed to query latency" << endl;
        }
        cerr << "Latency = " << latency << " usec" << endl;
        int latframes = int(double(latency) / 1000000.0 * m_sampleRate);
        cerr << "that's " << latframes << " frames" << endl;

        const pa_buffer_attr *attr;
        if (!(attr = pa_stream_get_buffer_attr(m_stream))) {
            cerr << "AudioPulseAudioTarget::streamStateChanged: Cannot query stream buffer attributes" << endl;
            m_source->setTarget(this, m_bufferSize);
            m_source->setTargetSampleRate(m_sampleRate);
            if (latframes != 0) m_source->setTargetPlayLatency(latframes);
        } else {
            int targetLength = attr->tlength;
            cerr << "AudioPulseAudioTarget::streamStateChanged: stream target length = " << targetLength << endl;
            m_source->setTarget(this, targetLength);
            m_source->setTargetSampleRate(m_sampleRate);
            if (latframes == 0) latframes = targetLength;
            cerr << "latency = " << latframes << endl;
            m_source->setTargetPlayLatency(latframes);
        }
    }
    break;
    
    case PA_STREAM_FAILED:
    default:
        cerr << "AudioPulseAudioTarget::streamStateChanged: Error: "
             << pa_strerror(pa_context_errno(m_context)) << endl;
        //!!! do something...
        break;
    }
}

void
AudioPulseAudioTarget::contextStateChangedStatic(pa_context *,
                                                 void *data)
{
    AudioPulseAudioTarget *target = (AudioPulseAudioTarget *)data;
    
//    assert(context == target->m_context);

    target->contextStateChanged();
}

void
AudioPulseAudioTarget::contextStateChanged()
{
#ifdef DEBUG_AUDIO_PULSE_AUDIO_TARGET
    cerr << "AudioPulseAudioTarget::contextStateChanged" << endl;
#endif
    QMutexLocker locker(&m_mutex);

    switch (pa_context_get_state(m_context)) {

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            cerr << "AudioPulseAudioTarget::contextStateChanged: Ready"
                      << endl;

            m_stream = pa_stream_new(m_context, "stream", &m_spec, 0);
            assert(m_stream); //!!!
            
            pa_stream_set_state_callback(m_stream, streamStateChangedStatic, this);
            pa_stream_set_write_callback(m_stream, streamWriteStatic, this);
            pa_stream_set_overflow_callback(m_stream, streamOverflowStatic, this);
            pa_stream_set_underflow_callback(m_stream, streamUnderflowStatic, this);
            if (pa_stream_connect_playback
                (m_stream, 0, 0,
                 pa_stream_flags_t(PA_STREAM_INTERPOLATE_TIMING |
                                   PA_STREAM_AUTO_TIMING_UPDATE),
                 0, 0)) { //??? return value
                cerr << "AudioPulseAudioTarget: Failed to connect playback stream" << endl;
            }

            break;

        case PA_CONTEXT_TERMINATED:
            cerr << "AudioPulseAudioTarget::contextStateChanged: Terminated" << endl;
            //!!! do something...
            break;

        case PA_CONTEXT_FAILED:
        default:
            cerr << "AudioPulseAudioTarget::contextStateChanged: Error: "
                      << pa_strerror(pa_context_errno(m_context)) << endl;
            //!!! do something...
            break;
    }
}

void
AudioPulseAudioTarget::streamOverflowStatic(pa_stream *, void *)
{
    cerr << "AudioPulseAudioTarget::streamOverflowStatic: Overflow!" << endl;
}

void
AudioPulseAudioTarget::streamUnderflowStatic(pa_stream *, void *data)
{
    cerr << "AudioPulseAudioTarget::streamUnderflowStatic: Underflow!" << endl;
    AudioPulseAudioTarget *target = (AudioPulseAudioTarget *)data;
    if (target && target->m_source) {
        target->m_source->audioProcessingOverload();
    }
}

#endif /* HAVE_PULSEAUDIO */

