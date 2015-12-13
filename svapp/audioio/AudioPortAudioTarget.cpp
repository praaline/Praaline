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

#ifdef HAVE_PORTAUDIO_2_0

#include "AudioPortAudioTarget.h"
#include "AudioCallbackPlaySource.h"

#include <iostream>
#include <cassert>
#include <cmath>

#ifndef _WIN32
#include <pthread.h>
#endif

//#define DEBUG_AUDIO_PORT_AUDIO_TARGET 1

AudioPortAudioTarget::AudioPortAudioTarget(AudioCallbackPlaySource *source) :
    AudioCallbackPlayTarget(source),
    m_stream(0),
    m_bufferSize(0),
    m_sampleRate(0),
    m_latency(0),
    m_prioritySet(false),
    m_done(false)
{
    PaError err;

#ifdef DEBUG_AUDIO_PORT_AUDIO_TARGET
    cerr << "AudioPortAudioTarget: Initialising for PortAudio v19" << endl;
#endif

    err = Pa_Initialize();
    if (err != paNoError) {
	cerr << "ERROR: AudioPortAudioTarget: Failed to initialize PortAudio: " << Pa_GetErrorText(err) << endl;
	return;
    }

    m_bufferSize = 2048;
    m_sampleRate = 44100;
    if (m_source && (m_source->getSourceSampleRate() != 0)) {
	m_sampleRate = int(m_source->getSourceSampleRate());
    }

    PaStreamParameters op;
    op.device = Pa_GetDefaultOutputDevice();
    op.channelCount = 2;
    op.sampleFormat = paFloat32;
    op.suggestedLatency = 0.2;
    op.hostApiSpecificStreamInfo = 0;
    err = Pa_OpenStream(&m_stream, 0, &op, m_sampleRate,
                        paFramesPerBufferUnspecified,
                        paNoFlag, processStatic, this);

    if (err != paNoError) {

        cerr << "WARNING: AudioPortAudioTarget: Failed to open PortAudio stream with default frames per buffer, trying again with fixed frames per buffer..." << endl;
        
        err = Pa_OpenStream(&m_stream, 0, &op, m_sampleRate,
                            1024,
                            paNoFlag, processStatic, this);
	m_bufferSize = 1024;
    }

    if (err != paNoError) {
	cerr << "ERROR: AudioPortAudioTarget: Failed to open PortAudio stream: " << Pa_GetErrorText(err) << endl;
        cerr << "Note: device ID was " << op.device << endl;
	m_stream = 0;
	Pa_Terminate();
	return;
    }

    const PaStreamInfo *info = Pa_GetStreamInfo(m_stream);
    m_latency = int(info->outputLatency * m_sampleRate + 0.001);
    if (m_bufferSize < m_latency) m_bufferSize = m_latency;

    cerr << "PortAudio latency = " << m_latency << " frames" << endl;

    err = Pa_StartStream(m_stream);

    if (err != paNoError) {
	cerr << "ERROR: AudioPortAudioTarget: Failed to start PortAudio stream: " << Pa_GetErrorText(err) << endl;
	Pa_CloseStream(m_stream);
	m_stream = 0;
	Pa_Terminate();
	return;
    }

    if (m_source) {
	cerr << "AudioPortAudioTarget: block size " << m_bufferSize << endl;
	m_source->setTarget(this, m_bufferSize);
	m_source->setTargetSampleRate(m_sampleRate);
	m_source->setTargetPlayLatency(m_latency);
    }

#ifdef DEBUG_PORT_AUDIO_TARGET
    cerr << "AudioPortAudioTarget: initialised OK" << endl;
#endif
}

AudioPortAudioTarget::~AudioPortAudioTarget()
{
    cerr << "AudioPortAudioTarget::~AudioPortAudioTarget()" << endl;

    if (m_source) {
        m_source->setTarget(0, m_bufferSize);
    }

    shutdown();

    if (m_stream) {

        cerr << "closing stream" << endl;

	PaError err;
	err = Pa_CloseStream(m_stream);
	if (err != paNoError) {
	    cerr << "ERROR: AudioPortAudioTarget: Failed to close PortAudio stream: " << Pa_GetErrorText(err) << endl;
	}

        cerr << "terminating" << endl;

	err = Pa_Terminate();
        if (err != paNoError) {
            cerr << "ERROR: AudioPortAudioTarget: Failed to terminate PortAudio: " << Pa_GetErrorText(err) << endl;
	}   
    }

    m_stream = 0;

    cerr << "AudioPortAudioTarget::~AudioPortAudioTarget() done" << endl;
}

void 
AudioPortAudioTarget::shutdown()
{
#ifdef DEBUG_PORT_AUDIO_TARGET
    cerr << "AudioPortAudioTarget::shutdown" << endl;
#endif
    m_done = true;
}

bool
AudioPortAudioTarget::isOK() const
{
    return (m_stream != 0);
}

double
AudioPortAudioTarget::getCurrentTime() const
{
    if (!m_stream) return 0.0;
    else return Pa_GetStreamTime(m_stream);
}

int
AudioPortAudioTarget::processStatic(const void *input, void *output,
                                    unsigned long nframes,
                                    const PaStreamCallbackTimeInfo *timeInfo,
                                    PaStreamCallbackFlags flags, void *data)
{
    return ((AudioPortAudioTarget *)data)->process(input, output,
                                                   nframes, timeInfo,
                                                   flags);
}

void
AudioPortAudioTarget::sourceModelReplaced()
{
    m_source->setTargetSampleRate(m_sampleRate);
}

int
AudioPortAudioTarget::process(const void *, void *outputBuffer,
                              sv_frame_t nframes,
                              const PaStreamCallbackTimeInfo *,
                              PaStreamCallbackFlags)
{
#ifdef DEBUG_AUDIO_PORT_AUDIO_TARGET    
    cerr << "AudioPortAudioTarget::process(" << nframes << ")" << endl;
#endif

    if (!m_source || m_done) {
#ifdef DEBUG_AUDIO_PORT_AUDIO_TARGET
        cerr << "AudioPortAudioTarget::process: Doing nothing, no source or application done" << endl;
#endif
        return 0;
    }

    if (!m_prioritySet) {
#ifndef _WIN32
        sched_param param;
        param.sched_priority = 20;
        if (pthread_setschedparam(pthread_self(), SCHED_RR, &param)) {
            cerr << "AudioPortAudioTarget: NOTE: couldn't set RT scheduling class" << endl;
        } else {
            cerr << "AudioPortAudioTarget: NOTE: successfully set RT scheduling class" << endl;
        }
#endif
        m_prioritySet = true;
    }

    float *output = (float *)outputBuffer;

    assert(nframes <= m_bufferSize);

    static float **tmpbuf = 0;
    static int tmpbufch = 0;
    static int tmpbufsz = 0;

    int sourceChannels = m_source->getSourceChannelCount();

    // Because we offer pan, we always want at least 2 channels
    if (sourceChannels < 2) sourceChannels = 2;

    if (!tmpbuf || tmpbufch != sourceChannels || int(tmpbufsz) < m_bufferSize) {

	if (tmpbuf) {
	    for (int i = 0; i < tmpbufch; ++i) {
		delete[] tmpbuf[i];
	    }
	    delete[] tmpbuf;
	}

	tmpbufch = sourceChannels;
	tmpbufsz = m_bufferSize;
	tmpbuf = new float *[tmpbufch];

	for (int i = 0; i < tmpbufch; ++i) {
	    tmpbuf[i] = new float[tmpbufsz];
	}
    }
	
    sv_frame_t received = m_source->getSourceSamples(nframes, tmpbuf);

    float peakLeft = 0.0, peakRight = 0.0;

    for (int ch = 0; ch < 2; ++ch) {
	
	float peak = 0.0;

	if (ch < sourceChannels) {

	    // PortAudio samples are interleaved
	    for (int i = 0; i < nframes; ++i) {
                if (i < received) {
                    output[i * 2 + ch] = tmpbuf[ch][i] * m_outputGain;
                    float sample = fabsf(output[i * 2 + ch]);
                    if (sample > peak) peak = sample;
                } else {
                    output[i * 2 + ch] = 0;
                }
	    }

	} else if (ch == 1 && sourceChannels == 1) {

	    for (int i = 0; i < nframes; ++i) {
                if (i < received) {
                    output[i * 2 + ch] = tmpbuf[0][i] * m_outputGain;
                    float sample = fabsf(output[i * 2 + ch]);
                    if (sample > peak) peak = sample;
                } else {
                    output[i * 2 + ch] = 0;
                }
	    }

	} else {
	    for (int i = 0; i < nframes; ++i) {
		output[i * 2 + ch] = 0;
	    }
	}

	if (ch == 0) peakLeft = peak;
	if (ch > 0 || sourceChannels == 1) peakRight = peak;
    }

    m_source->setOutputLevels(peakLeft, peakRight);

    if (Pa_GetStreamCpuLoad(m_stream) > 0.7) {
        if (m_source) m_source->audioProcessingOverload();
    }

    return 0;
}

#endif /* HAVE_PORTAUDIO */

