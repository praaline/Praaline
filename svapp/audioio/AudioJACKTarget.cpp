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

#ifdef HAVE_JACK

#include "AudioJACKTarget.h"
#include "AudioCallbackPlaySource.h"

#include <iostream>
#include <cmath>

#include <alloca.h>

//#define DEBUG_AUDIO_JACK_TARGET 1

#ifdef BUILD_STATIC
#ifdef Q_OS_LINUX

// Some lunacy to enable JACK support in static builds.  JACK isn't
// supposed to be linked statically, because it depends on a
// consistent shared memory layout between client library and daemon,
// so it's very fragile in the face of version mismatches.
//
// Therefore for static builds on Linux we avoid linking against JACK
// at all during the build, instead using dlopen and runtime symbol
// lookup to switch on JACK support at runtime.  The following big
// mess (down to the #endifs) is the code that implements this.

static void *symbol(const char *name)
{
    static bool attempted = false;
    static void *library = 0;
    static std::map<const char *, void *> symbols;
    if (symbols.find(name) != symbols.end()) return symbols[name];
    if (!library) {
        if (!attempted) {
            library = ::dlopen("libjack.so.1", RTLD_NOW);
            if (!library) library = ::dlopen("libjack.so.0", RTLD_NOW);
            if (!library) library = ::dlopen("libjack.so", RTLD_NOW);
            if (!library) {
                cerr << "WARNING: AudioJACKTarget: Failed to load JACK library: "
                          << ::dlerror() << " (tried .so, .so.0, .so.1)"
                          << endl;
            }
            attempted = true;
        }
        if (!library) return 0;
    }
    void *symbol = ::dlsym(library, name);
    if (!symbol) {
        cerr << "WARNING: AudioJACKTarget: Failed to locate symbol "
                  << name << ": " << ::dlerror() << endl;
    }
    symbols[name] = symbol;
    return symbol;
}

static jack_client_t *dynamic_jack_client_open(const char *client_name,
                                               jack_options_t options,
                                               jack_status_t *status, ...)
{
    typedef jack_client_t *(*func)(const char *client_name,
                                   jack_options_t options,
                                   jack_status_t *status, ...);
    void *s = symbol("jack_client_open");
    if (!s) return 0;
    func f = (func)s;
    return f(client_name, options, status); // varargs not supported here
}

static int dynamic_jack_set_process_callback(jack_client_t *client,
                                             JackProcessCallback process_callback,
                                             void *arg)
{
    typedef int (*func)(jack_client_t *client,
                        JackProcessCallback process_callback,
                        void *arg);
    void *s = symbol("jack_set_process_callback");
    if (!s) return 1;
    func f = (func)s;
    return f(client, process_callback, arg);
}

static int dynamic_jack_set_xrun_callback(jack_client_t *client,
                                          JackXRunCallback xrun_callback,
                                          void *arg)
{
    typedef int (*func)(jack_client_t *client,
                        JackXRunCallback xrun_callback,
                        void *arg);
    void *s = symbol("jack_set_xrun_callback");
    if (!s) return 1;
    func f = (func)s;
    return f(client, xrun_callback, arg);
}

static const char **dynamic_jack_get_ports(jack_client_t *client, 
                                           const char *port_name_pattern, 
                                           const char *type_name_pattern, 
                                           unsigned long flags)
{
    typedef const char **(*func)(jack_client_t *client, 
                                 const char *port_name_pattern, 
                                 const char *type_name_pattern, 
                                 unsigned long flags);
    void *s = symbol("jack_get_ports");
    if (!s) return 0;
    func f = (func)s;
    return f(client, port_name_pattern, type_name_pattern, flags);
}

static jack_port_t *dynamic_jack_port_register(jack_client_t *client,
                                               const char *port_name,
                                               const char *port_type,
                                               unsigned long flags,
                                               unsigned long buffer_size)
{
    typedef jack_port_t *(*func)(jack_client_t *client,
                                 const char *port_name,
                                 const char *port_type,
                                 unsigned long flags,
                                 unsigned long buffer_size);
    void *s = symbol("jack_port_register");
    if (!s) return 0;
    func f = (func)s;
    return f(client, port_name, port_type, flags, buffer_size);
}

static int dynamic_jack_connect(jack_client_t *client,
                                const char *source,
                                const char *dest)
{
    typedef int (*func)(jack_client_t *client,
                        const char *source,
                        const char *dest);
    void *s = symbol("jack_connect");
    if (!s) return 1;
    func f = (func)s;
    return f(client, source, dest);
}

static void *dynamic_jack_port_get_buffer(jack_port_t *port,
                                          jack_nframes_t sz)
{
    typedef void *(*func)(jack_port_t *, jack_nframes_t);
    void *s = symbol("jack_port_get_buffer");
    if (!s) return 0;
    func f = (func)s;
    return f(port, sz);
}

static int dynamic_jack_port_unregister(jack_client_t *client,
                                        jack_port_t *port)
{
    typedef int(*func)(jack_client_t *, jack_port_t *);
    void *s = symbol("jack_port_unregister");
    if (!s) return 0;
    func f = (func)s;
    return f(client, port);
}

static void dynamic_jack_port_get_latency_range(jack_port_t *port,
                                                jack_latency_callback_mode_t mode,
                                                jack_latency_range_t *range)
{
    typedef void (*func)(jack_port_t *, jack_latency_callback_mode_t, jack_latency_range_t *);
    void *s = symbol("jack_port_get_latency_range");
    if (!s) {
        range->min = range->max = 0;
        return;
    }
    func f = (func)s;
    f(port, mode, range);
}

#define dynamic1(rv, name, argtype, failval) \
    static rv dynamic_##name(argtype arg) { \
        typedef rv (*func) (argtype); \
        void *s = symbol(#name); \
        if (!s) return failval; \
        func f = (func) s; \
        return f(arg); \
    }

dynamic1(jack_client_t *, jack_client_new, const char *, 0);
dynamic1(jack_nframes_t, jack_get_buffer_size, jack_client_t *, 0);
dynamic1(jack_nframes_t, jack_get_sample_rate, jack_client_t *, 0);
dynamic1(int, jack_activate, jack_client_t *, 1);
dynamic1(int, jack_deactivate, jack_client_t *, 1);
dynamic1(int, jack_client_close, jack_client_t *, 1);
dynamic1(jack_nframes_t, jack_frame_time, jack_client_t *, 0);
dynamic1(const char *, jack_port_name, const jack_port_t *, 0);

#define jack_client_new dynamic_jack_client_new
#define jack_client_open dynamic_jack_client_open
#define jack_get_buffer_size dynamic_jack_get_buffer_size
#define jack_get_sample_rate dynamic_jack_get_sample_rate
#define jack_set_process_callback dynamic_jack_set_process_callback
#define jack_set_xrun_callback dynamic_jack_set_xrun_callback
#define jack_activate dynamic_jack_activate
#define jack_deactivate dynamic_jack_deactivate
#define jack_client_close dynamic_jack_client_close
#define jack_frame_time dynamic_jack_frame_time
#define jack_get_ports dynamic_jack_get_ports
#define jack_port_register dynamic_jack_port_register
#define jack_port_unregister dynamic_jack_port_unregister
#define jack_port_name dynamic_jack_port_name
#define jack_connect dynamic_jack_connect
#define jack_port_get_buffer dynamic_jack_port_get_buffer

#endif
#endif

AudioJACKTarget::AudioJACKTarget(AudioCallbackPlaySource *source) :
    AudioCallbackPlayTarget(source),
    m_client(0),
    m_bufferSize(0),
    m_sampleRate(0),
    m_done(false)
{
    JackOptions options = JackNullOption;
#ifdef HAVE_PORTAUDIO_2_0
    options = JackNoStartServer;
#endif
#ifdef HAVE_LIBPULSE
    options = JackNoStartServer;
#endif

    JackStatus status = JackStatus(0);
    m_client = jack_client_open(source->getClientName().toLocal8Bit().data(),
                                options, &status);
    
    if (!m_client) {
        cerr << "AudioJACKTarget: Failed to connect to JACK server: status code "
                  << status << endl;
        return;
    }

    m_bufferSize = jack_get_buffer_size(m_client);
    m_sampleRate = jack_get_sample_rate(m_client);

    jack_set_xrun_callback(m_client, xrunStatic, this);
    jack_set_process_callback(m_client, processStatic, this);

    if (jack_activate(m_client)) {
	cerr << "ERROR: AudioJACKTarget: Failed to activate JACK client"
		  << endl;
    }

    if (m_source) {
	sourceModelReplaced();
    }
    
    // Mainstream JACK (though not jackdmp) calls mlockall() to lock
    // down all memory for real-time operation.  That isn't a terribly
    // good idea in an application like this that may have very high
    // dynamic memory usage in other threads, as mlockall() applies
    // across all threads.  We're far better off undoing it here and
    // accepting the possible loss of true RT capability.
    MUNLOCKALL();
}

AudioJACKTarget::~AudioJACKTarget()
{
    cerr << "AudioJACKTarget::~AudioJACKTarget()" << endl;

    if (m_source) {
        m_source->setTarget(0, m_bufferSize);
    }

    shutdown();

    if (m_client) {

        while (m_outputs.size() > 0) {
            std::vector<jack_port_t *>::iterator itr = m_outputs.end();
            --itr;
            jack_port_t *port = *itr;
            cerr << "unregister " << m_outputs.size() << endl;
            if (port) jack_port_unregister(m_client, port);
            m_outputs.erase(itr);
        }
        cerr << "Deactivating... ";
	jack_deactivate(m_client);
        cerr << "done\nClosing... ";
	jack_client_close(m_client);
        cerr << "done" << endl;
    }

    m_client = 0;

    cerr << "AudioJACKTarget::~AudioJACKTarget() done" << endl;
}

void
AudioJACKTarget::shutdown()
{
    m_done = true;
}

bool
AudioJACKTarget::isOK() const
{
    return (m_client != 0);
}

double
AudioJACKTarget::getCurrentTime() const
{
    if (m_client && m_sampleRate) {
        return double(jack_frame_time(m_client)) / double(m_sampleRate);
    } else {
        return 0.0;
    }
}

int
AudioJACKTarget::processStatic(jack_nframes_t nframes, void *arg)
{
    return ((AudioJACKTarget *)arg)->process(nframes);
}

int
AudioJACKTarget::xrunStatic(void *arg)
{
    return ((AudioJACKTarget *)arg)->xrun();
}

void
AudioJACKTarget::sourceModelReplaced()
{
    m_mutex.lock();

    m_source->setTarget(this, m_bufferSize);
    m_source->setTargetSampleRate(m_sampleRate);

    int channels = m_source->getSourceChannelCount();

    // Because we offer pan, we always want at least 2 channels
    if (channels < 2) channels = 2;

    if (channels == (int)m_outputs.size() || !m_client) {
	m_mutex.unlock();
	return;
    }

    const char **ports =
	jack_get_ports(m_client, NULL, NULL,
		       JackPortIsPhysical | JackPortIsInput);
    int physicalPortCount = 0;
    while (ports[physicalPortCount]) ++physicalPortCount;

#ifdef DEBUG_AUDIO_JACK_TARGET    
    cerr << "AudioJACKTarget::sourceModelReplaced: have " << channels << " channels and " << physicalPortCount << " physical ports" << endl;
#endif

    while ((int)m_outputs.size() < channels) {

        const int namelen = 30;
	char name[namelen];
	jack_port_t *port;

	snprintf(name, namelen, "out %d", int(m_outputs.size() + 1));

	port = jack_port_register(m_client,
				  name,
				  JACK_DEFAULT_AUDIO_TYPE,
				  JackPortIsOutput,
				  0);

	if (!port) {
	    cerr
		<< "ERROR: AudioJACKTarget: Failed to create JACK output port "
		<< m_outputs.size() << endl;
	} else {
            jack_latency_range_t range;
            jack_port_get_latency_range(port, JackPlaybackLatency, &range);
	    m_source->setTargetPlayLatency(range.max);
            cerr << "AudioJACKTarget: output latency is " << range.max << endl;
	}

	if ((int)m_outputs.size() < physicalPortCount) {
	    jack_connect(m_client, jack_port_name(port), ports[m_outputs.size()]);
	}

	m_outputs.push_back(port);
    }

    while ((int)m_outputs.size() > channels) {
	std::vector<jack_port_t *>::iterator itr = m_outputs.end();
	--itr;
	jack_port_t *port = *itr;
	if (port) jack_port_unregister(m_client, port);
	m_outputs.erase(itr);
    }

    m_mutex.unlock();
}

int
AudioJACKTarget::process(jack_nframes_t nframes)
{
    if (m_done) return 0;

    if (!m_mutex.tryLock()) {
	return 0;
    }

    if (m_outputs.empty()) {
	m_mutex.unlock();
	return 0;
    }

#ifdef DEBUG_AUDIO_JACK_TARGET    
    cout << "AudioJACKTarget::process(" << nframes << "): have a source" << endl;
#endif

#ifdef DEBUG_AUDIO_JACK_TARGET    
    if (m_bufferSize != nframes) {
	cerr << "WARNING: m_bufferSize != nframes (" << m_bufferSize << " != " << nframes << ")" << endl;
    }
#endif

    float **buffers = (float **)alloca(m_outputs.size() * sizeof(float *));

    for (int ch = 0; ch < (int)m_outputs.size(); ++ch) {
	buffers[ch] = (float *)jack_port_get_buffer(m_outputs[ch], nframes);
    }

    sv_frame_t received = 0;

    if (m_source) {
	received = m_source->getSourceSamples(nframes, buffers);
    }

    for (int ch = 0; ch < (int)m_outputs.size(); ++ch) {
        for (sv_frame_t i = received; i < nframes; ++i) {
            buffers[ch][i] = 0.0;
        }
    }

    float peakLeft = 0.0, peakRight = 0.0;

    for (int ch = 0; ch < (int)m_outputs.size(); ++ch) {

	float peak = 0.0;

	for (int i = 0; i < (int)nframes; ++i) {
	    buffers[ch][i] *= m_outputGain;
	    float sample = fabsf(buffers[ch][i]);
	    if (sample > peak) peak = sample;
	}

	if (ch == 0) peakLeft = peak;
	if (ch > 0 || m_outputs.size() == 1) peakRight = peak;
    }
	    
    if (m_source) {
	m_source->setOutputLevels(peakLeft, peakRight);
    }

    m_mutex.unlock();
    return 0;
}

int
AudioJACKTarget::xrun()
{
    cerr << "AudioJACKTarget: xrun!" << endl;
    if (m_source) m_source->audioProcessingOverload();
    return 0;
}

#endif /* HAVE_JACK */

