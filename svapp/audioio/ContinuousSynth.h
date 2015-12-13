/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef CONTINUOUS_SYNTH_H
#define CONTINUOUS_SYNTH_H

#include "base/BaseTypes.h"

/**
 * Mix into a target buffer a signal synthesised so as to sound at a
 * specific frequency. The frequency may change with each processing
 * block, or may be switched on or off.
 */

class ContinuousSynth
{
public:
    ContinuousSynth(int channels, sv_samplerate_t sampleRate, sv_frame_t blockSize, int waveType);
    ~ContinuousSynth();
    
    void setChannelCount(int channels);

    void reset();

    /**
     * Mix in a signal to be heard at the given fundamental
     * frequency. Any oscillator state will be maintained between
     * process calls so as to provide a continuous sound. The f0 value
     * may vary between calls.
     *
     * Supply f0 equal to 0 if you want to maintain the f0 from the
     * previous block (without having to remember what it was).
     *
     * Supply f0 less than 0 for silence. You should continue to call
     * this even when the signal is silent if you want to ensure the
     * sound switches on and off cleanly.
     */
    void mix(float **toBuffers,
             float gain,
             float pan,
             float f0);

private:
    int m_channels;
    sv_samplerate_t m_sampleRate;
    sv_frame_t m_blockSize;

    double m_prevF0;
    double m_phase;

    int m_wavetype;
};

#endif
