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

#include "ContinuousSynth.h"

#include "base/Debug.h"
#include "system/System.h"

#include <cmath>

ContinuousSynth::ContinuousSynth(int channels, sv_samplerate_t sampleRate, sv_frame_t blockSize, int waveType) :
    m_channels(channels),
    m_sampleRate(sampleRate),
    m_blockSize(blockSize),
    m_prevF0(-1.0),
    m_phase(0.0),
    m_wavetype(waveType) // 0: 3 sinusoids, 1: 1 sinusoid, 2: sawtooth, 3: square
{
}

ContinuousSynth::~ContinuousSynth()
{
}

void
ContinuousSynth::reset()
{
    m_phase = 0;
}

void
ContinuousSynth::mix(float **toBuffers, float gain, float pan, float f0f)
{
    double f0(f0f);
    if (f0 == 0.0) f0 = m_prevF0;

    bool wasOn = (m_prevF0 > 0.0);
    bool nowOn = (f0 > 0.0);

    if (!nowOn && !wasOn) {
        m_phase = 0;
        return;
    }

    sv_frame_t fadeLength = 100;

    float *levels = new float[m_channels];
    
    for (int c = 0; c < m_channels; ++c) {
        levels[c] = gain * 0.5f; // scale gain otherwise too loud compared to source
    }
    if (pan != 0.0 && m_channels == 2) {
        levels[0] *= 1.0f - pan;
        levels[1] *= pan + 1.0f;
    }

//    cerr << "ContinuousSynth::mix: f0 = " << f0 << " (from " << m_prevF0 << "), phase = " << m_phase << endl;

    for (sv_frame_t i = 0; i < m_blockSize; ++i) {

        double fHere = (nowOn ? f0 : m_prevF0);

        if (wasOn && nowOn && (f0 != m_prevF0) && (i < fadeLength)) {
            // interpolate the frequency shift
            fHere = m_prevF0 + ((f0 - m_prevF0) * double(i)) / double(fadeLength);
        }

        double phasor = (fHere * 2 * M_PI) / m_sampleRate;
    
        m_phase = m_phase + phasor;

        int harmonics = int((m_sampleRate / 4) / fHere - 1);
        if (harmonics < 1) harmonics = 1;

        switch (m_wavetype) {
        case 1:
            harmonics = 1;
            break;
        case 2:
            break;
        case 3:
            break;
        default:
            harmonics = 3;
            break;
        }

        for (int h = 0; h < harmonics; ++h) {

            double v = 0;
            double hn = 0;
            double hp = 0;

            switch (m_wavetype) {
            case 1: // single sinusoid
                v = sin(m_phase);
                break;
            case 2: // sawtooth
                if (h != 0) {
                    hn = h + 1;
                    hp = m_phase * hn;
                    v = -(1.0 / M_PI) * sin(hp) / hn;
                } else {
                    v = 0.5;
                }
                break;
            case 3: // square
                hn = h*2 + 1;
                hp = m_phase * hn;
                v = sin(hp) / hn;
                break;
            default: // 3 sinusoids
                hn = h + 1;
                hp = m_phase * hn;
                v = sin(hp) / hn;
                break;
            }

            if (!wasOn && i < fadeLength) {
                // fade in
                v = v * (double(i) / double(fadeLength));
            } else if (!nowOn) {
                // fade out
                if (i > fadeLength) v = 0;
                else v = v * (1.0 - (double(i) / double(fadeLength)));
            }

            for (int c = 0; c < m_channels; ++c) {
                toBuffers[c][i] += float(levels[c] * v);
            }
        }
    }    

    m_prevF0 = f0;

    delete[] levels;
}

