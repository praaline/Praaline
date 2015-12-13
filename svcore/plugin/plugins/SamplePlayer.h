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

#ifndef _SAMPLE_PLAYER_H_
#define _SAMPLE_PLAYER_H_

#define DSSI_API_LEVEL 2

#include "../api/ladspa.h"
#include "../api/dssi.h"

#include <seq_event.h>

#include <QMutex>
#include <QString>
#include <vector>

class SamplePlayer
{
public:
    static const DSSI_Descriptor *getDescriptor(unsigned long index);

private:
    SamplePlayer(int sampleRate);
    ~SamplePlayer();

    enum {
	OutputPort    = 0,
	RetunePort    = 1,
	BasePitchPort = 2,
        ConcertAPort  = 3,
	SustainPort   = 4,
	ReleasePort   = 5,
	PortCount     = 6
    };

    enum {
	Polyphony = 128
    };

    static const char *const portNames[PortCount];
    static const LADSPA_PortDescriptor ports[PortCount];
    static const LADSPA_PortRangeHint hints[PortCount];
    static const LADSPA_Properties properties;
    static const LADSPA_Descriptor ladspaDescriptor;
    static const DSSI_Descriptor dssiDescriptor;
    static const DSSI_Host_Descriptor *hostDescriptor;

    static LADSPA_Handle instantiate(const LADSPA_Descriptor *, unsigned long);
    static void connectPort(LADSPA_Handle, unsigned long, LADSPA_Data *);
    static void activate(LADSPA_Handle);
    static void run(LADSPA_Handle, unsigned long);
    static void deactivate(LADSPA_Handle);
    static void cleanup(LADSPA_Handle);
    static char *configure(LADSPA_Handle, const char *, const char *);
    static const DSSI_Program_Descriptor *getProgram(LADSPA_Handle, unsigned long);
    static void selectProgram(LADSPA_Handle, unsigned long, unsigned long);
    static int getMidiController(LADSPA_Handle, unsigned long);
    static void runSynth(LADSPA_Handle, unsigned long,
			 snd_seq_event_t *, unsigned long);
    static void receiveHostDescriptor(const DSSI_Host_Descriptor *descriptor);
    static void workThreadCallback(LADSPA_Handle);

    void searchSamples();
    void loadSampleData(QString path);
    void runImpl(unsigned long, snd_seq_event_t *, unsigned long);
    void addSample(int, unsigned long, unsigned long);

    float *m_output;
    float *m_retune;
    float *m_basePitch;
    float *m_concertA;
    float *m_sustain;
    float *m_release;

    float *m_sampleData;
    size_t m_sampleCount;
    int m_sampleRate;

    long m_ons[Polyphony];
    long m_offs[Polyphony];
    int m_velocities[Polyphony];
    long m_sampleNo;

    QString m_sampleDir;
    QString m_program;
    std::vector<std::pair<QString, QString> > m_samples; // program name, path
    bool m_sampleSearchComplete;
    int m_pendingProgramChange;

    QMutex m_mutex;
};


#endif
